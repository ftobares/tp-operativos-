#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <pthread.h>
#include "app.h"
#include "serializer.h"
#include "logic.h"

int main(void) {

	puts("Inicializando App...");

	if(inicializar() != 0){
		finalizarPrograma();
		return EXIT_FAILURE;
	}

	puts("App se ha inicializado correctamente");
	puts("Ingrese cualquier mensaje por consola para finalizar el programa.");

	char* entrada[30];
	gets(entrada);
	finalizarPrograma();
	return EXIT_SUCCESS;
}

int inicializar(){

	// Se levanta archivo de configuracion
	config = cargar_configuracion("./src/app.config", APP);

	// Se crea un logger
	logger = log_create(config->archivo_log,"App",1,LOG_LEVEL_TRACE);

	// Se inicializa lista de hilos
	threads = list_create();

	// Se inicializan listas para almacenar datos temporales
	clientesConectados = list_create();
	restaurantesConectados = list_create();
	pedidosEnCreacion = list_create();
	segmentosLibres = list_create();

	// Listas y colas dejan de apuntar a "basura"
	colaNew = NULL;
	colaReady = NULL;
	listaExec = NULL;
	colaBlock = NULL;
	colaExit = NULL;
	repartidores = NULL;
	restoDefault = NULL;

	if(pthread_mutex_init(&mutexListaThreads,NULL) != 0) return -1;
	if(pthread_mutex_init(&mutexClientesConectados,NULL) != 0) return -1;
	if(pthread_mutex_init(&mutexRestaurantesConectados,NULL) != 0) return -1;
	if(pthread_mutex_init(&mutexPedidosEnCreacion,NULL) != 0) return -1;

	// Se realiza el init
	if (conectarseConComanda(config->ip_comanda, config->puerto_comanda) != 0) {
		log_error(logger,"No se pudo inicializar la app correctamente. Hubo un error al conectarse al módulo Comanda.");
		return -1;
	}

	log_debug(logger, "INIT 1/3: Handshake con la comanda realizado con éxito.");

	if (abrirPuertoEscucha(config->puerto_escucha) != 0) {
		log_error(logger,"No se pudo inicializar la app correctamente. Hubo un error al abrir el socket de escucha.");
		return -2;
	}
	log_debug(logger, "INIT 2/3: Puerto de escucha abierto con éxito.");

	if (iniciarPlanificador() != 0) {
		log_error(logger,"No se pudo inicializar la app correctamente. Hubo un error al iniciar el planificador.");
		return -3;
	}
	log_debug(logger, "INIT 3/3: Planificador inicializado con éxito.");

	// Si se pudo inicializar correctamente, retornamos 0.
	return 0;
}

int conectarseConComanda(char* ip, char* puerto){
	t_socket comanda = crear_socket_de_conexion(ip,puerto);

	// Chequea que la instancia de comanda esté levantada
	if(comanda.socket == -1){
		log_error(logger,"No se pudo crear el socket de conexión con la Comanda.");
		return -1;
	}
	log_info(logger,"Socket de conexión con la Comanda creado con éxito.");

	if(!conectar_socket(comanda)){
		freeaddrinfo(comanda.socket_info);
		log_error(logger,"No se pudo establecer la conexión con la Comanda.");
		return -1;
	}

	// Se hace un handshake a la comanda.
	t_ping_s* ping = malloc(sizeof(t_ping_s));
	ping->result = 1;

	t_buffer mensaje = serializar_mensaje(ping,T_PING,sizeof(uint32_t),comanda.socket);
	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el mensaje de handshake a la Comanda.");
		free(ping);
		free(mensaje.data);
		freeaddrinfo(comanda.socket_info);
		return -1;
	}
	log_info(logger, "Handshake con la Comanda realizado exitosamente.");
	free(ping);
	free(mensaje.data);
	close(comanda.socket);
	freeaddrinfo(comanda.socket_info);
	return 0;
}

int abrirPuertoEscucha(char* puerto){

	// Creación del Listener
	t_socket servidor = crear_socket_de_escucha(puerto);

	if(servidor.socket == 0){
		log_error(logger, "INIT 2/3: Hubo un error al crear el socket de escucha.");
		freeaddrinfo(servidor.socket_info);
		return -1;
	}

	log_info(logger,"INIT 2/3: Socket de escucha creado con éxito.");

	bool escuchando = bind_listen_socket_escucha(servidor);
	if(!escuchando){
		log_error(logger, "INIT 2/3: Hubo un error al bindear el puerto del servidor.");
		freeaddrinfo(servidor.socket_info);
		return -1;
	}
	log_info(logger,"INIT 2/3: App escuchando por peticiones entrantes en el puerto %s.", puerto);

	// Creación de hilo que espera conexiones entrantes
	pthread_t listen;
	pthread_t * t1 = malloc(sizeof(pthread_t));

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	t_socket * pServidor = malloc(sizeof(t_socket));
	*pServidor = servidor;

	// Se crea hilo que escuche las peticiones entrantes
	if(pthread_create(&listen, &attr, escucharPeticiones, (void *)pServidor) != 0){
		log_error(logger, "INIT 2/3: Hubo un error al ponerse a escuchar peticiones.");
		free(t1);
		free(pServidor);
		freeaddrinfo(servidor.socket_info);
		return -1;
	}
	*t1 = listen;

	list_add(segmentosLibres,pServidor);

	pthread_mutex_lock(&mutexListaThreads);
	list_add(threads,t1);
	pthread_mutex_unlock(&mutexListaThreads);

	freeaddrinfo(servidor.socket_info);
	return 0;

}

// === Listener de mensajes entrantes

void* atenderCliente(void* data){
	t_tipo_mensaje TIPO_MENSAJE;
	t_socket* socketCliente = (t_socket*) data;

	do {
		t_buffer mensaje = recibir_mensaje(socketCliente->socket);

		TIPO_MENSAJE = mensaje.msj_type;
		if(TIPO_MENSAJE == 0){
			log_error(logger, "Error al recibir un mensaje");
			continue;
		}

		log_info(logger,"=== LLEGÓ UN MENSAJE ===");

		// Creo una peticion para antenderla en un hilo aparte
		t_peticion* peticion = malloc(sizeof(t_peticion));
		peticion->socketCliente = socketCliente;
		peticion->mensaje = &mensaje;

		switch (TIPO_MENSAJE) {
			case T_DATOS_CLIENTE: {
				guardarConexionCliente(peticion);
				free(mensaje.data);
				break;
			}
			case T_DATOS_RESTAURANTE: {
				guardarConexionRestaurante(peticion);
				free(mensaje.data);
				break;
			}
			case T_CONSULTAR_RESTAURANTES: {
				consultarRestaurantes(peticion);
				break;
			}
			case T_SELECCIONAR_RESTAURANTE: {
				seleccionarRestaurante(peticion);
				free(mensaje.data);
				break;
			}
			case T_CONSULTAR_PLATOS: {
				consultarPlatos(peticion);
				break;
			}
			case T_CREAR_PEDIDO: {
				crearPedido(peticion);
				break;
			}
			case T_ANIADIR_PLATO: {
				aniadirPlato(peticion);
				free(mensaje.data);
				break;
			}
			case T_PLATO_LISTO: {
				platoListo(peticion);
				//free(mensaje.data);
				break;
			}
			case T_CONFIRMAR_PEDIDO_SOLO_ID: {
				confirmarPedido(peticion);
				free(mensaje.data);
				break;
			}
			case T_CONSULTAR_PEDIDO: {
				consultarPedido(peticion);
				free(mensaje.data);
				break;
			}
		}
		if(peticion != NULL) free(peticion);

	} while(TIPO_MENSAJE != SOCKET_CLOSED);

	bool tieneElMismoThreadId(pthread_t* thread){
		return pthread_self() == *thread;
	}
	pthread_mutex_lock(&mutexListaThreads);
	pthread_t* hilo = list_remove_by_condition(threads,(void*) tieneElMismoThreadId);
	free(hilo);
	pthread_mutex_unlock(&mutexListaThreads);

	eliminarConexionCliente(socketCliente);
	pthread_exit(NULL);
}

void* escucharPeticiones(void *data){

	t_socket socketConexion;
	t_socket* socketServidor = (t_socket*) data;
	pthread_t hilo;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	while(1){
		log_debug(logger, "APP esperando por mensajes entrantes...");
		socketConexion = aceptando_conexiones(*socketServidor);

		t_socket * socketCliente = malloc(sizeof(t_socket));
		*socketCliente = socketConexion;

		if(socketCliente->socket == -1){
			log_error(logger, "Hubo un error aceptando una conexión entrante...");
			free(socketCliente);
			continue;
		}
		log_debug(logger, "Se ha conectado un cliente al servidor... Socket = %d",socketCliente->socket);
		if (pthread_create(&hilo, &attr, atenderCliente, (void*) socketCliente) != 0) {
			log_error(logger,"Hubo un error al crear un hilo para atender a un cliente");
			free(socketCliente);
			continue;
		}
		pthread_t * thread = malloc(sizeof(t_socket));
		*thread = hilo;

		pthread_mutex_lock(&mutexListaThreads);
		list_add(threads,thread);
		pthread_mutex_unlock(&mutexListaThreads);
	}
}

// === Finalización del programa ===
void finalizarPrograma(){
	puts("Liberando recursos...");
	liberarHilos();
	liberarClientesConectados();
	liberarRestaurantesConectados();
	liberarPedidosEnCreacion();
	liberarColas();
	liberarRepartidores();
	liberarRestauranteDefault();
	free(ALGORITMO_DE_PLANIFICACION);
	liberarSemaforosPlanificador();
	liberarConfig();
	log_destroy(logger);
	liberarSegmentosLibres();
	puts("Finalizando programa...");
}

// === Liberación de Memoria ===
void liberarConfig(){
	free(config->puerto_escucha);
	free(config->ip_comanda);
	free(config->puerto_comanda);
	free(config->algoritmo_planificacion);
	free(config->repartidores);
	free(config->frecuencia_descanso);
	free(config->tiempo_descanso);
	free(config->archivo_log);
	free(config->platos_default);
	free(config);
}

void liberarClientesConectados(){
	void cliente_conectado_destroy(t_cliente_app* cliente){
		close(cliente->socketCliente->socket);
		free(cliente->socketCliente);
		free(cliente->idCliente);
		free(cliente->host);
		free(cliente->puerto);
		free(cliente->posicion);
		free(cliente);
	}
	list_destroy_and_destroy_elements(clientesConectados,(void*)cliente_conectado_destroy);
}

void liberarRestaurantesConectados(){
	void restaurante_conectado_destroy(t_restaurante_app* restaurante){
		free(restaurante->nombre);
		close(restaurante->socketRestaurante->socket);
		free(restaurante->socketRestaurante);
		free(restaurante->ip);
		free(restaurante->puerto);
		free(restaurante->posicion);
		free(restaurante);
	}
	list_destroy_and_destroy_elements(restaurantesConectados,(void*)restaurante_conectado_destroy);
}

void liberarPedidosEnCreacion(){
	void pedido_destroy(t_pedido_en_creacion* pedido){
		free(pedido);
	}
	list_destroy_and_destroy_elements(pedidosEnCreacion,(void*)pedido_destroy);
}

void liberarSegmentosLibres(){
	void liberar(void* segmento){
		free(segmento);
	}
	list_destroy_and_destroy_elements(segmentosLibres,(void*)liberar);

}

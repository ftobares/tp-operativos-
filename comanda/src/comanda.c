#include "comanda.h"

int main(void) {

	// DEBUG
	
	/*
	bool seguir = true;
	while(seguir) {
		t_log* loggerTest = log_create("/home/utnso/Documentos/tp-2020-2c-Snoop-/comanda/test.log", "Comanda" , 0, LOG_LEVEL_INFO);
		inicializarManejoDeMemoria(128, 2048, "LRU", loggerTest);
		seguir = testComanda();
		finalizarManejoDeMemoria();
		log_destroy(loggerTest);
	}
	*/
	
	int codInicializacion = inicializar();

	if (codInicializacion != 0) {
		switch(codInicializacion) {
			case 1:
				printf("Ha habido un error al cagar el archivo de configuración.\n");
				break;
			case 2:
				printf("Ha habido un error al cargar el logger.\n");	
				break;
			case 3: 
				printf("Ha habido un error al inicializar las colas\n");
				break;
			case 4: 
				printf("Ha habido un error al inicializar el socket de escucha\n");
				break;
			case 5: 
				printf("Ha habido un error al reservar los espacios de memoria principal y SWAP\n");
				break;
		}
		
		if (logger != NULL) {
			log_error(logger,"No se pudo inicializar la comanda");
		}		
		printf("Ha habido un error al inicializar la comanda");
		finalizar();
		return EXIT_FAILURE;
	}

	printf("¡Bienvenido al modulo comanda!\nOprima cualquier tecla finalizar...\n\n"); 	

	char exitCode[30];
	gets(exitCode);

	printf("Finalizando la comanda...\n");
	finalizar();
	printf("Comanda finalizado con éxito.\n\n");
	return EXIT_SUCCESS;
}

void enviarRespuesta(int result, int socketClient) {
	t_resultado_operacion* paquete = malloc(sizeof(t_resultado_operacion));

	if(result == EXIT_SUCCESS) result = 1;
	else result = 0;

	paquete->resultado = result;
	t_header* header = malloc(sizeof(t_header));
	header->msj_type = T_RESULTADO_OPERACION;
	header->size = (sizeof(uint32_t));
	t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, socketClient);

	if (!enviar_mensaje(&buffer)) {
		log_info(logger, "Hubo un error al enviar el mensaje de respuesta");
	}
	free(buffer.data);
	free(paquete);
	free(header);
}

void enviarRespuestaObtenerPedido(t_obtener_pedido_s* respuesta, int socketClient) {
	t_header* header = malloc(sizeof(t_header));
	header->msj_type = T_OBTENER_PEDIDO_RESPUESTA;

	uint32_t tamanioPlato = (sizeof(uint32_t) * 2) + sizeof(char[24]);
	
	header->size = ((sizeof(uint32_t) * 2) + (respuesta->cantidadPlatos * tamanioPlato));

	t_buffer buffer = serializar_mensaje(respuesta, header->msj_type, header->size, socketClient);

	if (!enviar_mensaje(&buffer)) {
		log_info(logger, "Hubo un error al enviar el mensaje de respuesta");
	}
	free(buffer.data);
	free(header);
}

void nuevaConexion(int* socketCliente) {
	int result = 0;
	t_buffer buffer = recibir_mensaje(*socketCliente);
	while (buffer.msj_type != SOCKET_CLOSED) {
		void* bufferData = buffer.data;
		switch(buffer.msj_type) {
			case T_PING:;
				t_ping_s* paquetePing = (t_ping_s*)deserializar_mensaje(&buffer);
				printf("Mensaje ping recibido. Code: %i\n", paquetePing->result);
				break;
			
			case T_DATOS_CLIENTE: 
				manejarCliente(&buffer);
				break;
				
			case T_GUARDAR_PEDIDO:;
				t_pedido_s* paqueteGuardarPedido = (t_pedido_s*)deserializar_mensaje(&buffer);
				result = guardarPedido(paqueteGuardarPedido->nombre_restaurante, paqueteGuardarPedido->id_pedido);
				free(paqueteGuardarPedido);
				enviarRespuesta(result, *socketCliente);
				break;
			case T_GUARDAR_PLATO:;
				t_guardar_plato_s* paqueteGuardarPlato = (t_guardar_plato_s*)deserializar_mensaje(&buffer);
				result = guardarPlato(paqueteGuardarPlato->nombre_restaurante, paqueteGuardarPlato->id_pedido, paqueteGuardarPlato->nombre_plato, paqueteGuardarPlato->cantidad);
				free(paqueteGuardarPlato);
				enviarRespuesta(result, *socketCliente);
				break;
			case T_OBTENER_PEDIDO:;
				t_pedido_s* paqueteObtenerPedido = (t_pedido_s*)deserializar_mensaje(&buffer);
				t_obtener_pedido_s* pedidoObtenido = obtenerPedido(paqueteObtenerPedido->nombre_restaurante, paqueteObtenerPedido->id_pedido);
				
				
				if (pedidoObtenido == NULL) {
					pedidoObtenido = malloc(sizeof(t_obtener_pedido_s));
					pedidoObtenido->cantidadPlatos = 0;
					pedidoObtenido->estado = -1;
				}
				
				enviarRespuestaObtenerPedido(pedidoObtenido, *socketCliente);

				if (pedidoObtenido->cantidadPlatos != 0) {
					void freePlato(t_obtener_pedido_plato_s* plato) {free(plato);}
					list_destroy_and_destroy_elements(pedidoObtenido->platos, freePlato);
				}

				free(paqueteObtenerPedido->nombre_restaurante);
				free(paqueteObtenerPedido);
				free(pedidoObtenido);
				break;	
			case T_CONFIRMAR_PEDIDO:;
				t_pedido_s* paqueteConfirmarPedido = (t_pedido_s*)deserializar_mensaje(&buffer);
				result = confirmarPedido(paqueteConfirmarPedido->nombre_restaurante, paqueteConfirmarPedido->id_pedido);
				free(paqueteConfirmarPedido->nombre_restaurante);
				free(paqueteConfirmarPedido);
				enviarRespuesta(result, *socketCliente);
				break;
			case T_PLATO_LISTO:;
				t_plato_listo_s* paquetePlatoListo = (t_plato_listo_s*)deserializar_mensaje(&buffer);
				result = platoListo(paquetePlatoListo->nombre_restaurante, paquetePlatoListo->id_pedido, paquetePlatoListo->nombre_plato);
				free(paquetePlatoListo->nombre_restaurante);
				free(paquetePlatoListo);
				enviarRespuesta(result, *socketCliente);
				break;
			case T_FINALIZAR_PEDIDO:;
				t_pedido_s* paqueteFinalizarPedido = (t_pedido_s*)deserializar_mensaje(&buffer);
				result = finalizarPedido(paqueteFinalizarPedido->nombre_restaurante, paqueteFinalizarPedido->id_pedido);
				free(paqueteFinalizarPedido->nombre_restaurante);
				free(paqueteFinalizarPedido);
				enviarRespuesta(result, *socketCliente);
				break;
			default:
				printf("Mensaje no reconocido\n");
		}

		free(bufferData);
		buffer = recibir_mensaje(*socketCliente);
	}

	// cliente cerro el socket
	int32_t threadId = pthread_self();
	int filtro(int32_t thread) {
		return thread == threadId;
	}
	// elimino hilo que termino de procesar
	list_remove_by_condition(threads, (void*)filtro);
	pthread_exit(0);
}


int escuchar() {
	socketEscucha = malloc(sizeof(t_socket));
	*socketEscucha = crear_socket_de_escucha(config->puerto_escucha);

	if (bind_listen_socket_escucha(*socketEscucha)) {
		while (1) {
			t_socket socketCliente = aceptando_conexiones(*socketEscucha);
			if (socketCliente.socket != -1) {
				// creo hilo para la nueva conexion
				pthread_t thread;
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
				log_info(logger, "Nueva conexion aceptada (nro socket: %d)",socketCliente.socket );
				pthread_create(&thread, &attr, nuevaConexion, &socketCliente.socket);				
				// agrego a lista de threads trabajando
				list_add(threads, thread);
			}
		}
	} else {
		perror("No se pudo conectar\n");
		return 1;
	}
	return 0;
}


// Metodos de inicializacion
int inicializar() {
	config = cargar_configuracion("./src/comanda.config", COMANDA);
	if (config == NULL) return 1;

	logger = log_create(config->archivo_log, "Comanda" , 0, LOG_LEVEL_INFO);
	if (logger == NULL) return 2;

	if (iniciarColas() != 0) return 3;

	if (inicializarSocketEscucha() != 0) return 4;
	
	if (inicializarManejoDeMemoria(config->tamanio_memoria, config->tamanio_swap, config->algoritmo_reemplazo, logger) != 0) return 5;
	
	return EXIT_SUCCESS;
}

int inicializarSocketEscucha() {
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_create(&threadEscucha, &attr, escuchar, NULL);
	return EXIT_SUCCESS;
}

int iniciarColas() {
	threads = list_create();
	return EXIT_SUCCESS;
}

// FIN Metodos de inicializacion 

// Metodos de finalizacion
void finalizar(){
	log_destroy(logger);
	liberarConfig();
	cerrarSocket();
	liberarHilos();
	liberarColas();
	finalizarManejoDeMemoria();
}

void liberarConfig(){
	free(config->algoritmo_reemplazo);
	free(config->archivo_log);
	free(config->puerto_escucha);
	free(config);
}

void liberarHilo(int32_t thread) {
	pthread_cancel(&thread);
}

void liberarHilos() {
	pthread_cancel(&threadEscucha);
}

void liberarColas() {
	list_destroy_and_destroy_elements(threads, liberarHilo);
}

void cerrarSocket() { 
	if (socketEscucha != NULL) {
		close(socketEscucha->socket);
		freeaddrinfo(socketEscucha->socket_info);
		free(socketEscucha);
	}
}
// FIN Metodos de finalizacion

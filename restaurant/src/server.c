#include "server.h"

volatile sig_atomic_t exitServer = 1;

void liberarHiloSrv(pthread_t* thread);
void liberarHilosSrv();
void liberarRecursosServer();

int startupServer() {

	log_info(logger, "Iniciando Server");

	threadsConexionesEntrantes = list_create();

	// Escucho mensajes entrantes
	socketEscucha = crear_socket_de_escucha(config->puerto_escucha);

	if (bind_listen_socket_escucha(socketEscucha)) {

		log_info(logger, "Socket abierto correctamente. Esperando mensajes entrantes...\n");

		while (exitServer) {
			t_socket socketCliente = aceptando_conexiones(socketEscucha);

			if (socketCliente.socket != SOCKET_CLOSED) {
				log_info(logger, "Mensaje recibido. Socket: %d", socketCliente.socket);

				pthread_t* thread = malloc(sizeof(pthread_t));
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

				t_socket* pSocket = malloc(sizeof(t_socket));
				*pSocket = socketCliente;

				log_info(logger, "Enviando mensaje a hilo de procesamiento");

				pthread_create(&thread, &attr, procesarMensaje, pSocket);

				log_info(logger, "Mensaje enviado a hilo de procesamiento correctamente");

				list_add(threadsConexionesEntrantes, thread);
			}
		}
	} else {
		perror("No se pudo conectar\n");
		liberarRecursosServer();
		return EXIT_FAILURE;
	}

	log_info(logger, "Finalizando Server");

	liberarRecursosServer();

	return EXIT_SUCCESS;
}

void procesarMensaje(t_socket* socketCliente) {

	log_info(logger, "Procesando mensaje");

	t_buffer buffer = recibir_mensaje(socketCliente->socket);

	log_info(logger, "Buffer obtenido correctamente.");
	log_info(logger, "Message type: %d", buffer.msj_type);

	switch (buffer.msj_type) {
	case SOCKET_CLOSED:
		//Habria que hacer algo??
		break;

	case T_DATOS_CLIENTE: 
		manejarCliente(&buffer);
		break;

	case T_CONSULTAR_PLATOS:
		consultarPlatos(&buffer);
		break;

	case T_CONSULTAR_PLATOS_RESPUESTA:
		respuestaConsultarPlatos(&buffer);
		break;

	case T_CREAR_PEDIDO:
		crearPedido(&buffer);
		break;

	case T_CREAR_PEDIDO_RESPUESTA:
		respuestaCrearPedido(&buffer);
		break;

	case T_ANIADIR_PLATO:
		aniadirPlato(&buffer);
		break;

	case T_ANIADIR_PLATO_RESPUESTA: // Ojo acá que en este caso se recibe un OK/FAIL o sea tipo = T_RESULTADO_OPERACION
		respuestaAniadirPlato(&buffer);
		break;

	case T_CONFIRMAR_PEDIDO_SOLO_ID:
		confirmarPedido(&buffer);
		break;

	case T_CONFIRMAR_PEDIDO_RESPUESTA:
		respuestaConfirmarPedido(&buffer);
		break;

	case T_CONSULTAR_PEDIDO:
		consultarPedido(&buffer);
		break;

	case T_CONSULTAR_PEDIDO_RESPUESTA:
		respuestaConsultarPedido(&buffer);
		break;

	default:
		log_error(logger, "Tipo de mensaje invalido");
	}

	log_info(logger, "Mensaje procesado OK, liberando buffer");
}

void finalizarServer(){
	log_info(logger, "Forzando cierre de Server...");
	log_info(logger, "Set isExit en Server. [preValue: 1, postValue: 0]");
	exitServer = 0;

	shutdown(socketEscucha.socket, 2);
	close(socketEscucha.socket);
	freeaddrinfo(socketEscucha.socket_info);
}

/********************** PRIVATE FUNCTIONS **********************/

void liberarHiloSrv(pthread_t* thread){
	pthread_cancel(*thread);
}

void liberarHilosSrv(){
	list_destroy_and_destroy_elements(threadsConexionesEntrantes, liberarHiloSrv);
}

void liberarRecursosServer(){
	int size = list_size(threadsConexionesEntrantes);
	log_info(logger, "Tamanio lista threads en server: %i", size);
	if(size > 0){
		liberarHilosSrv();
	}else{
		list_destroy(threadsConexionesEntrantes);
	}
}


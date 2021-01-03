#include "conexiones.h"

void* levantar_servidor(){

	socket_servidor = crear_socket_de_escucha(puertoEscucha);
	if(socket_servidor.socket == 0){
		log_error(logger, "Hubo un error al crear el socket de escucha.");
		return -1;
	}

	if (!bind_listen_socket_escucha(socket_servidor)) {
		perror("No se pudo conectar\n");
		return -1;
	}
	
	while(1){
		t_socket socketCliente = aceptando_conexiones(socket_servidor);

		t_socket* socketCli = malloc(sizeof(t_socket));
		*socketCli = socketCliente;

		if(socketCliente.socket != SOCKET_CLOSED) {

		if(socketCliente.socket == -1)
			log_error(logger,"No se pudo aceptar una conexion entrante.");

		pthread_t hilo_actualizacion;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);	

		if(pthread_create(&hilo_actualizacion,&attr,recibir_actualizacion,(void*) socketCli)!=0){
			log_error(logger,"Error creación hilo que realiza la actualizacion.");
		}

		list_add(hilos,hilo_actualizacion);
		}
		else{
			log_error(logger,"No se pudo conectar el modulo externo.");
			return -1;
		}
	}

	return 0;
}

void* manejar_conexiones(t_socket servidor){
	t_socket cliente;
	
	pthread_t hilo_actualizacion;
/*	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);*/

	while(1){
		cliente = aceptando_conexiones(socket_servidor);
		if(cliente.socket == -1)
			log_error(logger,"No se pudo aceptar una conexion entrante.");
		if(pthread_create(&hilo_actualizacion,NULL,recibir_actualizacion,(void*) &cliente)!=0){
			log_error(logger,"Error creación hilo que realiza la actualizacion.");
		}
	}
}

void* handshake(){
	printf("-------------Conectandose al Modulo solicitado-------------\n\n");
	char* port = string_new();
	string_append(&port,string_itoa(config->puerto));
	socket_conectado = crear_socket_de_conexion(config->ip, port);
	if(conectar_socket(socket_conectado) == false){
		free(port);
		return -1;
	}
	
	t_buffer enviar = cargarCliente();
	enviar_mensaje(&enviar);

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_DATOS_CLIENTE_RESPUESTA){
		log_error(logger,"No se pudo realizar el handshake inicial con el modulo solicitado.");
		printf("No se pudo realizar el handshake inicial con el modulo solicitado. Abortando programa.\n");
		finalizar();
	}
	t_tipo_modulo* handshakeok = deserializar_mensaje(&recibido);
	moduloConectado = handshakeok->modulo;
	if(handshakeok->modulo == T_APP){
		log_debug(logger,"El modulo leyo nuestro ID y es la APP.");
	}
	else{
		log_debug(logger,"El modulo leyo nuestro ID y no estamos conectados a la APP.");
	}
	
	printf("\n-------------Usted esta conectado al modulo solicitado por configuracion-------------\n");
	log_info(logger,"Se realizo el handshake con el modulo solicitado.");
	free(handshakeok);
	free(recibido.data);
	free(enviar.data);
	free(port);
}

t_buffer cargarCliente(){
	uint32_t size_envio;
	t_cliente* info_cliente = malloc(sizeof(t_cliente));
	info_cliente->id_cliente = config->id_cliente;
	info_cliente->id_cliente_size = strlen(info_cliente->id_cliente) + 1;
	info_cliente->posicion_x = config->posicion_x;
	info_cliente->posicion_y = config->posicion_y;
	info_cliente->puerto_escucha = strdup(puertoEscucha);
	info_cliente->puerto_escucha_size = strlen(info_cliente->puerto_escucha) + 1;
	info_cliente->ip_escucha = strdup(hostEscucha);
	info_cliente->ip_escucha_size = strlen(info_cliente->ip_escucha) + 1;
	size_envio = sizeof(uint32_t)*5 + info_cliente->id_cliente_size + info_cliente->puerto_escucha_size + info_cliente->ip_escucha_size;

	t_tipo_mensaje cliente = T_DATOS_CLIENTE;
	t_buffer enviar = serializar_mensaje(info_cliente, cliente, size_envio, socket_conectado.socket);

	free(info_cliente->puerto_escucha);
	free(info_cliente->ip_escucha);
	free(info_cliente);
	return enviar;
}

void* cierre_app(){
	//freeaddrinfo(socket_cliente_app.socket_info);
	if(socket_conectado.socket != -1)
		close(socket_conectado.socket);
}

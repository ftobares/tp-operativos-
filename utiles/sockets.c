#include "sockets.h";

/**
 * @NAME: crear_socket_de_conexion
 * @DESC: Recibe dos strings, uno para la IP y otro para el PUERTO.
 *		  Crea el socket y asgina el identificador. Rellana la estructura
 *		  addrinfo con AF_UNSPEC y SOCK_STREAM, utilizando getaddrinfo.
 */
t_socket crear_socket_de_conexion(char* ip, char* puerto) 
{
	t_socket un_socket;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &(un_socket.socket_info));

	if ((un_socket.socket = socket(un_socket.socket_info->ai_family,
			un_socket.socket_info->ai_socktype,
			un_socket.socket_info->ai_protocol)) != 0) {
		return un_socket;
	} else {
		perror("Error al crear el socket de conexion\n");
		return un_socket;
	}
}

/**
 * @NAME: crear_socket_de_escucha
 * @DESC: Recibe un int con el puerto a escuchar.
 *		  Crea el socket y asigna el identificador. Rellena addrinfo
 *		  con AF_INET, INADDR_ANY y el PUERTO.
 */
t_socket crear_socket_de_escucha(char* puerto) 
{
	t_socket un_socket;
	un_socket.socket = 0;
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, puerto, &hints, &(un_socket.socket_info));

	if ((un_socket.socket = socket(un_socket.socket_info->ai_family,
			un_socket.socket_info->ai_socktype,
			un_socket.socket_info->ai_protocol)) != 0) {
		return un_socket;
	} else {
		perror("Error al crear el socket de conexion\n");
		return un_socket;
	}
}

/**
 * @NAME: conectar_socket
 * @DESC: Recibe un socket de conexion y se conecta al servidor definido
 */
bool conectar_socket(t_socket p_socket){
	if(connect(p_socket.socket, p_socket.socket_info->ai_addr, p_socket.socket_info->ai_addrlen) == 0){
		return true;
	}
	perror("Error al conectar el socket");
	return false;
}

/**
 * @NAME: bind_listen_socket_escucha
 * @DESC: Recibe un socket de escucha.
 * 		  Lo habilita para ser reutilizado por el SO.
 * 		  Bindea el socket al puerto configurado en el struct.
 * 		  Deja en escucha para nuevas conexiones.
 */
bool bind_listen_socket_escucha(t_socket p_socket){
	int activado = 1;
	if(setsockopt(p_socket.socket, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado)) == -1){
		perror("Error al activar re utilizacion de socket");
		return false;
	}
	if(bind(p_socket.socket, p_socket.socket_info->ai_addr, p_socket.socket_info->ai_addrlen) == 0){
		if(listen(p_socket.socket, MAX_CONNECTIONS) == 0){
			return true;
		}
		perror("Error en la escucha de conexiones");
		return false;
	}
	perror("Error en la escucha de conexiones");
	return false;
}

/**
 * @NAME: aceptando_conexiones
 * @DESC: Recibe un socket de escucha.
 * 		  Y genera un nuevo socket cliente, mediante la funcion accept
 */
t_socket aceptando_conexiones(t_socket socket_escucha){
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);
	t_socket socket_cliente;

	if((socket_cliente.socket = accept(socket_escucha.socket, (struct sockaddr *) &addr, &addrlen)) != -1){
		return socket_cliente;
	}
	perror("Error aceptando una conexion entrante");
	return socket_cliente;
}

/*@NAME: recibir_mensaje
 *@DESC: Recibe datos y los guarda en el buffer
 *		 hay que deserializar el buffer.data
 */
t_buffer recibir_mensaje(uint32_t un_socket) {

	t_buffer buff;
	buff.size = -1;
	buff.socket = un_socket;

	t_header header;
	int result = recv(buff.socket, &header, sizeof(header), MSG_WAITALL);
	if(result == 0){
		header.msj_type = SOCKET_CLOSED;
		buff.msj_type = header.msj_type;
		return buff;
	}else if(result < 0){
		header.msj_type = 0;
		buff.msj_type = header.msj_type;
		return buff;
	}

	if(header.size == 0){
		buff.data = NULL;
		buff.size = 0;
		buff.msj_type = header.msj_type;
	}
	else{
		buff.data = malloc(header.size);
		buff.size = recv(buff.socket, buff.data, header.size, 0);
		buff.msj_type = header.msj_type;
	}
	return buff;
}

/*@NAME: enviar_mensaje
 *@DESC: Envia datos cargados en el buffer
 *		 Datos ya deben venir serializados
 */
bool enviar_mensaje(t_buffer* buffer){
	t_header header;
	header.size = buffer->size;
	header.msj_type = buffer->msj_type;

	if(send(buffer->socket, &header, sizeof(header), 0) < 0)
		return false; // Si falla al enviar el header

	if(buffer->size <= 0){
		return true; // No envia 'body' y devuelve OK del header.
	}

	if(send(buffer->socket, buffer->data, header.size, 0) < 0){
		return false; // Si falla al enviar el cuerpo
	}
	return true;
}


t_socket conectarse(uint32_t puerto){

 	t_socket socket_cliente = crear_socket_de_conexion("127.0.0.1",puerto);

	if(!conectar_socket(socket_cliente)){
		perror("No se pudo conectar.");
		socket_cliente.socket = -1;
		return socket_cliente;
	}

	return socket_cliente;
}


t_buffer empaquetar_buffer(char* string, uint32_t socket, int msj_type, int emisor){
	t_buffer buffer_enviar;
	buffer_enviar.socket = socket;
	buffer_enviar.msj_type = msj_type;
	buffer_enviar.size = string_length(string)+2;
	buffer_enviar.data = string;

	return buffer_enviar;
}

t_buffer escuchar_recibir_cerrar(char* puerto){

	t_socket socket_escucha = crear_socket_de_escucha(puerto);
	bind_listen_socket_escucha(socket_escucha);
	t_socket socket_cliente;
	socket_cliente = aceptando_conexiones(socket_escucha);	//logica recibir mensaje de modulos
	t_buffer recibido = recibir_mensaje(socket_cliente.socket);
	close(socket_escucha.socket);

	return recibido;
}

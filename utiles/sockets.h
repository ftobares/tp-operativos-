#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdbool.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_CONNECTIONS 10
#define MAX_PACKAGE_SIZE 1024
#define SOCKET_CLOSED -1

typedef struct {
	int32_t socket;
	struct addrinfo* socket_info;
} t_socket;

typedef struct __attribute__((__packed__ )){
	uint32_t msj_type;
	int32_t size;
} t_header;

typedef struct __attribute__((__packed__ )){
	int32_t msj_type;
	uint32_t socket;
	int32_t size;
	void* data;
} t_buffer;

/**
 * @NAME: crear_socket_de_conexion
 * @DESC: Recibe dos strings, uno para la IP y otro para el PUERTO.
 *		  Crea el socket y asgina el identificador. Rellana la estructura
 *		  addrinfo con AF_UNSPEC y SOCK_STREAM, utilizando getaddrinfo.
 */
t_socket crear_socket_de_conexion (char* ip, char* puerto);

/**
 * @NAME: crear_socket_de_escucha
 * @DESC: Recibe un int con el puerto a escuchar.
 *		  Crea el socket y asigna el identificador. Rellena addrinfo
 *		  con AF_INET, INADDR_ANY y el PUERTO.
 */
t_socket crear_socket_de_escucha (char* puerto);

/**
 * @NAME: conectar_socket
 * @DESC: Recibe un socket de conexion y se conecta al servidor definido
 */
bool conectar_socket(t_socket p_socket);

/**
 * @NAME: bind_listen_socket_escucha
 * @DESC: Recibe un socket de escucha.
 * 		  Lo habilita para ser reutilizado por el SO.
 * 		  Bindea el socket al puerto configurado en el struct.
 * 		  Deja en escucha para nuevas conexiones.
 */
bool bind_listen_socket_escucha(t_socket p_socket);

/**
 * @NAME: aceptando_conexiones
 * @DESC: Recibe un socket de escucha.
 * 		  Y genera un nuevo socket cliente, mediante la funcion accept
 */
t_socket aceptando_conexiones(t_socket socket_escucha);

/*@NAME: recibir_mensaje
 *@DESC: Recibe datos y los guarda en el buffer
 *		 hay que deserializar el buffer.data
 */
t_buffer recibir_mensaje(uint32_t un_socket);

/*@NAME: enviar_mensaje
 *@DESC: Envia datos cargados en el buffer
 *		 Datos ya deben venir serializados
 */
bool enviar_mensaje(t_buffer* buffer);

/*@NAME: enviar_mensaje
 *@DESC: Toma las funciones de sockets y se 
 *		 conecta a un servidor.
 */
t_socket conectarse(uint32_t puerto);


/*@NAME: empaquetar_buffer
 *@DESC:  Dado un string a enviar, socket y tipo mensaje
 *		  devuelve un buffer listo para enviar
 */
t_buffer empaquetar_buffer(char* string, uint32_t socket, int msj_type, int emisor);


/*@NAME: empaquetar_buffer
 *@DESC:  Dado un string a enviar, socket y tipo mensaje
 *		  devuelve un buffer listo para enviar
 */
t_buffer escuchar_recibir_cerrar(char* puerto);

#endif

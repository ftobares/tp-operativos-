#ifndef SERIALIZER_H_
#define SERIALIZER_H_

#include <stdint.h>
#include <string.h>
#include "sockets.h"
#include "utiles.h"

typedef enum {
	T_ERROR,
	T_CONSULTAR_RESTAURANTES,
	T_LISTADO_RESTAURANTES,
	T_SELECCIONAR_RESTAURANTE,
	T_OBTENER_RESTAURANTE,
	T_OBTENER_RESTAURANTE_RESPUESTA,
	T_CONSULTAR_PLATOS,
	T_CONSULTAR_PLATOS_RESPUESTA,
	T_CREAR_PEDIDO,
	T_CREAR_PEDIDO_RESPUESTA,
	T_GUARDAR_PEDIDO,
	T_ANIADIR_PLATO,
	T_ANIADIR_PLATO_RESPUESTA,
	T_GUARDAR_PLATO,
	T_CONFIRMAR_PEDIDO,
	T_CONFIRMAR_PEDIDO_SOLO_ID,
	T_CONFIRMAR_PEDIDO_RESPUESTA,
	T_PLATO_LISTO,
	T_CONSULTAR_PEDIDO,
	T_CONSULTAR_PEDIDO_RESPUESTA,
	T_OBTENER_PEDIDO,
	T_OBTENER_PEDIDO_RESPUESTA,
	T_FINALIZAR_PEDIDO,
	T_TERMINAR_PEDIDO,
	T_OBTENER_RECETA,
	T_OBTENER_RECETA_RESPUESTA,
	T_PING,
	T_DATOS_CLIENTE,
	T_DATOS_CLIENTE_RESPUESTA,
	T_DATOS_RESTAURANTE,
	T_RESULTADO_OPERACION
} t_tipo_mensaje;

// Sub structs de uso comun

enum HANDSHAKE { T_APP, T_RESTAURANTE, T_COMANDA, T_SINDICATO, T_CLIENTE, _ERROR };

typedef struct{
	uint32_t x;
	uint32_t y;
} t_posicion_s;

typedef struct
{
	uint32_t id;
	uint32_t afinidad_size;
    char* afinidad;
} t_cocinero_s;

typedef struct
{
	uint32_t id;
	uint32_t nombre_size;
	char* nombre;
	uint32_t precio;
} t_plato_s;

typedef struct
{
	uint32_t paso_size;
	char* paso;
	uint32_t tiempo;
}t_receta_s;
// Structs principales

typedef struct {
	uint32_t result;
} t_ping_s;

typedef struct {
	uint32_t text_size;
	char* text;
} __attribute__((packed)) t_single_text_s;

typedef struct {
	uint32_t nombre_restaurante_size;
	char* nombre_restaurante;
	uint32_t id_pedido;
	uint32_t nombre_plato_size;
	char* nombre_plato;
    uint32_t cantidad;
} __attribute__((packed)) t_guardar_plato_s;

typedef struct {
	uint32_t nombre_restaurante_size;
	char* nombre_restaurante;
	uint32_t id_pedido;
} __attribute__((packed)) t_pedido_s; /* guardar_pedido, confirmar_pedido, obtener_pedido, y finalizar_pedido */

typedef struct 
{
	uint32_t nombre_restaurante_size;
	char* nombre_restaurante;
} __attribute__((packed)) t_nombre_restaurante_s;

typedef struct
{
	uint32_t id;
	uint32_t nombre_size;
	char* nombre;
	t_posicion_s* posicion;
	uint32_t cantCocineros;
	t_list* cocineros;
	uint32_t cantPlatos;
	t_list* platos;
	uint32_t cantidadHornos;
	uint32_t cantidadPedidos;
} __attribute__((packed)) t_obtener_restaurante_respuesta_s;

typedef struct {
	uint32_t nombre_restaurante_size;
	char* nombre_restaurante;
	uint32_t id_pedido;
	uint32_t nombre_plato_size;
	char* nombre_plato;
} __attribute__((packed)) t_plato_listo_s;

typedef struct
{
	uint32_t id_cliente_size;
    char* id_cliente;
    uint32_t posicion_x;
    uint32_t posicion_y;
	uint32_t puerto_escucha_size;
	char* puerto_escucha;
	uint32_t ip_escucha_size;
	char* ip_escucha;
} __attribute__((packed)) t_cliente;

typedef struct{
	enum HANDSHAKE modulo;
} __attribute__((packed)) t_tipo_modulo;

typedef struct
{
	uint32_t nombre_restaurante_size;
    char* nombre_restaurante;
    uint32_t posicion_x;
    uint32_t posicion_y;
    uint32_t ip_size;
    char* ip;
    uint32_t puerto_size;
    char* puerto;
} __attribute__((packed)) t_restaurante_handshake;

typedef struct {
	uint32_t nombre_plato_size;
	char* nombre_plato;
} __attribute__((packed)) t_obtener_receta_s;


typedef struct{
	uint32_t nombre_plato_size;
	char* nombre_plato;
	uint32_t cantidadPasos;
	t_list* pasos;										//pasos --> char*, size y tiempo (uint32_t)
} __attribute__((packed)) t_obtener_receta_respuesta;

typedef struct{
	uint32_t nombre_paso_size;
	char* nombre_paso;
	uint32_t tiempo;									//pasos --> char*, size y tiempo (uint32_t)
} __attribute__((packed)) t_paso_s;

typedef struct{
	uint32_t id_long;
	void* id;
	uint32_t nombre_restaurante_long;
	char* nombre_restaurante;
	uint32_t lista_platos_long;
	t_list* lista_platos; //Ver si esto se puede serializar bien
	uint32_t estado_precio_long;
	uint32_t estado; //Estado = 1 => Listo // Estado = 2 => Sin terminar
	uint32_t precio;
} __attribute__((packed)) t_pedido_respuesta_s;

typedef struct{
	uint32_t listado_size;
	char* listado;
} __attribute__((packed)) t_listado_restaurantes;

typedef struct{
	uint32_t id_cliente_size;
	char* id_cliente;
	uint32_t id_restaurante_size;
	char* id_restaurante;
} __attribute__((packed)) t_seleccionar_restaurante;

typedef struct {
	uint32_t resultado;
}  __attribute__((packed)) t_resultado_operacion;

typedef struct {
	uint32_t cantidad;
    uint32_t cantidadLista;
    char comida[24];
}  __attribute__((packed)) t_obtener_pedido_plato_s;

typedef struct {
	uint32_t estado;
	uint32_t cantidadPlatos;
	t_list* platos; // t_obtener_pedido_plato_s[]
}  __attribute__((packed)) t_obtener_pedido_s;

typedef struct {
	uint32_t nombre_restaurante_size;
	char* nombre_restaurante;
	uint32_t estado;
	uint32_t cantidadPlatos;
	t_list* platos; // List(t_obtener_pedido_plato_s*)
}  __attribute__((packed)) t_consultar_pedido_s;

typedef struct{
	uint32_t id_pedido;
} __attribute__((packed)) t_id_pedido;

typedef struct{
	uint32_t id_pedido;
	uint32_t plato_size;
	char* plato;
} __attribute__((packed)) t_aniadir_plato;


t_buffer crear_buffer(uint32_t msj_type, uint32_t socket, int32_t size,void* data);

t_buffer crear_buffer_sin_cuerpo(uint32_t msj_type, uint32_t socket);

/**
 * @NAME: serializar mensaje
 * @DESC: Recibe un paquete (mensaje), un tipo de mensaje, un tamanio y el socket que se utiliza para enviar mensaje
 * 		  Retorna un buffer para ser enviado.
 */
t_buffer serializar_mensaje(void* paqueteSinSerializar, t_tipo_mensaje tipoMensaje, uint32_t size, uint32_t un_socket);


/**
 * @NAME: deserializar mensaje
 * @DESC: Recibe un puntero a un buffer.
 * 		  Devuelve un puntero a un paquete.
 * 		  El paquete debe ser casteado al paquete esperado
 * 		  y realizar las validaciones que sean necesarias
 */
void* deserializar_mensaje(t_buffer* buffer);

/**
 * @NAME: deserializar un mensaje a tipo t_pedido_s
 * @DESC: Recibe un puntero a un buffer.
 * 		  Devuelve un puntero a t_pedido_s
 */
t_pedido_s* deserializar_pedido(t_buffer* buffer);

/**
 * @NAME: deserializar un mensaje a tipo t_nombre_restaurante_s
 * @DESC: Recibe un puntero a un buffer.
 * 		  Devuelve un puntero a t_nombre_restaurante_s
 */
t_nombre_restaurante_s* deserializar_nombre_restaurante(t_buffer* buffer);

/**
 * @NAME: deserializar un mensaje a tipo t_single_text_s
 * @DESC: Recibe un puntero a un buffer.
 * 		  Devuelve un puntero a tipo t_single_text_s
 */
t_single_text_s* deserializar_single_text(t_buffer* buffer);

/**
 * @NAME: serializar pedido
 * @DESC: Recibe el pedido a serializar y puntero en donde volcar la serializacion
 */
void serializar_pedido(t_pedido_s* paquetePedido, char* paqueteSerializado);

/**
 * @NAME: serializar nombre restaurante
 * @DESC: Recibe el nombre de restaurante a serializar y puntero en donde volcar la serializacion
 */
void serializar_nombre_restaurante(t_nombre_restaurante_s* paqueteNombreRestaurante, char* paqueteSerializado);

/*
 * @NAME: serializar_datos_restaurante
 * @DESC:
 */
void serializar_datos_restaurante(t_obtener_restaurante_respuesta_s* paqueteDatosRestaurante, char* paqueteSerializado);

/**
 * @NAME: deserializar_datos_restaurante
 * @DESC:
 */
t_obtener_restaurante_respuesta_s* deserializar_datos_restaurante(t_buffer* buffer);

 /* @NAME: serializar single_text
 * @DESC: Recibe el t_single_text_s* a serializar y puntero en donde volcar la serializacion
 */
void serializar_single_text(t_single_text_s* paqueteText, char* paqueteSerializado);

t_guardar_plato_s* deserializar_t_guardar_plato(t_buffer* buffer);

char* deserializar_string(t_buffer* buffer, int string_size);
uint32_t deserializar_int(t_buffer* buffer);

#endif

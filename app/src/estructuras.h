#ifndef SRC_ESTRUCTURAS_H_
#define SRC_ESTRUCTURAS_H_

#include "serializer.h"
#include <semaphore.h>

typedef enum {SIN_BLOQUEO, REPARTIDOR_CANSADO, PEDIDO_SIN_TERMINAR} t_bloqueo;

// Repartidor
typedef struct {
	t_posicion* posicion;
	int id;
	int frecuencia_descanso;
	int tiempo_descanso;
	int libre;
	void* pedidoAsignado;
	sem_t* enMovimiento;
	sem_t* enDescanso;
	sem_t* enEspera;
} t_repartidor;

// Pedido Control Block
typedef struct {
	uint32_t id_pedido;
	char* restaurante;
	char* id_cliente;
	int tiempo_espera;
	double estimacion;
	uint32_t ultimaRafaga;
	bool listo;
	t_posicion* posicion_restaurante;
	t_posicion* posicion_cliente;
	t_repartidor* repartidor_asignado;
	t_bloqueo motivoBloqueo;
} t_PCB;

typedef struct {
	t_socket* socketCliente;
	t_buffer* mensaje;
} t_peticion;

typedef struct {
	t_socket* socketRestaurante;
	char* nombre;
	t_posicion* posicion;
	t_list* platos;
	char* ip;
	char* puerto;
} t_restaurante_app;

typedef struct {
	t_socket* socketCliente;
	char* idCliente;
	char* host;
	char* puerto;
	t_posicion* posicion;
	t_restaurante_app* restauranteSeleccionado;
} t_cliente_app;

typedef struct {
	uint32_t id_pedido;
	t_restaurante_app* restaurante;
	t_cliente_app* cliente;
} t_pedido_en_creacion;

// Hilo con data
typedef struct {
	pthread_t* hilo;
	void* data;
} t_thread;
#endif

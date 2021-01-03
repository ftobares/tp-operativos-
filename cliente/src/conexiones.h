#ifndef SRC_CONEXIONES_H_
#define SRC_CONEXIONES_H_

#include "consola.h"
#include "actualizaciones.h"

pthread_mutex_t mutex;
t_queue* cola_actualizaciones;
sem_t hay_actualizacion;
t_list* hilos;
t_socket socket_servidor;

void* levantar_servidor();
void* handshake();
t_buffer cargarCliente();
void* manejar_conexiones(t_socket servidor);
void* cierre_app();

#endif

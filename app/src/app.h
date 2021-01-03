#ifndef SRC_APP_H_
#define SRC_APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <utiles_config.h>
#include "sockets.h"
#include <semaphore.h>
#include "planificador.h"

t_log* logger;
t_app_config* config;
t_socket socketEscucha;

t_list* clientesConectados;
t_list* restaurantesConectados;
t_list* pedidosEnCreacion;

// Liberación de memoria
t_list* segmentosLibres;

// Inicialización
int inicializar();
int conectarseConComanda(char* ip, char* puerto);
int abrirPuertoEscucha(char* puerto);

// Administración de Mensajes
void* atenderPeticiones(void *data);
void* escucharPeticiones(void *data);

// Finalización
void liberarClientesConectados();
void liberarRestaurantesConectados();
void liberarConfig();
void finalizarPrograma();
void liberarSemaforos();

#endif

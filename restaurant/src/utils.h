#ifndef RESTAURANT_UTILS_H
#define RESTAURANT_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <semaphore.h>
#include <stdbool.h>
#include <signal.h>

#include "utiles_config.h"
#include "utiles.h"
#include "sockets.h"
#include "serializer.h"
#include "planificador.h"

#define CONFIG_PATH "./src/restaurant.config"
#define SIN_AFINIDAD "SIN_AFINIDAD"
#define REPOSAR "Reposar"
#define HORNEAR "Hornear"

uint32_t CANTIDAD_PEDIDOS;

t_restaurante_config* config;
t_log* logger;
t_obtener_restaurante_respuesta_s* restaurante;
pthread_mutex_t mutexCantidadPedidos;

void empaquetarNombreRestaurante(t_nombre_restaurante_s* paqueteAEnviar);
void armarHeader(t_header* header, t_tipo_mensaje tipoMensaje, int32_t size);

void leerConfiguracion();
void liberarConfig(t_restaurante_config* config);

int calcularTamanioRestaurant(t_obtener_restaurante_respuesta_s* paqueteAEnviar);

void liberarRestaurante(t_obtener_restaurante_respuesta_s* restaurante);

uint32_t obtenerIdPedido();
void setCantidadPedidos(uint32_t cantidadPedidos);

/*** UNIT TEST ***/
void testPlanificador(char* useCase);
t_obtener_restaurante_respuesta_s* getMockRestaurante(char* nombreRestaurante);

#endif

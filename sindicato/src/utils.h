#ifndef SRC_UTILS_H
#define SRC_UTILS_H

#include "utiles_config.h"
#include "utiles.h"
#include "sockets.h"
#include "serializer.h"

#include <sys/types.h>
#include <sys/stat.h>

t_log* logger;
int iniciar_logger(void);

t_sindicato_config* config;
int iniciar_config(void);

int crear_directorio_si_no_existe(char* path);

char* get_value_from_text(char* text);

t_list* create_cocineros_list(char* _cantidad_cocineros, char* _afinidades);
t_list* create_platos_list(char* _nombres, char* _precios);
t_list* create_pasos_list(char* _nombres, char* _tiempos);

void free_receta(t_receta* receta);
void free_paso(t_paso* paso);
void free_restaurante(t_restaurante* restaurante);
void free_plato(t_plato* plato);
void free_cocinero(t_cocinero* cocinero);
void free_pedido(t_pedido* pedido);
void free_pedido_plato(t_pedido_plato* plato);

#endif
#ifndef UTILES_H_
#define UTILES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models.h"
#include "sockets.h"
#include <readline/readline.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <pthread.h>


enum API { CONSULTAR_RESTAURANTES, SELECCIONAR_RESTAURANTE, OBTENER_RESTAURANTE,
		   CONSULTAR_PLATOS, CREAR_PEDIDO, GUARDAR_PEDIDO, ANIADIR_PLATO,
		   GUARDAR_PLATO, CONFIRMAR_PEDIDO, PLATO_LISTO, CONSULTAR_PEDIDO, OBTENER_PEDIDO,
		   FINALIZAR_PEDIDO, TERMINAR_PEDIDO, OBTENER_RECETA, EXIT, ERROR };

char* leerlinea();

char** leerInstruccion();

enum API obtenerComando(char* comando);

t_posicion* parsePosicion(char* lista);

t_list* parseLista(char* lista);

char* lista_to_text(t_list* lista);

t_list* parseListaPosiciones(char* posiciones);

bool try_string_to_int(char* str, int* result);

//void ejecutarComando(char* input);

char* append(char* comando, char* argumento);

char* intPortToString(int port);

#endif

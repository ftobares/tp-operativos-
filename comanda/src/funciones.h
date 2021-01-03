#ifndef SRC_FUNCIONES_H_
#define SRC_FUNCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <serializer.h>
#include "memoria.h"


int guardarPedido(char* nombreRestaurant, int idPedido);
int guardarPlato(char* nombreRestaurant, int idPedido, char* nombrePlato, int cantidadPlato);
t_obtener_pedido_s* obtenerPedido(char* nombreRestaurant, int idPedido);
int confirmarPedido(char* nombreRestaurant, int idPedido);
int platoListo(char* nombreRestaurant, int idPedido, char* nombrePlato);
int finalizarPedido(char* nombreRestaurant, int idPedido);
void* manejarCliente(t_buffer* buffer);

#endif /* SRC_FUNCIONES_H_ */

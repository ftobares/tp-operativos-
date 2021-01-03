#ifndef SRC_FUNCIONES_H_
#define SRC_FUNCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <utiles.h>
#include "cliente.h"

t_list* pedidos;

typedef struct{
    char* id;
    char* estado;
}i_pedido;

// Funciones de la API

// TODO Modificar tipo de retorno (pongo void para que compile por ahora)
// TODO Ver si los parámetros van todos string o no, ojo porque si bien desde la consola entran como string, esta interfaz se va a tener que respetar en todos los modulos
void consultarRestaurantes();
void seleccionarRestaurante(char* restaurante);
void obtenerRestaurante(char* restaurante);
void consultarPlatos(char* restaurante);
void crearPedido();
void guardarPedido(char* restaurante, char* id_pedido);
void aniadirPlato(char* plato, char* id_pedido);
void guardarPlato(char* restaurante, char* id_pedido, char* comida, char* cantidad);
void confirmarPedido(char* id_pedido, char* restaurante);
void platoListo(char* restaurante, char* id_pedido, char* comida);
void consultarPedido(char* id_pedido);
void obtenerPedido(char* restaurante, char* id_pedido);
void finalizarPedido(char* restaurante, char* id_pedido);
void terminarPedido(char* restaurante, char* id_pedido);
void obtenerReceta(char* plato);

//  FUNCIONES DE CONEXION


#endif /* SRC_FUNCIONES_H_ */

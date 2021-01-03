#ifndef SRC_LOGIC_H
#define SRC_LOGIC_H

#include "utils.h"
#include "fileSystem.h"

void crearRestaurante(t_restaurante* restaurante);
void crearReceta(t_receta* receta);
void guardarPlatoRestaurante(t_buffer* buffer);
void consultarPlatos(t_buffer* buffer);
void guardarPedido(t_buffer* buffer);
void confirmarPedido(t_buffer* buffer);
void obtenerPedido(t_buffer* buffer);
void obtenerRestaurante(t_buffer* buffer);
void platoListo(t_buffer* buffer);
void obtenerReceta(t_buffer* buffer);
void terminarPedido(t_buffer* buffer);

#endif

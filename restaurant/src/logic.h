#ifndef SRC_LOGIC_H
#define SRC_LOGIC_H

#include "utils.h"

t_log* logger;
t_list* pedidosPendientes;
t_socket socketApp;
t_socket socketSindicato;
uint32_t id_pedido_actual;
uint32_t mensaje_anterior;

void* consultarPlatos(t_buffer* buffer);
void* respuestaConsultarPlatos(t_buffer* buffer);
void* crearPedido(t_buffer* buffer);
void* respuestaCrearPedido(t_buffer* buffer);
void* aniadirPlato(t_buffer* buffer);
void* respuestaAniadirPlato(t_buffer* buffer);
void* confirmarPedido(t_buffer* buffer);
void* respuestaConfirmarPedido(t_buffer* buffer);
void* consultarPedido(t_buffer* buffer);
void* respuestaConsultarPedido(t_buffer* buffer);
void* platoListo(char* restaurante, uint32_t id_pedido, char* comida);
void* terminarPedido(uint32_t id_pedido);
void* manejarCliente(t_buffer* buffer);
#endif

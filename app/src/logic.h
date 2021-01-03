#ifndef SRC_LOGIC_H_
#define SRC_LOGIC_H_

#include "app.h"
#include "serializer.h"

// Lógica
void consultarRestaurantes(t_peticion* peticion);
void seleccionarRestaurante(t_peticion* peticion);
void consultarPlatos(t_peticion* peticion);
void crearPedido(t_peticion* peticion);
void aniadirPlato(t_peticion* peticion);
void consultarPedido(t_peticion* peticion);

// Utilidades
void responderConEstado(bool OK, t_socket* socket);
char* obtenerPlatosRestoDefault();

// Administración de conexiones
void guardarConexionCliente(t_peticion* peticion);
void eliminarConexionCliente(t_socket* socketCliente);
char* obtenerRestaurantesConectados();

#endif

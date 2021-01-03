#ifndef RESTAURANTE_SERVER_H
#define RESTAURANTE_SERVER_H

#include "utils.h"

t_socket socketEscucha;
t_list* threadsConexionesEntrantes;

int startupServer();

void procesarMensaje(t_socket* socketCliente);

void finalizarServer();

#endif

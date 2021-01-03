#ifndef SRC_SERVER_H
#define SRC_SERVER_H

//Utiles de Sindicato
#include "utils.h"

//De sindicato
#include "logic.h"

int runServer(void);

void procesarMensaje(t_socket* socketCliente);

#endif

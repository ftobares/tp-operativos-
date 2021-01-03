#ifndef SRC_CONSOLE_H
#define SRC_CONSOLE_H

#include "utils.h"
#include "logic.h"

int runConsole();

void cmd_crearRestaurante(char** parametros);
void cmd_crearReceta(char** parametros);

void cmd_testServer();
void cmd_testGuardarPlato();
void cmd_testGuardarPedido();
void cmd_testPlatoListo();

t_restaurante* parseRestauranteFromParams(char** parametros);
t_receta* parseRecetaFromParams(char** parametros);

#endif
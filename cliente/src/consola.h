#ifndef SRC_CONSOLA_H_
#define SRC_CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utiles.h>
#include <utiles_config.h>
#include <semaphore.h>
#include <serializer.h>

char* hostEscucha;
char* puertoEscucha;

t_log* logger;
t_cliente_config* config;
t_socket socket_cliente_app;
t_socket socket_conectado;
pthread_t consola;
pthread_t servidor;
enum HANDSHAKE moduloConectado;

void* manejar_consola();
void ejecutarComando(char* input);

#endif

#ifndef SRC_RESTAURANT_H_
#define SRC_RESTAURANT_H_

// Dependencias internas
#include "server.h"
#include "logic.h"
#include "utils.h"

// Semaforos
sem_t signalRecibido;
sem_t signalError;

t_socket socketSindicato;
t_socket socketApp;

#endif

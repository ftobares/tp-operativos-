#ifndef SRC_ACTUALIZACIONES_H_
#define SRC_ACTUALIZACIONES_H_

#include "conexiones.h"
#include "consola.h"

void* inicializar_pool();
void* manejar_actualizacion();
void recibir_actualizacion(void* socket_cliente);
void* finalizar_pedido(uint32_t, char*);

#endif
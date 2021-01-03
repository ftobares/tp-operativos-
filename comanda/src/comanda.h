#ifndef SRC_COMANDA_H_guardarPedido
#define SRC_COMANDA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <utiles_config.h>
#include <serializer.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/collections/queue.h>

#include "memoria.h"
#include "funciones.h"
#include "test.c"

uint32_t TAMANIO_FRAME;
uint32_t MAXIMA_CANTIDAD_FRAME_MP;
uint32_t MAXIMA_CANTIDAD_FRAME_SWAP;

// general
t_log* logger;
t_comanda_config* config;

t_list* tablasDeSegmentos; // t_tabla_segmentos[]

t_list* threads;
t_socket* socketEscucha;
pthread_t threadEscucha;

typedef struct {
    t_tipo_mensaje tipo_mensaje;
    void* data;
    uint32_t socket_cliente;
} t_mensaje;

int inicializar();
void liberarConfig();
void finalizar();
int iniciarColas();
void nuevaConexion(int* socketCliente);	
int escuchar();
void liberarColas();
void cerrarSocket();
int inicializarSocketEscuchar();
int reservarEspaciosMemoria();
void liberarHilo(int32_t thread);
void liberarMemorias();
void liberarColas();
void enviarRespuesta(int result, int socketClient);
void enviarRespuestaObtenerPedido(t_obtener_pedido_s* respuesta, int socketClient);
#endif
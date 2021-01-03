#ifndef SRC_MEMORIA_H_
#define SRC_MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <commons/collections/queue.h>

#include <models.h> 

#include <fcntl.h>           // librerias para manejo de archivos
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>


typedef struct {
    int cantidad;
    int cantidadLista;
    char comida[24];
} t_frame;

typedef struct {
   char* nombreRestaurante;
   t_list* segmentos;
} t_tabla_segmentos;

typedef struct {
    uint32_t idPedido;
    estado_pedido estadoPedido;
    t_list* tablaPaginas;
} t_segmento;

typedef struct {
    uint32_t nroFrame;
} t_pagina;

typedef struct {
    uint32_t nroFrame;
    int uso;
    int modificado;
} t_info_frame;

uint32_t TAMANIO_FRAME;
uint32_t MAXIMA_CANTIDAD_FRAME_MP;
uint32_t MAXIMA_CANTIDAD_FRAME_SWAP;
uint32_t TAMANIO_STRING_COMIDA;

t_list* tablasDeSegmentos; // t_tabla_segmentos[]
t_list* framesMp; // t_info_frame []
t_list* framesSwap; // t_info_frame []
t_list* lru; // int[]

int punteroClock; // puntero para algoritmo CLOCK

void* memoriaPrincipal; 
void* swap;

int algoritmo;

int swapfd;

t_log* logger;



t_frame* leerFrame(void* memoria, uint32_t nroFrame);
t_frame* leerFrameDeSwap(uint32_t nroFrame);
t_frame* leerFrameDeMP(uint32_t nroFrame);
t_frame* leerFrameDeMPoSwap(uint32_t nroFrame);
t_frame* leerFrameDeMpByIndice(uint32_t indice);

t_info_frame* obtenerInfoFrameSwap(uint32_t nroFrame);
t_info_frame* obtenerInfoFrameMp(uint32_t nroFrame);
t_info_frame* obtenerInfoFrame(t_list* frames, uint32_t nroFrame);

int finalizarManejoDeMemoria();
int inicializarManejoDeMemoria(uint32_t tamanioMp, uint32_t tamanioSwap, char* pAlgoritmo, t_log* pLogger);

int escribirFrame(void* memoria, t_frame* frame, uint32_t posicion);

int obtenerVictimaReemplazo();

int escribirFrameEnMp(t_frame* frame, uint32_t nroFrame);
int escribirFrameEnSwap(t_frame* frame, uint32_t nroFrame);

int buscarNroFrameDisponible();

void llevarFrameASwap(uint32_t frameMp);

t_tabla_segmentos* obtenerTablaDeSegmentos(char* nombreRestaurante);
t_tabla_segmentos* obtenerCrearTablaSegmentos(char* nombreRestaurante);

t_segmento* obtenerSegmento(t_tabla_segmentos* tablaSegmentos, uint32_t idPedido);
t_segmento* obtenerCrearSegmento(t_tabla_segmentos* tablaSegmentos, uint32_t idPedido);

t_segmento* buscarSegmento(char* nombreRestaurante, uint32_t idPedido, t_tabla_segmentos* tablaSegmentos);

int crearPagina(t_segmento* segmento, char nombrePlato[24], int cantidad);
int crearPaginaSiNoExiste(t_segmento* segmento, char nombrePlato[24], int cantidad);

t_pagina* obtenerPagina(t_segmento* segmento, char nombrePlato[24]);
    
struct stat swaplong(int fileDescriptor);

void liberarPaginasDeSegmento(t_segmento* segmento);

#endif /* SRC_MEMORIA_H_ */

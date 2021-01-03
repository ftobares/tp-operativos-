#ifndef RESTAURANTE_PLANIFICADOR_H
#define RESTAURANTE_PLANIFICADOR_H

#include "utils.h"

// Definiciones de estructuras
typedef struct {
	uint32_t id;
    char* afinidad;
    bool libre;
} t_cocinero_cpu;

// Plato Control Block
typedef struct {
	uint32_t id_pedido;
	uint32_t id_plato;
	t_receta* receta;
	t_cocinero_cpu* cocinero;
	char* tipoBloqueo;
	int ciclosBloqueado;
	int quatumProcesados;
	int pasoActualEnReceta;
	int ciclosProcesadosEnPasoDeReceta;
} t_pcb;

typedef struct {
	bool libre;
	t_pcb* plato;
	sem_t* semaforo;
} t_horno_io;

typedef struct {
	char* afinidad;
	t_queue* afinidadQueue;
	sem_t* semaforo;
	pthread_mutex_t* mutex;
	sem_t* cocineroLibreEnAfinidad;
} t_queue_afinidad;

// Constantes
int RETARDO_CICLO_CPU;
int CANTIDAD_AFINIDADES;

// Queue simples
t_queue* queueNew;
t_queue* queueReady;
t_queue* queueExit;
t_queue* queueHornos;
t_queue* queueExec;
t_queue* queueBlock;

// Listas complejas
t_list* listQueueAfinidades; // Lista de t_queue_afinidad
t_list* listCocineros; // Lista de t_cocinero_cpu
t_list* listHornos; // Lista de t_horno_io

// Lista de Threads
t_list* threadsPlanificador;

// Semaforos Simples
sem_t semPlatosNew;
sem_t semPlatosReady;
sem_t semPlatosBlock;
sem_t semPlatosExec;
sem_t semPlatosExit;
sem_t semQueueHornos;
sem_t semCocinerosSinAfinidad;
sem_t semCPUBurstExec;
sem_t semCPUBurstBlock;
sem_t semCPUBurstIO;

// Mutex simpes
pthread_mutex_t mutexQueueNew;
pthread_mutex_t mutexQueueReady;
pthread_mutex_t mutexQueueBlock;
pthread_mutex_t mutexQueueHornos;
pthread_mutex_t mutexQueueExec;
pthread_mutex_t mutexQueueExit;
pthread_mutex_t mutexCocinerosLibres;

// Startup
int startupPlanificador();

// Core Logic
t_pcb* crearPCB(uint32_t id_pedido, uint32_t id_plato, char* plato, t_list* pasosReceta);
void encolarNuevoPlato(t_pcb* pcb);
void destruirPCB(t_pcb* plato);

// Exit
void finalizarPlanificador();

#endif

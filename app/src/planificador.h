#ifndef SRC_PLANIFICADOR_H_
#define SRC_PLANIFICADOR_H_

#include <pthread.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <semaphore.h>
#include "estructuras.h"

// Datos para la planificación
int RETARDO_CICLO_CPU;
int GRADO_DE_MULTIPROCESAMIENTO;
char* ALGORITMO_DE_PLANIFICACION;
double ALPHA;
int ESTIMACION_INICIAL;

// Restaurante default
t_restaurante_app* restoDefault;

// Colas de planificación
t_list * colaNew;
t_list * colaReady;
t_list * listaExec;
t_list * colaBlock;
t_list * colaExit;

// Lista de repartidores existentes
t_list * repartidores;

// Lista de hilos creados
t_list * threads;

// Logger
t_log* planificador_logger;

// Semaforos
sem_t pedidosNuevos;
sem_t pedidosListos;
sem_t pedidosTerminados;
sem_t pedidosBloqueados;
sem_t repartidoresLibres;
sem_t coresLibres;

// Mutex
pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexColaBlock;
pthread_mutex_t mutexColaExec;
pthread_mutex_t mutexColaExit;
pthread_mutex_t mutexListaThreads;

// Mutex listas temporales
pthread_mutex_t mutexClientesConectados;
pthread_mutex_t mutexRestaurantesConectados;
pthread_mutex_t mutexPedidosEnCreacion;

// Inicialización
int iniciarPlanificador();
int inicializarSemaforosPlanificador();
void inicializarColas();
int crearRepartidores();
void crearRestauranteDefault();
void asignarValoresPlanificacion();
int abrirHilosPlanificador();

// Planificación largo plazo
void* planificarPedidosNuevos(void *data);
void* planificarPedidosTerminados(void *data);

// Planificación corto plazo
void* planificarPedidosListos(void *data);

// Lógica del repartidor
bool estaListo(t_PCB* pedido);
bool esRestauranteDefault(char* restaurante);
void* entregarPedidos(void* data);
void moverseHacia(t_repartidor* repartidor, t_posicion* posicion);
void recorrerCamino(t_repartidor* repartidor, t_posicion* destino);

// Repartidores bloqueados
void* repartidorBloqueado(void *data);
void* planificarPedidosBloqueados(void* data);

// Algoritmos de planificacion
t_PCB* obtenerProximoAEjecutar();
t_PCB* obtenerProximoFIFO();
t_PCB* obtenerProximoHRRN();
t_PCB* obtenerProximoSJF();

// Calculos de planificación
void estimarRafagas();
double calculoHRRN(t_PCB* pcb);
t_PCB* obtenerPedidoRRMasAlto();
t_PCB* obtenerPedidoConEstimacionSJFmasCorta();

// Finalización
void liberarColas();
void liberarRepartidores();
void liberarHilos();
void liberarRestauranteDefault();
void liberarSemaforosPlanificador();

// Utilidades
double calcularDistancia(t_posicion* inicio, t_posicion* fin);
t_repartidor* obtenerRepartidorMasCercano(t_list* repartidoresLibres, t_PCB* pedido);
bool llegoADestino(t_repartidor* repartidor, t_posicion* destino);
t_list* obtenerRepartidoresLibres();

#endif

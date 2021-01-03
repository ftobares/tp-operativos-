#ifndef UTILES_CONFIG_H_
#define UTILES_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>

typedef struct {
	char* ip;
	int puerto;
	char* archivo_log;
	int posicion_x;
	int posicion_y;
	char* id_cliente;
} t_cliente_config;

typedef struct {
	char* puerto;
	char* punto_montaje;
	char* ruta_log;
} t_sindicato_config;

typedef struct {
	char* ip_comanda;
	char* puerto_comanda;
	char* puerto_escucha;
	int retardo_cpu;
	int multiprocesamiento;
	char* algoritmo_planificacion; // FIFO , HRRN , SJF-SD
	double alpha; // Si aplica, para SJF
	int estimacion_inicial; // Si aplica, en SJF
	char* repartidores;
	char* frecuencia_descanso;
	char* tiempo_descanso;
	char* archivo_log;
	char* platos_default;
	int posicion_x_default;
	int posicion_y_default;
} t_app_config;

typedef struct {
	char* puerto_escucha;
	int tamanio_memoria;
	int tamanio_swap;
	char* algoritmo_reemplazo;
	char* archivo_log;
} t_comanda_config;

typedef struct {
	char* puerto_escucha;
	char* ip_sindicato;
	char* puerto_sindicato;
	char* ip_app;
	char* puerto_app;
	int quantum;
	char* archivo_log;
	char* algoritmo_planificacion;
	char* nombre_restaurante;
	int retardo_ciclo_cpu;
} t_restaurante_config;

typedef enum {
	CLIENTE, SINDICATO, RESTAURANTE, APP, COMANDA
} t_tipo_archivo;

void* cargar_configuracion(char* path_archivo, t_tipo_archivo tipo_archivo);

#endif

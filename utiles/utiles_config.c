#include "utiles_config.h"

t_config *config;
t_sindicato_config* sindicato_config;
t_cliente_config* cliente_config;
t_app_config* app_config;
t_comanda_config* comanda_config;
t_restaurante_config* restaurante_config;

bool validar_configuracion(t_config* config) {
	return (config_keys_amount(config) > 0);
}

void* cargar_configuracion(char* path_archivo, t_tipo_archivo tipo_archivo) {

	char* config_path;
	config_path = string_new();
	string_append(&config_path, path_archivo);
	config = config_create(config_path);

	if (!validar_configuracion(config)) {
		printf("ERROR: No se encontró el archivo de configuración.");
		free(config_path);
		free(config); //Libero la memoria de config
	}

	switch (tipo_archivo) {
	case CLIENTE:
		cliente_config = malloc(sizeof(t_cliente_config));
		cliente_config->ip = strdup(config_get_string_value(config, "IP"));
		cliente_config->puerto = config_get_int_value(config,
				"PUERTO");
		cliente_config->archivo_log = strdup(
				config_get_string_value(config, "ARCHIVO_LOG"));
		cliente_config->posicion_x = config_get_int_value(config, "POSICION_X");
		cliente_config->posicion_y = config_get_int_value(config, "POSICION_Y");
		cliente_config->id_cliente = strdup(config_get_string_value(config, "ID_CLIENTE"));
		config_destroy(config);
		free(config_path);
		return cliente_config;
	case SINDICATO:
		sindicato_config = malloc(sizeof(t_sindicato_config));
		sindicato_config->punto_montaje = strdup(config_get_string_value(config, "PUNTO_MONTAJE"));
		sindicato_config->puerto = strdup(config_get_string_value(config,"PUERTO_ESCUCHA"));
		sindicato_config->ruta_log = strdup(config_get_string_value(config, "RUTA_LOG"));
		
		config_destroy(config);
		free(config_path);
		return sindicato_config;
	case RESTAURANTE:
		restaurante_config = malloc(sizeof(t_restaurante_config));
		restaurante_config->puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
		restaurante_config->ip_sindicato = strdup(config_get_string_value(config, "IP_SINDICATO"));
		restaurante_config->puerto_sindicato = strdup(config_get_string_value(config, "PUERTO_SINDICATO"));
		restaurante_config->ip_app = strdup(config_get_string_value(config, "IP_APP"));
		restaurante_config->puerto_app = strdup(config_get_string_value(config, "PUERTO_APP"));
		restaurante_config->quantum = config_get_int_value(config, "QUANTUM");
		restaurante_config->archivo_log = strdup(config_get_string_value(config,"ARCHIVO_LOG"));
		restaurante_config->algoritmo_planificacion = strdup(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
		restaurante_config->nombre_restaurante = strdup(config_get_string_value(config, "NOMBRE_RESTAURANTE"));
		restaurante_config->retardo_ciclo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
		config_destroy(config);
		free(config_path);
		return restaurante_config;
	case COMANDA:
		comanda_config = malloc(sizeof(t_comanda_config));
		comanda_config->tamanio_memoria = config_get_int_value(config, "TAMANIO_MEMORIA");
		comanda_config->puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
		comanda_config->tamanio_swap = config_get_int_value(config, "TAMANIO_SWAP"); 
		comanda_config->algoritmo_reemplazo = strdup(
				config_get_string_value(config, "ALGORITMO_REEMPLAZO"));
		comanda_config->archivo_log = strdup(
				config_get_string_value(config, "ARCHIVO_LOG"));
		config_destroy(config);
		free(config_path);
		return comanda_config;
	case APP:
		app_config = malloc(sizeof(t_app_config));
		app_config->ip_comanda = strdup(config_get_string_value(config, "IP_COMANDA"));
		app_config->puerto_comanda = strdup(config_get_string_value(config, "PUERTO_COMANDA"));
		app_config->puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
		app_config->retardo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
		app_config->multiprocesamiento = config_get_int_value(config, "GRADO_DE_MULTIPROCESAMIENTO");
		app_config->algoritmo_planificacion = strdup(config_get_string_value(config, "ALGORITMO_DE_PLANIFICACION"));
		app_config->alpha = config_get_double_value(config,"ALPHA");
		app_config->estimacion_inicial = config_get_int_value(config,"ESTIMACION_INICIAL");
		app_config->repartidores = strdup(config_get_string_value(config,"REPARTIDORES"));
		app_config->frecuencia_descanso = strdup(config_get_string_value(config,"FRECUENCIA_DE_DESCANSO"));
		app_config->tiempo_descanso = strdup(config_get_string_value(config,"TIEMPO_DE_DESCANSO"));
		app_config->archivo_log = strdup(config_get_string_value(config,"ARCHIVO_LOG"));
		app_config->platos_default = strdup(config_get_string_value(config,"PLATOS_DEFAULT"));
		app_config->posicion_x_default = config_get_int_value(config, "POSICION_REST_DEFAULT_X");
		app_config->posicion_y_default = config_get_int_value(config, "POSICION_REST_DEFAULT_Y");
		config_destroy(config);
		free(config_path);
		return app_config;
	default:
		printf("ERROR cargando configuracion tipo de archivo invalido");
		config_destroy(config);
		return 1;
	}
}

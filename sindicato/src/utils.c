#include "utils.h"

int iniciar_config()  
{
	log_info(logger, "Leyendo configuración");

	config = cargar_configuracion("./src/sindicato.config", SINDICATO);

	log_info(logger, "Config - Puerto escucha: %s", config->puerto);
	log_info(logger, "Config - Punto de montaje: %s", config->punto_montaje);
	log_info(logger, "Config - Ruta log: %s", config->ruta_log);

	return 0;
}

int iniciar_logger()  
{
	logger = log_create("logs/sindicato.log", "sindicato", 1, LOG_LEVEL_INFO);

	log_info(logger, "SINDICATO: Logger inicializado correctamente");

	return 0;
}

int finalizar()
{
	finalizar_file_system();
	liberar_constantes();

	free(config->puerto);
	free(config->punto_montaje);
	free(config->ruta_log);
	free(config);

	log_destroy(logger);
}

int crear_directorio_si_no_existe(char* path)
{
    log_info(logger, "Comprobando si existe directorio %s", path);

	struct stat st = {0};

	if (stat(path, &st) == -1) 
	{
		log_info(logger, "No existe. Creando directorio: %s", path);
    	mkdir(path, 0700);
	}
	else
	{
		log_info(logger, "Directorio %s ya existe", path);	
	}

	return 0;
}

int write_text_file(char* file_path, char* text)
{
	FILE* file = fopen(file_path, "w+");

    if (file == NULL)
    {
        log_error(logger, "Error creando archivo %s", file_path);

        return -1;
    }

    fprintf(file, text);

    fclose(file);

	return 0;
}

/*@NAME: restaurante_text_to_struct
 *@DESC: RECIBE UNA CADENA DE FORMATO:
 			CANTIDAD_COCINEROS=N
			POSICION=[X,Y]
			AFINIDAD_COCINEROS=[...]
			PLATOS=[...]
			PRECIO_PLATOS=[...]
			CANTIDAD_HORNOS=N
		 DEVUELVE EL STRUCT DEL RESTAURANTE*/
t_restaurante* restaurante_text_to_struct (char* nombre, char* text)
{
	char** data = string_split(text, "\n");

	t_restaurante* restaurante = malloc(sizeof(t_restaurante));

	restaurante->nombre = strdup(nombre);

	char* cantidad_cocineros_str = get_value_from_text(data[0]);
	char* posicion_str = get_value_from_text(data[1]);
	char* lista_afinidades_cocineros_str = get_value_from_text(data[2]);
	char* lista_platos_str = get_value_from_text(data[3]);
	char* lista_precio_platos_str = get_value_from_text(data[4]);
	char* cantidad_hornos_str = get_value_from_text(data[5]);

	for (int i = 0; i < 6; i++) free(data[i]);
	free(data);

	restaurante->cocineros = create_cocineros_list(cantidad_cocineros_str, lista_afinidades_cocineros_str);
	restaurante->posicion = parsePosicion(posicion_str);
	restaurante->platos = create_platos_list(lista_platos_str, lista_precio_platos_str);
	restaurante->cantidadHornos = atoi(cantidad_hornos_str);

	free(cantidad_cocineros_str);
	free(lista_afinidades_cocineros_str);
	free(posicion_str);
	free(lista_platos_str);
	free(lista_precio_platos_str);
	free(cantidad_hornos_str);

	return restaurante;
}

/*@NAME: pedido_text_to_struct
 *@DESC: RECIBE UNA CADENA DE FORMATO:
 			ESTADO=XXX
			LISTA_PLATOS=[...]
			CANTIDAD_PEDIDA=[...]
			CANTIDAD_LISTA=[...]
			PRECIO=N
		 DEVUELVE EL STRUCT t_pedido*/
t_pedido* pedido_text_to_struct (char* restaurante, uint32_t id_pedido, char* text)
{
	char** data = string_split(text, "\n");

	t_pedido* pedido = malloc(sizeof(t_pedido));

	pedido->nombre_restaurante = strdup(restaurante);
	pedido->id = id_pedido;
	
	char* estado_str = get_value_from_text(data[0]);
	char* lista_platos_str = get_value_from_text(data[1]);
	char* lista_cantidad_pedida_str = get_value_from_text(data[2]);
	char* lista_cantidad_lista_str = get_value_from_text(data[3]);
	char* precio_str = get_value_from_text(data[4]); 

	for (int i = 0; i < 5; i++) free(data[i]);

	free(data);

	pedido->estado = atoi(estado_str);
	pedido->precio = atoi(precio_str);

	free(estado_str);
	free(precio_str);

	t_list* lista_platos = parseLista(lista_platos_str);
	t_list* lista_cantidad_pedida = parseLista(lista_cantidad_pedida_str);
	t_list* lista_cantidad_lista = parseLista(lista_cantidad_lista_str);

	free(lista_platos_str);
	free(lista_cantidad_pedida_str);
	free(lista_cantidad_lista_str);

	pedido->platos = list_create();

	for (int i = 0; i < list_size(lista_platos); i++)
	{
		t_pedido_plato* plato = malloc(sizeof(t_pedido_plato));

		plato->nombre_plato = list_get(lista_platos, i);
		plato->cantidad_pedida = atoi(list_get(lista_cantidad_pedida, i));
		plato->cantidad_lista = atoi(list_get(lista_cantidad_lista, i));

		list_add(pedido->platos, plato);
	}

	list_destroy(lista_platos);
	list_destroy_and_destroy_elements(lista_cantidad_pedida, &free);
	list_destroy_and_destroy_elements(lista_cantidad_lista, &free);
	
	return pedido;
}

/*@NAME: receta_text_to_struct
 *@DESC: RECIBE UNA CADENA DE FORMATO:
 			PASOS=[...]
			TIEMPO_PASOS=[...]
		 DEVUELVE EL STRUCT DE LA RECETA*/
t_receta* receta_text_to_struct (char* plato, char* text)
{
	char** data = string_split(text, "\n");

	t_receta* receta = malloc(sizeof(t_receta));

	receta->plato = strdup(plato);

	char* pasos = get_value_from_text(data[0]);
	char* tiempo_pasos = get_value_from_text(data[1]);

	free(data[0]);
	free(data[1]);
	free(data);

	receta->pasos = create_pasos_list(pasos, tiempo_pasos);

	free(pasos);
	free(tiempo_pasos);
	
	return receta;
}

/*@NAME: get_value_from_text
 *@DESC: RECIBE UNA CADENA DE FORMATO CAMPO=VALOR Y DEVUELVE EL VALOR. */
char* get_value_from_text(char* text)
{
	int index;

	for (int i = 0; i < strlen(text); i++)
	{
		if (text[i] == '=')
		{
			index = i + 1;
			break;
		}
	}

	char* value = string_substring(text, index, strlen(text) - index);

	return value;
}

/*@NAME: create_cocineros_list
 *@DESC: RECIBE UN CHAR* CON LA CANTIDAD DE COCINEROS Y UN CHAR* EN 
 			FORMATO [...] CON SUS AFINIDADES. DEVUELVE UN t_list* de t_cocinero* */
t_list* create_cocineros_list(char* _cantidad_cocineros, char* _afinidades)
{
	int cantidadCocineros = atoi(_cantidad_cocineros);

	t_list* afinidades = parseLista(_afinidades);	
	t_list* cocineros = list_create();

	for (int i = 0; i < cantidadCocineros; i++)
	{
		t_cocinero* cocinero = malloc(sizeof(t_cocinero));

		cocinero->id = i + 1;

		if (i < list_size(afinidades))
		{
			char* afinidad = list_get(afinidades, i);
			cocinero->afinidad = strdup(afinidad); 
		}
		else
		{
			cocinero->afinidad = NULL;
		}
			
		list_add(cocineros, cocinero);
	}

	list_destroy_and_destroy_elements(afinidades, &free);

	return cocineros;
}

/*@NAME: create_platos_list
 *@DESC: RECIBE UN CHAR* [...] CON LOS NOMBRES DE LOS PLATOS Y UN CHAR* [...] 
 			CON SUS PRECIOS. DEVUELVE UN t_list* de t_plato* */
t_list* create_platos_list(char* _nombres, char* _precios)
{
	t_list* nombres = parseLista(_nombres);	
	t_list* precios = parseLista(_precios);
	
	if (list_size(nombres) != list_size(precios))
	{
		log_error(logger, "No se pudo crear lista de platos. Cantidad de platos != cantidad de precios.");
		return NULL;
	}

	t_list* platos = list_create();

	for (int i = 0; i < list_size(nombres); i++)
	{
		t_plato* plato = malloc(sizeof(t_plato));

		plato->id = i + 1;

		char* nombre_plato = list_get(nombres, i);

		plato->nombre = strdup(nombre_plato);
		plato->precio = atoi(list_get(precios, i));

		list_add(platos, plato);
	}

	list_destroy_and_destroy_elements(nombres, &free);
	list_destroy_and_destroy_elements(precios, &free);

	return platos;
}

/*@NAME: create_pasos_list
 *@DESC: RECIBE UN CHAR* [...] CON LOS NOMBRES DE LOS PASOS Y UN CHAR* [...] 
 			CON SUS TIEMPOS. DEVUELVE UN t_list* de t_paso* */
t_list* create_pasos_list(char* _nombres, char* _tiempos)
{
	log_info(logger, "Creando pasos list %s %s", _nombres, _tiempos);

	t_list* nombres = parseLista(_nombres);	
	t_list* tiempos = parseLista(_tiempos);
	t_list* pasos = list_create();

	if (list_size(nombres) != list_size(tiempos))
	{
		log_error(logger, "Error creando receta. Cantidad de pasos != Cantidad de tiempos");

		return NULL;
	}

	for (int i = 0; i < list_size(nombres); i++)
	{
		t_paso* paso = malloc(sizeof(t_paso));

		char* nombre = list_get(nombres, i);
		
		paso->nombre = strdup(nombre);
		paso->tiempo = atoi(list_get(tiempos, i));

		list_add(pasos, paso);
	}

	list_destroy_and_destroy_elements(nombres, &free);
	list_destroy_and_destroy_elements(tiempos, &free);

	return pasos;
}

void free_receta(t_receta* receta)
{
	list_destroy_and_destroy_elements(receta->pasos, &free_paso);

	free(receta->plato);
	free(receta);
}

void free_paso(t_paso* paso)
{
	free(paso->nombre);
	free(paso);
}

void free_restaurante(t_restaurante* restaurante)
{
	free(restaurante->nombre);
	free(restaurante->posicion);
	list_destroy_and_destroy_elements(restaurante->cocineros, &free_cocinero);
	list_destroy_and_destroy_elements(restaurante->platos, &free_plato);
	free(restaurante);
}

void free_cocinero(t_cocinero* cocinero)
{
	if(cocinero->afinidad != NULL) free(cocinero->afinidad);
	free(cocinero);
}

void free_plato(t_plato* plato)
{
	free(plato->nombre);
	free(plato);
}

void free_pedido(t_pedido* pedido)
{
	free(pedido->nombre_restaurante);

	list_destroy_and_destroy_elements(pedido->platos, &free_pedido_plato);

	free(pedido);
}

void free_pedido_plato(t_pedido_plato* plato)
{
	free(plato->nombre_plato);
	free(plato);
}

void restaurante_to_lower(t_restaurante* restaurante)
{
	string_to_lower(restaurante->nombre);

	for (int i = 0; i < list_size(restaurante->cocineros); i++)
	{
		t_cocinero* cocinero = list_get(restaurante->cocineros, i);

		if (cocinero->afinidad != NULL) string_to_lower(cocinero->afinidad);
	}

	for (int i = 0; i < list_size(restaurante->platos); i++)
	{
		t_plato* plato = list_get(restaurante->platos, i);

		string_to_lower(plato->nombre);
	}
}

void receta_to_lower(t_receta* receta)
{
	string_to_lower(receta->plato);

	for (int i = 0; i < list_size(receta->pasos); i++)
	{
		t_paso* paso = list_get(receta->pasos, i);

		string_to_lower(paso->nombre);
	}
}
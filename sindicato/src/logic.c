#include "logic.h"

void crearRestaurante(t_restaurante* restaurante)
{
	log_info(logger, "Creando restaurante: %s", restaurante->nombre);
	
	restaurante_to_lower(restaurante);

	guardar_restaurante(restaurante);

	free_restaurante(restaurante);
}

void crearReceta(t_receta* receta)
{
	log_info(logger, "Creando receta: %s", receta->plato);
	
	receta_to_lower(receta);

	guardar_receta(receta);
}

void guardarPlatoRestaurante(t_buffer* buffer)
{
	log_info(logger, "Guardando plato");

	t_guardar_plato_s* paquete = (t_guardar_plato_s*) deserializar_mensaje(buffer);

	log_info(logger, "ID Pedido: %d", paquete->id_pedido);
	log_info(logger, "Nombre plato: %s", paquete->nombre_plato);
	log_info(logger, "Nombre Restaurante: %s", paquete->nombre_restaurante);
	log_info(logger, "Cantidad del plato: %d", paquete->cantidad);
	
	string_to_lower(paquete->nombre_plato);
	string_to_lower(paquete->nombre_restaurante);

	int result = guardar_plato(paquete);

	if (result == 0) enviar_respuesta_resultado_operacion(buffer, 1);
	else enviar_respuesta_resultado_operacion(buffer, 0);

	free(paquete->nombre_plato);
	free(paquete->nombre_restaurante);
	free(paquete);
}

void consultarPlatos(t_buffer* buffer)
{
	t_nombre_restaurante_s* paquete = (t_nombre_restaurante_s*) deserializar_mensaje(buffer);

	log_info(logger, "ConsultarPlatos Restaurante: %s", paquete->nombre_restaurante);
	
	string_to_lower(paquete->nombre_restaurante);

	t_restaurante* restaurante = get_restaurante(paquete->nombre_restaurante);

	free(paquete->nombre_restaurante);
	free(paquete);

	if (restaurante == NULL)
	{
		log_error(logger, "No se pueden consultar los platos porque no se encontró el restaurante.");
	}

	t_list* lista_nombres_platos = list_create();

	for(int i = 0; i < list_size(restaurante->platos); i++)
	{
		t_plato* plato = list_get(restaurante->platos, i);
		list_add(lista_nombres_platos, plato->nombre);
	}
	
	char* text = lista_to_text(lista_nombres_platos);

	list_destroy_and_destroy_elements(lista_nombres_platos, &free);

	log_info(logger, "Enviando respuesta");

	t_single_text_s* paquete_rta = malloc(sizeof(t_single_text_s));

	paquete_rta->text_size = strlen(text) + 1;
	paquete_rta->text = strdup(text);

	log_info(logger, "Respuesta a Restaurante: Text %s", paquete_rta->text);

	t_header* header = malloc(sizeof(t_header));
			
	//Defino que tipo de mensaje se envia
	header->msj_type = T_CONSULTAR_PLATOS_RESPUESTA;

	//Seteo el tamanio total del mensaje
	header->size = (sizeof(uint32_t) * 1) + paquete_rta->text_size;

	t_buffer buffer_rta = serializar_mensaje(paquete_rta, header->msj_type, header->size, buffer->socket);

	enviar_mensaje(&buffer_rta);

	log_info(logger, "Respuesta enviada");

	free(paquete_rta->text);
	free(paquete_rta);
	free(header);
	free(buffer_rta.data);
	free(text);
}

void guardarPedido(t_buffer* buffer)
{
	t_pedido_s* paquete = (t_pedido_s*) deserializar_mensaje(buffer);

	log_info(logger, "Guardando Pedido ID: %d", paquete->id_pedido);
	log_info(logger, "Restaurante: %s", paquete->nombre_restaurante);
	
	string_to_lower(paquete->nombre_restaurante);

	int result = guardar_pedido(paquete);

	free(paquete->nombre_restaurante);
	free(paquete);
	
	if (result == 0) enviar_respuesta_resultado_operacion(buffer, 1);
	else enviar_respuesta_resultado_operacion(buffer, 0);
}

void confirmarPedido(t_buffer* buffer)
{
	t_pedido_s* paquete = (t_pedido_s*) deserializar_mensaje(buffer);

	log_info(logger, "ID Pedido: %d\n", paquete->id_pedido);
	log_info(logger, "Nombre Restaurante: %s\n", paquete->nombre_restaurante);
	
	string_to_lower(paquete->nombre_restaurante);

	int result = confirmar_pedido(paquete);
	
	if (result == 0) enviar_respuesta_resultado_operacion(buffer, 1);
	else enviar_respuesta_resultado_operacion(buffer, 0);
	
	free(paquete->nombre_restaurante);
	free(paquete);
}

void obtenerPedido(t_buffer* buffer)
{
	t_pedido_s* paquete = (t_pedido_s*) deserializar_mensaje(buffer);

	log_info(logger, "Obteniendo Pedido: %d\n", paquete->id_pedido);
	log_info(logger, "Nombre Restaurante: %s\n", paquete->nombre_restaurante);

	string_to_lower(paquete->nombre_restaurante);

	t_pedido* pedido = get_pedido(paquete);

	free(paquete->nombre_restaurante);
	free(paquete);

	t_obtener_pedido_s* paquete_rta = malloc(sizeof(t_obtener_pedido_s));
	
	int size_rta = sizeof(t_obtener_pedido_s);

	paquete_rta->estado = pedido->estado;
	paquete_rta->cantidadPlatos = list_size(pedido->platos);
	paquete_rta->platos = list_create();

	for(int i = 0; i < list_size(pedido->platos); i++)
	{
		t_obtener_pedido_plato_s* plato = malloc(sizeof(t_obtener_pedido_plato_s));
		
		t_pedido_plato* _plato = list_get(pedido->platos, i);

		strcpy(plato->comida, _plato->nombre_plato); 
		plato->cantidad = _plato->cantidad_pedida;
		plato->cantidadLista = _plato->cantidad_lista;

		size_rta += sizeof(t_obtener_pedido_plato_s);

		list_add(paquete_rta->platos, plato);
	}
	
	free_pedido(pedido);

	log_info(logger, "Enviando respuesta");

	t_header* header = malloc(sizeof(t_header));
			
	//Defino que tipo de mensaje se envia
	header->msj_type = T_OBTENER_PEDIDO_RESPUESTA;

	//Seteo el tamanio total del mensaje
	header->size = size_rta;

	t_buffer buffer_rta = serializar_mensaje(paquete_rta, header->msj_type, header->size, buffer->socket);

	enviar_mensaje(&buffer_rta);

	log_info(logger, "Respuesta enviada");

	list_destroy_and_destroy_elements(paquete_rta->platos, &free);
	free(paquete_rta);
	free(header);
	free(buffer_rta.data);
}

void obtenerRestaurante(t_buffer* buffer)
{
	t_nombre_restaurante_s* paquete = (t_nombre_restaurante_s*) deserializar_mensaje(buffer);
	
	log_info(logger, "Obtener restaurante: %s\n", paquete->nombre_restaurante);
	
	string_to_lower(paquete->nombre_restaurante);

	t_restaurante* restaurante = get_restaurante(paquete->nombre_restaurante);

	if (restaurante == NULL)
	{
		log_error(logger, "No se encontró el restaurante %s", paquete->nombre_restaurante);
	}

	free(paquete->nombre_restaurante);
	free(paquete);

	t_obtener_restaurante_respuesta_s* paquete_rta = malloc(sizeof(t_obtener_restaurante_respuesta_s));

	int size_rta = sizeof(t_obtener_restaurante_respuesta_s);

	if (restaurante != NULL)
	{
		paquete_rta->id = 1;
		paquete_rta->nombre_size = strlen(restaurante->nombre) + 1; size_rta += paquete_rta->nombre_size; 
		paquete_rta->nombre = strdup(restaurante->nombre);
		paquete_rta->posicion = malloc(sizeof(t_posicion_s));
		paquete_rta->posicion->x = restaurante->posicion->x;
		paquete_rta->posicion->y = restaurante->posicion->y; 
		paquete_rta->cantCocineros = list_size(restaurante->cocineros);
		paquete_rta->cocineros = list_create();

		for (int i = 0; i < paquete_rta->cantCocineros; i++)
		{
			t_cocinero_s* cocinero = malloc(sizeof(t_cocinero_s)); size_rta += sizeof(t_cocinero_s);
			t_cocinero* cocineroOK = list_get(restaurante->cocineros, i); 
			
			cocinero->id = cocineroOK->id;
			
			if (cocineroOK->afinidad != NULL)
			{
				cocinero->afinidad_size = strlen(cocineroOK->afinidad) + 1; size_rta += cocinero->afinidad_size;
				cocinero->afinidad = strdup(cocineroOK->afinidad);  
			}
			else
			{
				cocinero->afinidad_size = string_length("SIN_AFINIDAD") + 1; size_rta += cocinero->afinidad_size;
				cocinero->afinidad = strdup("SIN_AFINIDAD");  
			}

			list_add(paquete_rta->cocineros, cocinero);
		}
		
		paquete_rta->cantPlatos = list_size(restaurante->platos);
		paquete_rta->platos = list_create();

		for (int i = 0; i < paquete_rta->cantPlatos; i++)
		{
			t_plato_s* plato = malloc(sizeof(t_plato_s)); size_rta += sizeof(t_plato_s);
			
			t_plato* platoOK = list_get(restaurante->platos, i); 

			plato->id = platoOK->id;
			plato->precio = platoOK->precio;
			plato->nombre_size = strlen(platoOK->nombre) + 1; size_rta += plato->nombre_size;
			plato->nombre = strdup(platoOK->nombre);

			list_add(paquete_rta->platos, plato);
		}

		paquete_rta->cantidadHornos = restaurante->cantidadHornos;
		paquete_rta->cantidadPedidos = get_cantidad_pedidos(restaurante->nombre);

		free_restaurante(restaurante);
	}
	else
	{
		paquete_rta->id = -1;
		paquete_rta->nombre_size = strlen("NOT_FOUND") + 1; size_rta += paquete_rta->nombre_size; 
		paquete_rta->nombre = strdup("NOT_FOUND");
		paquete_rta->posicion = malloc(sizeof(t_posicion_s));
		paquete_rta->posicion->x = 0;
		paquete_rta->posicion->y = 0; 
		paquete_rta->cantCocineros = 0;
		paquete_rta->cocineros = list_create();
		paquete_rta->cantPlatos = 0;
		paquete_rta->platos = list_create();
		paquete_rta->cantidadHornos = 0;
		paquete_rta->cantidadPedidos = 0;
	}

	t_header* header = malloc(sizeof(t_header));
			
	//Defino que tipo de mensaje se envia
	header->msj_type = T_OBTENER_RESTAURANTE_RESPUESTA;

	//Seteo el tamanio total del mensaje
	header->size = size_rta;

	t_buffer buffer_rta = serializar_mensaje(paquete_rta, header->msj_type, header->size, buffer->socket);

	enviar_mensaje(&buffer_rta);

	log_info(logger, "Respuesta enviada");
	
	free(paquete_rta->nombre);
	free(paquete_rta->posicion);
	for (int i = 0; i < paquete_rta->cantCocineros; i++)
	{
		t_cocinero_s* cocinero = list_get(paquete_rta->cocineros, i); 
		if (cocinero->afinidad != NULL) free(cocinero->afinidad);
	}
	list_destroy_and_destroy_elements(paquete_rta->cocineros, &free);
	for (int i = 0; i < paquete_rta->cantPlatos; i++)
	{
		t_plato_s* plato = list_get(paquete_rta->platos, i); 
		free(plato->nombre);
	}
	list_destroy_and_destroy_elements(paquete_rta->platos, &free);
	free(paquete_rta);
	free(header);
	free(buffer_rta.data);
}

void platoListo(t_buffer* buffer)
{
	t_plato_listo_s* paquete = (t_plato_listo_s*) deserializar_mensaje(buffer);

	log_info(logger, "Nombre Restaurante: %s\n", paquete->nombre_restaurante);
	log_info(logger, "ID Pedido: %d\n", paquete->id_pedido);
	log_info(logger, "Nombre Plato: %s\n", paquete->nombre_plato);
	
	string_to_lower(paquete->nombre_restaurante);
	string_to_lower(paquete->nombre_plato);

	int result = plato_listo(paquete);
	
	if (result == 0) enviar_respuesta_resultado_operacion(buffer, 1);
	else enviar_respuesta_resultado_operacion(buffer, 0);

	free(paquete->nombre_restaurante);
	free(paquete->nombre_plato);
	free(paquete);
}

void obtenerReceta(t_buffer* buffer)
{
	t_obtener_receta_s* paquete = (t_obtener_receta_s*) deserializar_mensaje(buffer);

	log_info(logger, "Obteniendo receta %s", paquete->nombre_plato);

	string_to_lower(paquete->nombre_plato);

	t_receta* receta = get_receta(paquete);

	t_obtener_receta_respuesta* paquete_rta = malloc(sizeof(t_obtener_receta_respuesta));

	int size_rta = sizeof(t_obtener_receta_respuesta);

	if (receta == NULL)
	{
		paquete_rta->nombre_plato_size = 6; size_rta += 6;
		paquete_rta->nombre_plato = strdup("ERROR");
		paquete_rta->cantidadPasos = 0;
		paquete_rta->pasos = list_create();
	}
	else
	{
		paquete_rta->nombre_plato_size = strlen(receta->plato) + 1; 
		size_rta += paquete_rta->nombre_plato_size;
		paquete_rta->nombre_plato = strdup(receta->plato);

		paquete_rta->cantidadPasos = list_size(receta->pasos);
		paquete_rta->pasos = list_create();

		for (int i = 0; i < paquete_rta->cantidadPasos; i++)
		{
			t_paso* _paso = list_get(receta->pasos, i);

			t_paso_s* paso = malloc(sizeof(t_paso_s));

			paso->nombre_paso_size = strlen(_paso->nombre) + 1;
			paso->nombre_paso = strdup(_paso->nombre);
			paso->tiempo = _paso->tiempo;

			size_rta += sizeof(t_paso_s);
			size_rta += paso->nombre_paso_size;

			list_add(paquete_rta->pasos, paso);
		}

		free_receta(receta);
	}

	t_header* header = malloc(sizeof(t_header));
			
	//Defino que tipo de mensaje se envia
	header->msj_type = T_OBTENER_RECETA_RESPUESTA;

	//Seteo el tamanio total del mensaje
	header->size = size_rta;

	t_buffer buffer_rta = serializar_mensaje(paquete_rta, header->msj_type, header->size, buffer->socket);

	enviar_mensaje(&buffer_rta);

	log_info(logger, "Respuesta enviada");

	free(paquete_rta->nombre_plato);
	for (int i = 0; i < paquete_rta->cantidadPasos; i++)
	{
		t_paso_s* paso = list_get(paquete_rta->pasos, i);
		free(paso->nombre_paso);
	}
	list_destroy_and_destroy_elements(paquete_rta->pasos, &free);
	free(paquete_rta);
	free(header);
	free(buffer_rta.data);

	free(paquete->nombre_plato);
	free(paquete);
}

void terminarPedido(t_buffer* buffer)
{
	log_info(logger, "Deserializando buffer de terminarPedido");

	t_pedido_s* paquete = (t_pedido_s*) deserializar_mensaje(buffer);
	
	log_info(logger, "ID Pedido: %d\n", paquete->id_pedido);
	log_info(logger, "Nombre Restaurante: %s\n", paquete->nombre_restaurante);
	
	string_to_lower(paquete->nombre_restaurante);

	int result = terminar_pedido(paquete);

	if (result == 0) enviar_respuesta_resultado_operacion(buffer, 1);
	else enviar_respuesta_resultado_operacion(buffer, 0);

	free(paquete->nombre_restaurante);
	free(paquete);
}

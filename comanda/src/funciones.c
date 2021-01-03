#include "funciones.h"

int guardarPedido(char* nombreRestaurant, int idPedido) {

	log_info(logger, "Ejecutando guardar pedido. Nombre restaurante: '%s', ID pedido: %d", nombreRestaurant, idPedido );

	if (strlen(nombreRestaurant) > 24) {
		log_info(logger, "Nombre del restaurante no puede exceder los 24 caracteres");
		return EXIT_FAILURE;
	}

	// obtengo/creo la TS
	t_tabla_segmentos* tablaSegmentos = obtenerCrearTablaSegmentos(nombreRestaurant);

	// hubo un error
    if (tablaSegmentos == NULL) {
		log_info(logger, "No se pudo crear/obtener la tabla de segmentos para el restaurante '%s'", nombreRestaurant);
		return EXIT_FAILURE;
	}

	if (obtenerCrearSegmento(tablaSegmentos, idPedido) == NULL) {
		return EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}

int guardarPlato(char* nombreRestaurant, int idPedido, char* nombrePlato, int cantidadPlato) {

	log_info(logger, "Ejecutando guardar plato. Nombre restaurante: '%s', ID pedido: %d, Nombre plato: '%s', Cantidad plato: %d", nombreRestaurant, idPedido, nombrePlato, cantidadPlato);
	if (strlen(nombreRestaurant) > 24) {
		log_info(logger, "Nombre del restaurante no puede exceder los 24 caracteres");
		return EXIT_FAILURE;
	}

	// busco tabla de segmentos 
	t_tabla_segmentos* tablaSegmentos = obtenerTablaDeSegmentos(nombreRestaurant);
	if (tablaSegmentos == NULL) {
		log_info(logger, "No exite la tabla de segmentos para el restaurante '%s'", nombreRestaurant);
		return EXIT_FAILURE;
	};
	
	// busco segmento
	t_segmento* segmento = obtenerSegmento(tablaSegmentos, idPedido);
	if (segmento == NULL)  {
		log_info(logger, "No exite el segmento con ID pedido %d", idPedido);
		return EXIT_FAILURE;
	}

	// creo la pagina si corresponde
	return crearPaginaSiNoExiste(segmento, nombrePlato, cantidadPlato);
}

t_obtener_pedido_s* obtenerPedido(char* nombreRestaurant, int idPedido) {

	log_info(logger, "Ejecutando obtener pedido. Nombre restaurante: '%s', ID pedido: %d", nombreRestaurant, idPedido );
	if (strlen(nombreRestaurant) > 24) {
		log_info(logger, "Nombre del restaurante no puede exceder los 24 caracteres");
		return NULL;
	}

	// obtengo tabla de segmentos
	t_tabla_segmentos* tablaSegmentos = obtenerTablaDeSegmentos(nombreRestaurant);

	if (tablaSegmentos == NULL) {
		log_info(logger, "No exite la tabla de segmentos para el restaurante '%s'", nombreRestaurant);
		return NULL;
	}
	
	// obtengo segmento
	t_segmento* segmento_pedido = obtenerSegmento(tablaSegmentos, idPedido);

	if(segmento_pedido == NULL) {
		log_info(logger, "No existe el segmento con ID pedido %d", idPedido);
		return NULL;
	}

	// preparo respuesta
	t_obtener_pedido_s* pedido = malloc(sizeof(t_obtener_pedido_s));
	pedido->estado = segmento_pedido->estadoPedido;
	pedido->platos = list_create();

	int traerDatos(t_pagina* pag) {
		t_frame* leido = leerFrameDeMPoSwap(pag->nroFrame);
		t_obtener_pedido_plato_s* plato = malloc(sizeof(t_obtener_pedido_plato_s));
		plato->cantidad = leido->cantidad;
		plato->cantidadLista = leido->cantidadLista;
		strcpy(plato->comida,leido->comida);
		list_add(pedido->platos, plato);
	}

	// sobre cada pagina de la TP, leo el frame
	list_iterate(segmento_pedido->tablaPaginas, (int)traerDatos);

	// no existe platos
	if(list_size(pedido->platos) == 0) {
		log_info(logger, "No existe platos para el ID pedido %d", idPedido);
		list_destroy(pedido->platos);
		return NULL;
	}

	pedido->cantidadPlatos = list_size(pedido->platos);
	return pedido;
}

int confirmarPedido(char* nombreRestaurant, int idPedido) {

	log_info(logger, "Ejecutando confirmar pedido. Nombre restaurante: '%s', ID pedido: %d", nombreRestaurant, idPedido );

	if (strlen(nombreRestaurant) > 24) {
		log_info(logger, "Nombre del restaurante no puede exceder los 24 caracteres");
		return EXIT_FAILURE;
	}

	// obtengo tabla de segmentos
	t_tabla_segmentos* tablaSegmentos = obtenerTablaDeSegmentos(nombreRestaurant);

	if (tablaSegmentos == NULL) {
		log_info(logger, "No exite la tabla de segmentos para el restaurante '%s'", nombreRestaurant);
		return EXIT_FAILURE;
	}
	
	// obtengo segmento
	t_segmento* segmento = obtenerSegmento(tablaSegmentos, idPedido);

	if (segmento == NULL){
		log_info(logger, "No exite el segmento con ID pedido %d", idPedido);
		return EXIT_FAILURE;
	}

	// solo puedo confirmar si esta en PENDIENTE
	if (segmento->estadoPedido != PEDIDO_PENDIENTE) {
		log_info(logger, "Solo se pueden confirmar pedidos en estado PENDIENTE");
		return EXIT_FAILURE;
	}

	// marco como confirmado 
	segmento->estadoPedido = PEDIDO_CONFIRMADO;
	return EXIT_SUCCESS;
}

int platoListo(char* nombreRestaurant, int idPedido, char* nombrePlato) {

	log_info(logger, "Ejecutando plato listo. Nombre restaurante: '%s', ID pedido: %d", nombreRestaurant, idPedido );
	if (strlen(nombreRestaurant) > 24) {
		log_info(logger, "Nombre del restaurante no puede exceder los 24 caracteres");
		return EXIT_FAILURE;
	}

	// obtengo tabla de segmentos
	t_tabla_segmentos* tablaSegmentos = obtenerTablaDeSegmentos(nombreRestaurant);
	if (tablaSegmentos == NULL) {
		log_info(logger, "No exite la tabla de segmentos para el restaurante '%s'", nombreRestaurant);
		return EXIT_FAILURE;
	}
	
	// obtengo segmento
	t_segmento* segmento = obtenerSegmento(tablaSegmentos, idPedido);
	if (segmento == NULL) {
		log_info(logger, "No exite el segmento con ID pedido %d", idPedido);
		return EXIT_FAILURE;
	}
	
	// obtengo pagina
	t_pagina* pagina = obtenerPagina(segmento, nombrePlato);

	if (pagina == NULL) {
		log_info(logger, "No exite pagina para el plato '%s'", nombrePlato);
		return EXIT_FAILURE;
	}

	// leo frame para el NRO de frame (aca se hace el reemplazo si corresponde)
	t_frame* frame = leerFrameDeMPoSwap(pagina->nroFrame);

	// valido si esta en estado CONFIRMADO
	if (segmento->estadoPedido != PEDIDO_CONFIRMADO) {
		log_info(logger, "El pedido %d no se encuentra confirmado.", idPedido);
		return EXIT_FAILURE;
	}

	// sumo cantidad
	frame->cantidadLista += 1;

	// marco frame como modificado por si luego se selecciona como victima
	// de esta manera, antes de borrar de MP, llevo a SWAP
	t_info_frame* infoFrame =  obtenerInfoFrameMp(pagina->nroFrame);
	infoFrame->modificado = 1;

	// me fijo si estan todos los platos terminados
	bool pedidoTerminado = true;
	void checkCantidad(t_pagina* pagina) {
		t_frame* framePagina = leerFrameDeMPoSwap(pagina->nroFrame);
		if (framePagina->cantidadLista != framePagina->cantidad) {
			pedidoTerminado = false;
		}
	}

 	list_iterate(segmento->tablaPaginas,checkCantidad);

	// si estan, paso a estado TERMINADO
	if (pedidoTerminado) {
		segmento->estadoPedido = PEDIDO_TERMINADO;
	}

	return EXIT_SUCCESS;
}

int finalizarPedido(char* nombreRestaurant, int idPedido) {

	log_info(logger, "Ejecutando finalizar pedido. Nombre restaurante: '%s', ID pedido: %d", nombreRestaurant, idPedido );
	if (strlen(nombreRestaurant) > 24) {
		log_info(logger, "Nombre del restaurante no puede exceder los 24 caracteres");
		return EXIT_FAILURE;
	}
	// obtengo tabla de segmentos
	t_tabla_segmentos* tablaSegmentos = obtenerTablaDeSegmentos(nombreRestaurant);

	if (tablaSegmentos == NULL) {
		log_info(logger, "No exite la tabla de segmentos para el restaurante '%s'", nombreRestaurant);
		return EXIT_FAILURE;
	}

	// obtengo segmento
	t_segmento* segmento = obtenerSegmento(tablaSegmentos, idPedido);

	if (segmento == NULL){
		return EXIT_FAILURE;
	}

	int buscarSegmento(t_segmento* segmento){
		return segmento->idPedido == segmento->idPedido;
    }

	// libero las paginas del segmento
	liberarPaginasDeSegmento(segmento);

	// libero segmento de la tabla de segmentos
	list_remove_and_destroy_by_condition(tablaSegmentos->segmentos, buscarSegmento, &free);

	return EXIT_SUCCESS;
}

void* manejarCliente(t_buffer* buffer){
	t_cliente* cliente = deserializar_mensaje(buffer);

	//hago algo con estos datos?

	t_tipo_modulo* modulo = malloc(sizeof(t_tipo_modulo));

	modulo->modulo = T_COMANDA;
	uint32_t size = sizeof(modulo->modulo);
	t_buffer enviar = serializar_mensaje(modulo,T_DATOS_CLIENTE_RESPUESTA,size,buffer->socket);

	if(!enviar_mensaje(&enviar)){
		log_error(logger,"No se pudo enviar el tipo de modulo al cliente.");
	}
	free(modulo);
	free(enviar.data);
}
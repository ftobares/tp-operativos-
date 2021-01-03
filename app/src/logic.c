#include "logic.h"
#include <sys/time.h>

// === Utilidades ===

// Printing
void imprimirPedido(t_obtener_pedido_s* pedido){
	log_info(logger,"Datos del pedido recuperado:");
	log_info(logger,"Estado = %d", pedido->estado);
	log_info(logger,"Cantidad de platos = %d",pedido->cantidadPlatos);
	log_info(logger,"Longitud de la lista: %d", list_size(pedido->platos));
	for(int i=0; i<pedido->cantidadPlatos; i++){
		t_obtener_pedido_plato_s* plato = list_get(pedido->platos,i);
		log_info(logger,"Plato %d", (i+1));
		log_info(logger,"--> Comida: %s", plato->comida);
		log_info(logger,"--> Cantidad: %d", plato->cantidad);
		log_info(logger,"--> Cantidad lista: %d", plato->cantidadLista);
	}
}

void imprimirCliente(t_cliente_app* cliente){
	log_info(logger,"Datos del cliente:");
	log_info(logger,"--> ID = %s", cliente->idCliente);
	log_info(logger,"--> Posicion = %d|%d",cliente->posicion->x, cliente->posicion->y);
	log_info(logger,"--> Host: %s", cliente->host);
	log_info(logger,"--> Puerto: %s", cliente->puerto);
}

void imprimirRestaurante(t_restaurante_app* restaurante){

	log_info(logger,"Datos del restaurante:");
	log_info(logger,"--> Nombre = %s",restaurante->nombre);
	log_info(logger,"--> Posicion = %d|%d",restaurante->posicion->x,restaurante->posicion->y);
	log_info(logger,"--> Host = %s",restaurante->ip);
	log_info(logger,"--> Puerto = %s",restaurante->puerto);
}

// Busquedas
t_cliente_app* obtenerClienteSegunSocket(uint32_t socket){
	bool tieneElMismoSocket(t_cliente_app* cliente){
		return cliente->socketCliente->socket == socket;
	}
	return (t_cliente_app*) list_find(clientesConectados,(void*) tieneElMismoSocket);
}

t_restaurante_app* obtenerRestauranteSegunSocket(uint32_t socket){
	bool tieneElMismoSocket(t_restaurante_app* restaurante){
		return restaurante->socketRestaurante->socket == socket;
	}
	return (t_restaurante_app*) list_find(restaurantesConectados,(void*) tieneElMismoSocket);
}

t_cliente_app* obtenerClienteSegunId(char* id){
	bool tieneIdCliente(t_cliente_app* cliente){
		return string_equals_ignore_case(cliente->idCliente, id);
	}
	return (t_cliente_app*) list_find(clientesConectados,(void*)tieneIdCliente);
}

t_restaurante_app* obtenerRestauranteSegunNombre(char* nombre){
	bool tieneIdRestaurante(t_restaurante_app* restauranteApp){
		return string_equals_ignore_case(restauranteApp->nombre, nombre);
	}
	return (t_restaurante_app*) list_find(restaurantesConectados,(void*)tieneIdRestaurante);
}

t_list* obtenerClientesQueEligieronRestaurante(char* nombreRestaurante){
	bool clienteTieneAlRestaurante(t_cliente_app* cliente){
		return cliente->restauranteSeleccionado != NULL && string_equals_ignore_case(cliente->restauranteSeleccionado->nombre, nombreRestaurante);
	}
	return list_filter(clientesConectados,(void*)clienteTieneAlRestaurante);
}

t_pedido_en_creacion* obtenerPedidoSegunIdYRestaurante(uint32_t id_pedido, char* nombreRestaurante){
	bool tieneElMismoIdYRestaurante(t_pedido_en_creacion* pedido){
		return pedido->id_pedido == id_pedido && string_equals_ignore_case(pedido->restaurante->nombre,nombreRestaurante);
	}
	return (t_pedido_en_creacion*) list_find(pedidosEnCreacion,(void*) tieneElMismoIdYRestaurante);
}

t_PCB* obtenerPCBSegunIdPedidoYRestaurante(t_list* cola, uint32_t id_pedido, char* nombreRestaurante){
	bool tieneElMismoIdYRestaurante(t_PCB* pedido){
		return pedido->id_pedido == id_pedido && string_equals_ignore_case(pedido->restaurante,nombreRestaurante);
	}
	return (t_PCB*) list_find(cola,(void*) tieneElMismoIdYRestaurante);
}

t_PCB* obtenerPedidoPlanificadoSegunIdYRestaurante(uint32_t idPedido, char* nombreRestaurante){
	t_PCB* pedido = NULL;
	pthread_mutex_lock(&mutexColaNew);
	pthread_mutex_lock(&mutexColaExit);
	pthread_mutex_lock(&mutexColaBlock);
	pthread_mutex_lock(&mutexColaReady);
	pthread_mutex_lock(&mutexColaExec);

	pedido = obtenerPCBSegunIdPedidoYRestaurante(colaNew,idPedido,nombreRestaurante);
	if(pedido == NULL)
		pedido = obtenerPCBSegunIdPedidoYRestaurante(colaReady,idPedido,nombreRestaurante);
	if(pedido == NULL)
		pedido = obtenerPCBSegunIdPedidoYRestaurante(colaBlock,idPedido,nombreRestaurante);
	if(pedido == NULL)
		pedido = obtenerPCBSegunIdPedidoYRestaurante(listaExec,idPedido,nombreRestaurante);
	if(pedido == NULL)
		pedido = obtenerPCBSegunIdPedidoYRestaurante(colaExit,idPedido,nombreRestaurante);

	pthread_mutex_unlock(&mutexColaNew);
	pthread_mutex_unlock(&mutexColaExit);
	pthread_mutex_unlock(&mutexColaBlock);
	pthread_mutex_unlock(&mutexColaReady);
	pthread_mutex_unlock(&mutexColaExec);

	return pedido;
}

char* obtenerPlatosRestoDefault() {
	char* platos = string_new();
	string_append(&platos, "[");
	for (int i = 0; i < list_size(restoDefault->platos); i++) {
		char* plato = list_get(restoDefault->platos, i);
		string_append(&platos, plato);
	}
	string_append(&platos, "]");
	return platos;
}

char* obtenerRestaurantesConectados(){

	char* restaurantes = string_new();
	string_append(&restaurantes,"[");
	if(list_is_empty(restaurantesConectados)){
		string_append(&restaurantes,restoDefault->nombre);
	}
	else{
		for(int i=0; i<list_size(restaurantesConectados); i++){
			t_restaurante_app* restaurant = list_get(restaurantesConectados,i);
			if(i==0)
				string_append(&restaurantes,restaurant->nombre);
			else{
				string_append(&restaurantes,",");
				string_append(&restaurantes,restaurant->nombre);
			}
		}
	}
	string_append(&restaurantes,"]");

	return restaurantes;
}

bool contienePlato(t_list* platos, char* plato){
	bool tienePlato(char* platoLista){
		return string_equals_ignore_case(platoLista,plato);
	}
	return list_count_satisfying(platos,(void*)tienePlato) > 0;
}

// Eliminaciones
void eliminarClienteSegunSocket(uint32_t socket){
	bool tieneElMismoSocket(t_cliente_app* cliente){
		return cliente->socketCliente->socket == socket;
	}
	t_cliente_app* clienteEncontrado = (t_cliente_app*) list_remove_by_condition(clientesConectados,(void*) tieneElMismoSocket);
	free(clienteEncontrado->idCliente);
	free(clienteEncontrado->socketCliente);
	free(clienteEncontrado->host);
	free(clienteEncontrado->puerto);
	free(clienteEncontrado->posicion);
	free(clienteEncontrado);
}

void eliminarRestauranteSegunSocket(uint32_t socket){
	bool tieneElMismoSocket(t_restaurante_app* restaurante){
		return restaurante->socketRestaurante->socket == socket;
	}
	t_restaurante_app* restauranteEncontrado = (t_restaurante_app*) list_remove_by_condition(restaurantesConectados,(void*) tieneElMismoSocket);

	free(restauranteEncontrado->nombre);
	free(restauranteEncontrado->socketRestaurante);
	free(restauranteEncontrado->ip);
	free(restauranteEncontrado->puerto);
	free(restauranteEncontrado->posicion);
	free(restauranteEncontrado);
}

void eliminarPedidoEnCreacionSegunIdYRestaurante(uint32_t id_pedido, char* nombreRestaurante){
	bool tieneElMismoIdYRestaurante(t_pedido_en_creacion* pedido){
		return pedido->id_pedido == id_pedido && string_equals_ignore_case(pedido->restaurante->nombre,nombreRestaurante);
	}
	t_pedido_en_creacion* pedidoEncontrado = (t_pedido_en_creacion*) list_remove_by_condition(pedidosEnCreacion,(void*) tieneElMismoIdYRestaurante);
	free(pedidoEncontrado);
}

// Respuestas repetidas
void responderConEstado(bool OK, t_socket* socket){

	t_resultado_operacion* operacion = malloc(sizeof(t_resultado_operacion));
	operacion->resultado = OK;
	t_buffer mensaje = serializar_mensaje(operacion,T_RESULTADO_OPERACION,sizeof(uint32_t),socket->socket);
	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el mensaje de confirmación al cliente.");
	}

	free(mensaje.data);
	free(operacion);
}

void responderHandshake(t_socket* socket, enum HANDSHAKE modulo){
	// Se avisa al restaurante que fue registrado

	void* handshake;
	t_tipo_mensaje tipoMensaje;
	switch(modulo){
		case T_CLIENTE:{
			tipoMensaje = T_DATOS_CLIENTE_RESPUESTA;
			handshake = malloc(sizeof(t_tipo_modulo));
			((t_tipo_modulo*) handshake)->modulo = T_APP; // Indicamos que somos el módulo APP
			break;
		}
		case T_RESTAURANTE:{
			tipoMensaje = T_PING;
			handshake = malloc(sizeof(t_ping_s));
			((t_ping_s*) handshake)->result = 1; // OK
			break;
		}
	}

	t_buffer mensaje = serializar_mensaje(handshake,tipoMensaje,sizeof(uint32_t),socket->socket);

	if(!enviar_mensaje(&mensaje))
		log_error(logger, "No se pudo enviar el mensaje de confirmación al cliente.");
	else
		log_debug(logger, "Mensaje de confirmación al cliente enviado con éxito.");
	free(handshake);
	free(mensaje.data);
}

void responderConsultarPedidoConError(uint32_t socket){
	t_consultar_pedido_s* pedidoARetornar = malloc(sizeof(t_consultar_pedido_s));
	pedidoARetornar->nombre_restaurante = string_new();
	pedidoARetornar->nombre_restaurante_size = strlen(pedidoARetornar->nombre_restaurante) + 1;
	pedidoARetornar->estado = -1;
	pedidoARetornar->cantidadPlatos = 0;
	pedidoARetornar->platos = NULL;

	uint32_t size = sizeof(uint32_t)*3 + pedidoARetornar->nombre_restaurante_size;
	t_buffer mensaje = serializar_mensaje(pedidoARetornar,T_CONSULTAR_PEDIDO_RESPUESTA,size,socket);
	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el mensaje de confirmación al cliente.");
	}
	free(mensaje.data);
	free(pedidoARetornar->nombre_restaurante);
	free(pedidoARetornar);
}

// Otros
uint32_t crear_conexion(char* ip, char* puerto){
	t_socket conexion = crear_socket_de_conexion(ip,puerto);
	if(conexion.socket == -1)
		return -1;

	if(!conectar_socket(conexion)){
		return -1;
	}
	freeaddrinfo(conexion.socket_info);
	return conexion.socket;
}

uint32_t generarIdPedido(){

	// Tomamos el timestamp del momento y le quitamos los primeros dígitos significativos para que quepa en un UINT32_T
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long timestamp = (unsigned long long) (tv.tv_sec) * 1000 + (unsigned long long) (tv.tv_usec) / 1000;
	return (uint32_t) (timestamp % 1000000000);
}

void liberarPedido(t_obtener_pedido_s* pedido){
	list_destroy_and_destroy_elements(pedido->platos,&free);
	free(pedido);
}

// === Nuevas Conexiones ===
void guardarConexionCliente(t_peticion* peticion){

	log_debug(logger,"=== NUEVA CONEXIÓN: Un cliente se conectó a la App. ===");
	t_cliente* cliente = (t_cliente*) deserializar_mensaje(peticion->mensaje);

	t_cliente_app* clienteNuevo = malloc(sizeof(t_cliente_app));
	clienteNuevo->idCliente = strdup(cliente->id_cliente);
	t_posicion* posicion = malloc(sizeof(t_posicion));
	posicion->x = cliente->posicion_x;
	posicion->y = cliente->posicion_y;
	clienteNuevo->posicion = posicion;
	clienteNuevo->socketCliente = peticion->socketCliente;
	clienteNuevo->restauranteSeleccionado = NULL;
	clienteNuevo->host = strdup(cliente->ip_escucha);
	clienteNuevo->puerto = strdup(cliente->puerto_escucha);

	imprimirCliente(clienteNuevo);
	// Guardo al cliente en la lista de conectados
	pthread_mutex_lock(&mutexClientesConectados);
	list_add(clientesConectados, clienteNuevo);
	log_debug(logger,"== Clientes online = %d ==", list_size(clientesConectados));
	pthread_mutex_unlock(&mutexClientesConectados);

	// Libero el paquete enviado
	free(cliente->ip_escucha);
	free(cliente->puerto_escucha);
	free(cliente->id_cliente);
	free(cliente);

	// Se avisa al cliente que fue registrado
	responderHandshake(clienteNuevo->socketCliente, T_CLIENTE);
}

void guardarConexionRestaurante(t_peticion* peticion){

	log_debug(logger,"=== NUEVA CONEXIÓN: Un restaurante se conectó a la App. ===");
	t_restaurante_handshake* restaurante = (t_restaurante_handshake*) deserializar_mensaje(peticion->mensaje);

	t_restaurante_app* restauranteNuevo = malloc(sizeof(t_restaurante_app));
	restauranteNuevo->nombre = strdup(restaurante->nombre_restaurante);
	restauranteNuevo->platos = NULL;
	t_posicion* posicion = malloc(sizeof(t_posicion));
	posicion->x = restaurante->posicion_x;
	posicion->y = restaurante->posicion_y;
	restauranteNuevo->posicion = posicion;
	restauranteNuevo->socketRestaurante = peticion->socketCliente;
	restauranteNuevo->ip = strdup(restaurante->ip);
	restauranteNuevo->puerto = strdup(restaurante->puerto);

	imprimirRestaurante(restauranteNuevo);

	// Guardo al restaurante en la lista de conectados
	pthread_mutex_lock(&mutexRestaurantesConectados);
	list_add(restaurantesConectados, restauranteNuevo);
	log_debug(logger,"== Restaurantes online = %d ==", list_size(restaurantesConectados));
	pthread_mutex_unlock(&mutexRestaurantesConectados);

	// Libero el paquete enviado
	free(restaurante->nombre_restaurante);
	free(restaurante->ip);
	free(restaurante->puerto);
	free(restaurante);

	responderHandshake(restauranteNuevo->socketRestaurante, T_RESTAURANTE);
}

// === Desconexiones ===
void eliminarConexionCliente(t_socket* socketCliente){

	// Buscar en lista de restaurantes
	pthread_mutex_lock(&mutexRestaurantesConectados);
	t_restaurante_app* restauranteEncontrado = obtenerRestauranteSegunSocket(socketCliente->socket);
	pthread_mutex_unlock(&mutexRestaurantesConectados);
	if(restauranteEncontrado != NULL){
		log_debug(logger,"== DESCONEXIÓN: El restaurante %s se desconectó de la App ==", restauranteEncontrado->nombre);

		pthread_mutex_lock(&mutexClientesConectados);
		// Limpiamos asignación de restaurantes al cliente
		t_list* clientes = obtenerClientesQueEligieronRestaurante(restauranteEncontrado->nombre);

		for(int i=0; i<list_size(clientes); i++){
			t_cliente_app* cliente = list_get(clientesConectados,i);
			cliente->restauranteSeleccionado = NULL;
		}
		free(clientes);
		pthread_mutex_unlock(&mutexClientesConectados);

		// Quitamos al restaurante de la lista de conectados
		pthread_mutex_lock(&mutexRestaurantesConectados);
		eliminarRestauranteSegunSocket(socketCliente->socket);
		pthread_mutex_unlock(&mutexRestaurantesConectados);
		log_debug(logger,"== Restaurantes online = %d ==", list_size(restaurantesConectados));
		return;
	}


	// Buscar en lista de clientes
	pthread_mutex_lock(&mutexClientesConectados);
	t_cliente_app* clienteEncontrado = obtenerClienteSegunSocket(socketCliente->socket);
	pthread_mutex_unlock(&mutexClientesConectados);

	if(clienteEncontrado != NULL){
		log_debug(logger,"== DESCONEXIÓN: El cliente %s se desconectó de la App ==", clienteEncontrado->idCliente);
		pthread_mutex_lock(&mutexClientesConectados);
		eliminarClienteSegunSocket(socketCliente->socket);
		pthread_mutex_unlock(&mutexClientesConectados);
		log_debug(logger,"== Clientes online = %d ==", list_size(clientesConectados));
		return;
	}
	log_error(logger, "ERROR: No se pudo eliminar la conexión del cliente/restaurante. Se perdió el socket.");
}

// === Mensajes ===
void consultarRestaurantes(t_peticion* peticion){

	log_debug(logger, "=== Ejecutando CONSULTAR RESTAURANTES ===");

	pthread_mutex_lock(&mutexRestaurantesConectados);
	char* restaurantes = obtenerRestaurantesConectados();
	pthread_mutex_unlock(&mutexRestaurantesConectados);
	log_info(logger, "Restaurantes a enviar: %s.",restaurantes);

	t_listado_restaurantes* listaRestaurantes = malloc(sizeof(t_listado_restaurantes));
	listaRestaurantes->listado_size = strlen(restaurantes) + 1;
	listaRestaurantes->listado = restaurantes;
	uint32_t size = sizeof(listaRestaurantes->listado_size) + listaRestaurantes->listado_size;

	// Enviamos la respuesta
	t_buffer mensaje = serializar_mensaje(listaRestaurantes,T_LISTADO_RESTAURANTES,size,peticion->socketCliente->socket);

	if(!enviar_mensaje(&mensaje))
		log_error(logger, "No se pudo enviar el listado de restaurantes al cliente.");
	else
		log_debug(logger, "Mensaje con el listado de restaurantes enviado al cliente con éxito.");
	free(listaRestaurantes);
	free(restaurantes);
	free(mensaje.data);
}

void seleccionarRestaurante(t_peticion* peticion){

	log_debug(logger, "=== Ejecutando SELECCIONAR RESTAURANTE ===");

	t_seleccionar_restaurante* seleccion = (t_seleccionar_restaurante*) deserializar_mensaje(peticion->mensaje);

	log_info(logger,"--> ID CLIENTE = %s\n", seleccion->id_cliente);
	log_info(logger,"--> NOMBRE RESTAURANTE = %s\n", seleccion->id_restaurante);

	t_restaurante_app* restoEncontrado = NULL;

	if(list_is_empty(restaurantesConectados) && esRestauranteDefault(seleccion->id_restaurante)){
		log_info(logger,"=== El restaurante elegido es el DEFAULT\n");
		restoEncontrado = restoDefault;
	}
	else{
		pthread_mutex_lock(&mutexRestaurantesConectados);
		restoEncontrado = obtenerRestauranteSegunNombre(seleccion->id_restaurante);
		pthread_mutex_unlock(&mutexRestaurantesConectados);

		if(restoEncontrado == NULL){
			log_error(logger,"=== No se encontró el restaurante %s\n",seleccion->id_restaurante);
			responderConEstado(false,peticion->socketCliente);
			free(seleccion->id_cliente);
			free(seleccion->id_restaurante);
			free(seleccion);
			return;
		}
	}

	pthread_mutex_lock(&mutexClientesConectados);
	t_cliente_app* clienteEncontrado = obtenerClienteSegunId(seleccion->id_cliente);
	pthread_mutex_unlock(&mutexClientesConectados);

	// Asignamos el restaurante al cliente
	clienteEncontrado->restauranteSeleccionado = restoEncontrado;
	free(seleccion->id_cliente);
	free(seleccion->id_restaurante);
	free(seleccion);
	responderConEstado(true,peticion->socketCliente);
}

void consultarPlatos(t_peticion* peticion){

	log_debug(logger, "=== CONSULTAR PLATOS ===");

	char* platos;

	pthread_mutex_lock(&mutexClientesConectados);
	t_cliente_app* clienteEncontrado = obtenerClienteSegunSocket(peticion->socketCliente->socket);
	pthread_mutex_unlock(&mutexClientesConectados);

	pthread_mutex_lock(&mutexRestaurantesConectados);
	// Debe haber restaurantes conectados, y el cliente haber elegido un restaurante que no sea el default.
	if(!list_is_empty(restaurantesConectados) && clienteEncontrado->restauranteSeleccionado != NULL && !esRestauranteDefault(clienteEncontrado->restauranteSeleccionado->nombre))
	{
		t_restaurante_app* restaurante = obtenerRestauranteSegunNombre(clienteEncontrado->restauranteSeleccionado->nombre);
		if(restaurante != NULL){
			// Socket de Conexión
			uint32_t socket = crear_conexion(restaurante->ip,restaurante->puerto);
			t_buffer consulta = crear_buffer_sin_cuerpo(T_CONSULTAR_PLATOS, socket);
			if(!enviar_mensaje(&consulta)){
				log_error(logger, "No se pudo enviar el pedido de los platos al Restaurante.");
				platos = obtenerPlatosRestoDefault();
			}
			else{
				log_info(logger, "Consulta enviada al restaurante %s. Esperando respuesta...", restaurante->nombre);
				t_buffer respuesta = recibir_mensaje(socket);

				if(respuesta.msj_type != T_CONSULTAR_PLATOS_RESPUESTA){
					platos = obtenerPlatosRestoDefault();
				}
				else{
					t_single_text_s* platosMsj = (t_single_text_s*) deserializar_mensaje(&respuesta);
					platos = strdup(platosMsj->text);
					free(respuesta.data);
					free(platosMsj->text);
					free(platosMsj);
				}
			}
			if(socket > 0) close(socket);
		}
		else
			platos = obtenerPlatosRestoDefault();
	}

	else // Si va por este camino quiere decir que o no eligió restaurante, o el restaurante elegido era el default, o no había restaurantes conectados
		platos = obtenerPlatosRestoDefault();
	pthread_mutex_unlock(&mutexRestaurantesConectados);
	log_info(logger, "=== Platos a devolver: %s===",platos);

	// Enviar mensaje de vuelta al cliente
	t_single_text_s* respuesta = malloc(sizeof(t_single_text_s));
	respuesta->text = strdup(platos);
	respuesta->text_size = strlen(respuesta->text ) + 1;
	uint32_t size = sizeof(respuesta->text_size) + respuesta->text_size;

	t_buffer mensaje = serializar_mensaje(respuesta,T_CONSULTAR_PLATOS_RESPUESTA,size,peticion->socketCliente->socket);

	if(!enviar_mensaje(&mensaje))
		log_error(logger, "No se pudo enviar el listado de platos al cliente.");
	else
		log_debug(logger, "Mensaje con el listado de platos enviado al cliente con éxito.");

	free(mensaje.data);
	free(respuesta->text);
	free(respuesta);
	free(platos);
}

void crearPedido(t_peticion* peticion){

	log_debug(logger, "=== Ejecutando CREAR PEDIDO ===");

	uint32_t idPedido;
	char* restaurante;

	pthread_mutex_lock(&mutexClientesConectados);
	t_cliente_app* clienteEncontrado = obtenerClienteSegunSocket(peticion->socketCliente->socket);
	pthread_mutex_unlock(&mutexClientesConectados);

	t_restaurante_app* restauranteEncontrado;

	pthread_mutex_lock(&mutexRestaurantesConectados);
	// Debe haber restaurantes conectados, y el cliente haber elegido un restaurante que no sea el default.
	if(!list_is_empty(restaurantesConectados) && clienteEncontrado->restauranteSeleccionado != NULL && !esRestauranteDefault(clienteEncontrado->restauranteSeleccionado->nombre))
	{
		restauranteEncontrado = obtenerRestauranteSegunNombre(clienteEncontrado->restauranteSeleccionado->nombre);
		if(restauranteEncontrado != NULL){
			uint32_t socket = crear_conexion(restauranteEncontrado->ip,restauranteEncontrado->puerto);

			// Enviamos pedido de creación al Restaurante
			log_info(logger, "=== Se enviará la consulta al Restaurante %s ===",restauranteEncontrado->nombre);

			t_buffer consulta = crear_buffer_sin_cuerpo(T_CREAR_PEDIDO, socket);
			if(!enviar_mensaje(&consulta)){
				log_error(logger, "No se pudo enviar la solicitud de creación de pedido al Restaurante.");
				idPedido = generarIdPedido();
				restauranteEncontrado = restoDefault;
			}
			else{
				t_buffer respuesta = recibir_mensaje(socket);
				if(respuesta.msj_type != T_CREAR_PEDIDO_RESPUESTA){
					log_error(logger, "Hubo un error en la respuesta del Restaurante. Se seleccionará el Restaurante Default.");
					idPedido = generarIdPedido();
					restauranteEncontrado = restoDefault;
				}
				else{
					t_id_pedido* pedido = (t_id_pedido*) deserializar_mensaje(&respuesta);
					idPedido = pedido->id_pedido;
					if(idPedido == -1){
						t_id_pedido* resultado = malloc(sizeof(t_id_pedido));
						resultado->id_pedido = 0;
						t_buffer mensaje = serializar_mensaje(resultado,T_CREAR_PEDIDO_RESPUESTA,sizeof(uint32_t),peticion->socketCliente->socket);

						if(!enviar_mensaje(&mensaje)){
							log_error(logger, "No se pudo enviar el mensaje de error al cliente.");
						}
						else{
							log_debug(logger, "Mensaje con error al crear pedido enviado al cliente con éxito.");
						}
						free(resultado);
						free(respuesta.data);
						free(pedido);
						if(socket > 0) close(socket);
						return;
					}
					log_info(logger, "=== El ID de Pedido fue generado por el Restaurante %s",restauranteEncontrado->nombre);
					free(respuesta.data);
					free(pedido);
				}
			}
			if(socket > 0) close(socket);
		}
		else{ // Si el restaurante elegido no está disponible
			idPedido = generarIdPedido();
			restauranteEncontrado = restoDefault;
		}
	}
	else{ // Si el elegido es el default o no hay ningun restaurante conectado
		idPedido = generarIdPedido();
		restauranteEncontrado = restoDefault;
	}
	pthread_mutex_unlock(&mutexRestaurantesConectados);

	log_debug(logger, "=== ID PEDIDO GENERADO ==== %d", idPedido);

	// Enviamos la solicitud de guardado a la Comanda
	uint32_t socketComanda = crear_conexion(config->ip_comanda,config->puerto_comanda);
	t_pedido_s* solicitudGuardado = malloc(sizeof(t_pedido_s));
	solicitudGuardado->nombre_restaurante = strdup(restauranteEncontrado->nombre);
	solicitudGuardado->nombre_restaurante_size = strlen(restauranteEncontrado->nombre) + 1;
	solicitudGuardado->id_pedido = idPedido;
	int sizeMsg = sizeof(uint32_t)*2 + solicitudGuardado->nombre_restaurante_size;

	t_buffer guardarPedido = serializar_mensaje(solicitudGuardado,T_GUARDAR_PEDIDO,sizeMsg,socketComanda);

	if(!enviar_mensaje(&guardarPedido)){
		log_error(logger, "No se pudo enviar el mensaje a la Comanda.");
		free(solicitudGuardado->nombre_restaurante);
		free(solicitudGuardado);
		free(guardarPedido.data);
		return;
	}
	else
		log_debug(logger, "Mensaje de guardado de pedido enviado a la Comanda con éxito.");

	free(guardarPedido.data);
	free(solicitudGuardado->nombre_restaurante);
	free(solicitudGuardado);

	// Esperamos la respuesta de la Comanda
	t_buffer guardado = recibir_mensaje(socketComanda);
	if(guardado.msj_type != T_RESULTADO_OPERACION){
		log_error(logger, "Hubo un error en la respuesta.");
		responderConEstado(false,peticion->socketCliente); // TODO Validar
		return;
	}
	t_resultado_operacion* resultadoGuardado = (t_resultado_operacion*) deserializar_mensaje(&guardado);
	log_info(logger, "Resultado devuelto por la comanda.. %d",resultadoGuardado->resultado);
	if(!resultadoGuardado->resultado)
		idPedido = 0;

	close(socketComanda);
	// Devuelvo respuesta al cliente
	t_id_pedido* respuesta = malloc(sizeof(t_id_pedido));
	respuesta->id_pedido = idPedido;

	t_buffer mensaje = serializar_mensaje(respuesta,T_CREAR_PEDIDO_RESPUESTA,sizeof(uint32_t),peticion->socketCliente->socket);

	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el id de pedido al cliente.");
	}
	else
		log_debug(logger, "Mensaje con el id de pedido enviado al cliente con éxito.");

	// Guardado el pedido en una lista para poder referenciarlo luego
	t_pedido_en_creacion *pedido = malloc(sizeof(t_pedido_en_creacion));
	pedido->id_pedido = idPedido;
	pedido->restaurante = restauranteEncontrado;
	pedido->cliente = clienteEncontrado;
	list_add(pedidosEnCreacion,pedido);

	free(respuesta);
	free(resultadoGuardado);
	free(mensaje.data);
	free(guardado.data);
}

void aniadirPlato(t_peticion* peticion){
	log_debug(logger,"=== Ejecutando AÑADIR PLATO ===");

	t_aniadir_plato* seleccion = (t_aniadir_plato*) deserializar_mensaje(peticion->mensaje);
	log_info(logger, "ID PEDIDO = %d", seleccion->id_pedido);
	log_info(logger, "PLATO = %s", seleccion->plato);

	// Buscamos al cliente
	pthread_mutex_lock(&mutexClientesConectados);
	t_cliente_app* clienteEncontrado = obtenerClienteSegunSocket(peticion->socketCliente->socket);
	pthread_mutex_unlock(&mutexClientesConectados);

	// Buscar pedido en la lista para saber a que restaurant mandarle el mensaje
	char* nombreRestaurante = clienteEncontrado->restauranteSeleccionado == NULL ? strdup(restoDefault->nombre) : strdup(clienteEncontrado->restauranteSeleccionado->nombre);
	t_pedido_en_creacion* pedido = obtenerPedidoSegunIdYRestaurante(seleccion->id_pedido, nombreRestaurante);
	if(pedido == NULL){
		log_error(logger,"=== No se encontró el pedido ===");
		responderConEstado(false,peticion->socketCliente);
		free(seleccion->plato);
		free(seleccion);
		free(nombreRestaurante);
		return;
	}
	free(nombreRestaurante);
	// Validamos que el cliente sea el dueño del pedido
	if(!esElMismoCliente(pedido->cliente->idCliente,clienteEncontrado->idCliente)){
		log_error(logger,"=== El pedido no pertenece al cliente que lo solicitó ===");
		responderConEstado(false,peticion->socketCliente);
		free(seleccion->plato);
		free(seleccion);
		return;
	}

	// Si llegamos acá, es porque el pedido existe y es del cliente. Entonces enviamos el mensaje al restaurante (Si no es el DEFAULT)
	if(!esRestauranteDefault(pedido->restaurante->nombre)){
		log_info(logger,"=== No es restaurante DEFAULT ===");

		uint32_t socketRestaurante = crear_conexion(pedido->restaurante->ip,pedido->restaurante->puerto);

		t_aniadir_plato* agregarPlato = malloc(sizeof(t_aniadir_plato));
		agregarPlato->id_pedido = seleccion->id_pedido;
		agregarPlato->plato = strdup(seleccion->plato);
		agregarPlato->plato_size = seleccion->plato_size;
		uint32_t size = sizeof(uint32_t)*2 + agregarPlato->plato_size;
		t_buffer mensaje = serializar_mensaje(agregarPlato,T_ANIADIR_PLATO,size,socketRestaurante);

		if(!enviar_mensaje(&mensaje)){
			log_error(logger, "No se pudo enviar la solicitud de añadir plato al Restaurante.");
			responderConEstado(false,peticion->socketCliente);
			free(agregarPlato->plato);
			free(agregarPlato);
			free(seleccion->plato);
			free(seleccion);
			free(mensaje.data);
			return;
		}
		else{
			log_debug(logger,"=== Se envio el mensaje al restaurante, esperando respuesta.. ===");
			t_buffer respuesta = recibir_mensaje(socketRestaurante);
			if(respuesta.msj_type != T_RESULTADO_OPERACION)
			{
				log_error(logger, "Hubo un error en la respuesta del Restaurante.");
				responderConEstado(false,peticion->socketCliente);
				free(agregarPlato->plato);
				free(agregarPlato);
				free(seleccion->plato);
				free(seleccion);
				if (socketRestaurante > 0) close(socketRestaurante);
				return;
			}
			t_resultado_operacion* resultadoGuardado = (t_resultado_operacion*) deserializar_mensaje(&respuesta);
			if(!resultadoGuardado->resultado){
				responderConEstado(false,peticion->socketCliente);
				free(agregarPlato->plato);
				free(agregarPlato);
				free(seleccion);
				free(resultadoGuardado);
				if (socketRestaurante > 0) close(socketRestaurante);
				return;
			}
		}
		free(agregarPlato->plato);
		free(agregarPlato);
		close(socketRestaurante);
	}
	else{
		// Validamos que el plato enviado pertenezca al restaurante default
		if(!contienePlato(restoDefault->platos, seleccion->plato)){
			log_error(logger, "El plato no pertenece al restaurante Default");
			responderConEstado(false,peticion->socketCliente);
			free(seleccion->plato);
			free(seleccion);
			return;
		}
	}
	log_info(logger,"=== Se va a invocar a la Comanda ===");
	uint32_t socketComanda = crear_conexion(config->ip_comanda,config->puerto_comanda);

	t_guardar_plato_s* guardarPedido = malloc(sizeof(t_guardar_plato_s));
	guardarPedido->id_pedido = seleccion->id_pedido;
	guardarPedido->nombre_plato = strdup(seleccion->plato);
	guardarPedido->nombre_plato_size = seleccion->plato_size;
	guardarPedido->cantidad = 1;
	guardarPedido->nombre_restaurante = strdup(pedido->restaurante->nombre);
	guardarPedido->nombre_restaurante_size = strlen(guardarPedido->nombre_restaurante) + 1;
	uint32_t size = sizeof(uint32_t)*4 + guardarPedido->nombre_plato_size + guardarPedido->nombre_restaurante_size;
	t_buffer mensaje = serializar_mensaje(guardarPedido,T_GUARDAR_PLATO,size,socketComanda);

	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el mensaje a la Comanda.");
		responderConEstado(false,peticion->socketCliente);
		free(seleccion->plato);
		free(seleccion);
		free(guardarPedido->nombre_plato);
		free(guardarPedido->nombre_restaurante);
		free(guardarPedido);
		free(mensaje.data);
		return;
	}
	log_debug(logger,"=== Mensaje enviado a la Comanda, esperando respuesta...");
	free(mensaje.data);
	free(guardarPedido->nombre_restaurante);
	free(guardarPedido->nombre_plato);
	free(guardarPedido);
	free(seleccion->plato);
	free(seleccion);


	// Respuesta de la Comanda a Guardar Pedido
	t_buffer guardado = recibir_mensaje(socketComanda);
	if(guardado.msj_type != T_RESULTADO_OPERACION){
		log_error(logger, "Hubo un error en la respuesta.");
		responderConEstado(false,peticion->socketCliente);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	t_resultado_operacion* resultadoGuardado = (t_resultado_operacion*) deserializar_mensaje(&guardado);
	log_info(logger,"=== Resultado de la operación = %d ===",resultadoGuardado->resultado);
	if(!resultadoGuardado->resultado){
		responderConEstado(false,peticion->socketCliente);
		free(guardado.data);
		free(resultadoGuardado);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	free(guardado.data);
	free(resultadoGuardado);
	responderConEstado(true,peticion->socketCliente);
	close(socketComanda);
}

void confirmarPedido(t_peticion* peticion){

	log_debug(logger,"=== Ejecutando CONFIRMAR PEDIDO ===");
	t_id_pedido* pedido = deserializar_mensaje(peticion->mensaje);
	log_info(logger,"ID = %d",pedido->id_pedido);

	pthread_mutex_lock(&mutexClientesConectados);
	t_cliente_app* clienteEncontrado = obtenerClienteSegunSocket(peticion->socketCliente->socket);
	pthread_mutex_unlock(&mutexClientesConectados);

	// Valido existencia del pedido
	char* nombreRestaurante = clienteEncontrado->restauranteSeleccionado == NULL ? strdup(restoDefault->nombre) : strdup(clienteEncontrado->restauranteSeleccionado->nombre);
	t_pedido_en_creacion* pedidoEnCreacion = obtenerPedidoSegunIdYRestaurante(pedido->id_pedido, nombreRestaurante);
	if(pedidoEnCreacion == NULL){
		log_error(logger,"=== No se encontró el pedido ===");
		responderConEstado(false,peticion->socketCliente);
		free(pedido);
		free(nombreRestaurante);
		return;
	}
	free(nombreRestaurante);
	// Validamos que el cliente sea el dueño del pedido
	if(!string_equals_ignore_case(pedidoEnCreacion->cliente->idCliente,clienteEncontrado->idCliente)){
		log_error(logger,"=== El pedido no pertenece al cliente que lo solicitó ===");
		responderConEstado(false,peticion->socketCliente);
		free(pedido);
		return;
	}

	free(pedido);
	// Obtenemos el pedido desde la comanda
	uint32_t socketComanda = crear_conexion(config->ip_comanda,config->puerto_comanda);

	t_pedido_s* obtenerPedido = malloc(sizeof(t_pedido_s));
	obtenerPedido->id_pedido = pedidoEnCreacion->id_pedido;
	obtenerPedido->nombre_restaurante = strdup(pedidoEnCreacion->restaurante->nombre);
	obtenerPedido->nombre_restaurante_size = strlen(obtenerPedido->nombre_restaurante) + 1;
	uint32_t size = sizeof(uint32_t)*2 + obtenerPedido->nombre_restaurante_size;
	t_buffer mensaje = serializar_mensaje(obtenerPedido,T_OBTENER_PEDIDO,size,socketComanda);

	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el mensaje a la Comanda.");
		responderConEstado(false,peticion->socketCliente);
		free(obtenerPedido->nombre_restaurante);
		free(obtenerPedido);
		free(mensaje.data);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	free(obtenerPedido->nombre_restaurante);
	free(obtenerPedido);
	free(mensaje.data);
	log_debug(logger,"=== Mensaje OBTENER PEDIDO enviado a la Comanda. Esperando respuesta... ===");

	// Recibimos la respuesta de la comanda
	t_buffer respuestaComanda = recibir_mensaje(socketComanda);
	if(respuestaComanda.msj_type != T_OBTENER_PEDIDO_RESPUESTA)
	{
		log_error(logger, "Hubo un error en la respuesta.");
		responderConEstado(false,peticion->socketCliente);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	t_obtener_pedido_s* pedidoObtenido = (t_obtener_pedido_s*) deserializar_mensaje(&respuestaComanda);
	if(pedidoObtenido->estado == -1){
		log_error(logger, "La Comanda no encontró el pedido.");
		responderConEstado(false,peticion->socketCliente);
		free(respuestaComanda.data);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	imprimirPedido(pedidoObtenido);
	liberarPedido(pedidoObtenido);
	free(respuestaComanda.data);

	// Enviar confirmar pedido al restaurante (si no es el default)
	if(!esRestauranteDefault(pedidoEnCreacion->restaurante->nombre)){
		uint32_t socketRestaurante = crear_conexion(pedidoEnCreacion->restaurante->ip,pedidoEnCreacion->restaurante->puerto);
		t_id_pedido* confirmoPed = malloc(sizeof(t_id_pedido));
		confirmoPed->id_pedido = pedidoEnCreacion->id_pedido;
		t_buffer envio = serializar_mensaje(confirmoPed,T_CONFIRMAR_PEDIDO_SOLO_ID,sizeof(uint32_t),socketRestaurante);
		if(!enviar_mensaje(&envio)){
			log_error(logger, "No se pudo enviar el mensaje al Restaurante.");
			responderConEstado(false,peticion->socketCliente);
			free(confirmoPed);
			free(envio.data);
			if(socketComanda > 0) close(socketComanda);
			if(socketRestaurante > 0) close(socketRestaurante);
			return;
		}
		free(confirmoPed);
		free(envio.data);
		log_debug(logger,"=== Mensaje CONFIRMAR PEDIDO enviado al Restaurante. Esperando respuesta... ===");

		// Analizamos respuesta
		t_buffer respuesta = recibir_mensaje(socketRestaurante);
		if(respuesta.msj_type != T_RESULTADO_OPERACION){
			log_error(logger, "Hubo un error en la respuesta del Restaurante.");
			responderConEstado(false,peticion->socketCliente);
			if(socketComanda > 0) close(socketComanda);
			if(socketRestaurante > 0) close(socketRestaurante);
			return;
		}

		t_resultado_operacion* resultadoGuardado = (t_resultado_operacion*) deserializar_mensaje(&respuesta);
		log_info(logger,"=== Resultado de la operación: %d ===",resultadoGuardado->resultado);
		if(!resultadoGuardado->resultado){
			responderConEstado(false,peticion->socketCliente);
			free(resultadoGuardado);
			free(respuesta.data);
			if(socketComanda > 0) close(socketComanda);
			if(socketRestaurante > 0) close(socketRestaurante);
			return;
		}

		free(resultadoGuardado);
		free(respuesta.data);
		close(socketRestaurante);
	}
	// Si llegamos hasta acá, los mensajes con Comanda y Restaurante son exitosos. Se crea el PCB y se lo manda a cola de New.
	t_PCB* nuevoPedido = planificarPedidoNuevo(pedidoEnCreacion, peticion->socketCliente->socket);

	// Avisamos a la comanda...
	t_pedido_s* confirmarPedidoComanda = malloc(sizeof(t_pedido_s));
	confirmarPedidoComanda->id_pedido = nuevoPedido->id_pedido;
	confirmarPedidoComanda->nombre_restaurante = strdup(nuevoPedido->restaurante);
	confirmarPedidoComanda->nombre_restaurante_size = strlen(confirmarPedidoComanda->nombre_restaurante) + 1;
	int sizeConfirmar = sizeof(uint32_t) * 2 + confirmarPedidoComanda->nombre_restaurante_size;

	t_buffer confirmarConComanda = serializar_mensaje(confirmarPedidoComanda,T_CONFIRMAR_PEDIDO,sizeConfirmar,socketComanda);
	if(!enviar_mensaje(&confirmarConComanda)){
		log_error(logger, "No se pudo enviar el mensaje a la Comanda.");
		responderConEstado(false,peticion->socketCliente);
		free(confirmarPedidoComanda->nombre_restaurante);
		free(confirmarPedidoComanda);
		free(confirmarConComanda.data);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	free(confirmarPedidoComanda->nombre_restaurante);
	free(confirmarPedidoComanda);
	free(confirmarConComanda.data);
	log_debug(logger,"=== Mensaje CONFIRMAR PEDIDO enviado a la Comanda. Esperando respuesta... ===");

	// Analizamos respuesta
	t_buffer respuesta = recibir_mensaje(socketComanda);
	if(respuesta.msj_type != T_RESULTADO_OPERACION){
		log_error(logger, "Hubo un error en la respuesta de la Comanda.");
		responderConEstado(false,peticion->socketCliente);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	t_resultado_operacion* resultadoGuardado = (t_resultado_operacion*) deserializar_mensaje(&respuesta);
	log_info(logger,"=== Resultado de la operación: %d ===",resultadoGuardado->resultado);

	if(!resultadoGuardado->resultado){
		responderConEstado(false,peticion->socketCliente);
		free(resultadoGuardado);
		free(respuesta.data);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	free(resultadoGuardado);
	free(respuesta.data);
	close(socketComanda);

	// Por fin le contestamos al cliente
	responderConEstado(true,peticion->socketCliente);
}

void consultarPedido(t_peticion* peticion){

	log_debug(logger,"=== Ejecutando CONSULTAR PEDIDO ===");
	t_id_pedido* pedido = deserializar_mensaje(peticion->mensaje);
	log_info(logger,"ID = %d",pedido->id_pedido);

	// Obtenemos al clienet
	pthread_mutex_lock(&mutexClientesConectados);
	t_cliente_app* clienteEncontrado = obtenerClienteSegunSocket(peticion->socketCliente->socket);
	pthread_mutex_unlock(&mutexClientesConectados);

	// Valido existencia del pedido
	char* nombreRestaurante = clienteEncontrado->restauranteSeleccionado == NULL ? strdup(restoDefault->nombre) : strdup(clienteEncontrado->restauranteSeleccionado->nombre);
	t_PCB* pedidoAConsultar = obtenerPedidoPlanificadoSegunIdYRestaurante(pedido->id_pedido, nombreRestaurante);

	if(pedidoAConsultar == NULL){
		log_error(logger,"=== No se encontró el pedido ===");
		responderConsultarPedidoConError(peticion->socketCliente->socket);
		free(pedido);
		free(nombreRestaurante);
		return;
	}
	free(pedido);
	free(nombreRestaurante);

	// Validamos que el cliente sea el dueño del pedido
	if(!string_equals_ignore_case(pedidoAConsultar->id_cliente,clienteEncontrado->idCliente)){
		log_error(logger,"=== El pedido no pertenece al cliente que lo solicitó ===");
		responderConsultarPedidoConError(peticion->socketCliente->socket);
		return;
	}

	// 1. Llamar a comanda y pedirle el pedido
	uint32_t socketComanda = crear_conexion(config->ip_comanda,config->puerto_comanda);

	t_pedido_s* obtenerPedido = malloc(sizeof(t_pedido_s));
	obtenerPedido->id_pedido = pedidoAConsultar->id_pedido;
	obtenerPedido->nombre_restaurante = strdup(pedidoAConsultar->restaurante);
	obtenerPedido->nombre_restaurante_size = strlen(obtenerPedido->nombre_restaurante) + 1;
	uint32_t size = sizeof(uint32_t)*2 + obtenerPedido->nombre_restaurante_size;
	t_buffer mensaje = serializar_mensaje(obtenerPedido,T_OBTENER_PEDIDO,size,socketComanda);

	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el mensaje a la Comanda.");
		responderConsultarPedidoConError(peticion->socketCliente->socket);
		free(obtenerPedido->nombre_restaurante);
		free(obtenerPedido);
		free(mensaje.data);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	free(obtenerPedido->nombre_restaurante);
	free(obtenerPedido);
	free(mensaje.data);
	log_debug(logger,"=== Mensaje OBTENER PEDIDO enviado a la Comanda. Esperando respuesta... ===");

	// Recibimos la respuesta de la comanda
	t_buffer respuestaComanda = recibir_mensaje(socketComanda);
	if(respuestaComanda.msj_type != T_OBTENER_PEDIDO_RESPUESTA)
	{
		log_error(logger, "Hubo un error en la respuesta.");
		responderConsultarPedidoConError(peticion->socketCliente->socket);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	t_obtener_pedido_s* pedidoObtenido = (t_obtener_pedido_s*) deserializar_mensaje(&respuestaComanda);
	if(pedidoObtenido->estado == -1){
		log_error(logger, "La Comanda no encontró el pedido.");
		responderConsultarPedidoConError(peticion->socketCliente->socket);
		free(respuestaComanda.data);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	free(respuestaComanda.data);
	close(socketComanda);
	imprimirPedido(pedidoObtenido);

	t_consultar_pedido_s* pedidoARetornar = malloc(sizeof(t_consultar_pedido_s));
	pedidoARetornar->nombre_restaurante = strdup(pedidoAConsultar->restaurante);
	pedidoARetornar->nombre_restaurante_size = strlen(pedidoARetornar->nombre_restaurante) + 1;
	pedidoARetornar->estado = pedidoObtenido->estado;
	pedidoARetornar->cantidadPlatos = pedidoObtenido->cantidadPlatos;
	pedidoARetornar->platos = list_create();
	for(int i=0; i<pedidoObtenido->cantidadPlatos; i++){
		t_obtener_pedido_plato_s* pedidoPlato = (t_obtener_pedido_plato_s*) list_get(pedidoObtenido->platos,i);
		t_obtener_pedido_plato_s* pedidoPlatoDup = malloc(sizeof(t_obtener_pedido_plato_s));
		pedidoPlatoDup->cantidad = pedidoPlato->cantidad;
		pedidoPlatoDup->cantidadLista = pedidoPlato->cantidadLista;
		strcpy(pedidoPlatoDup->comida,pedidoPlato->comida);
		list_add(pedidoARetornar->platos,pedidoPlatoDup);
	}
	uint32_t size2 = sizeof(uint32_t)*3 + pedidoARetornar->nombre_restaurante_size + sizeof(t_obtener_pedido_plato_s) * pedidoARetornar->cantidadPlatos;
	t_buffer mensaje2 = serializar_mensaje(pedidoARetornar,T_CONSULTAR_PEDIDO_RESPUESTA,size2,peticion->socketCliente->socket);
	if(!enviar_mensaje(&mensaje2)){
		log_error(logger, "No se pudo enviar el mensaje de confirmación al cliente.");
	}

	liberarPedido(pedidoObtenido);

	free(pedidoARetornar->nombre_restaurante);
	list_destroy_and_destroy_elements(pedidoARetornar->platos,&free);
	free(pedidoARetornar);
	free(mensaje2.data);
}

void platoListo(t_peticion* peticion){

	log_debug(logger,"=== Ejecutando PLATO LISTO ===");
	t_plato_listo_s* platoListo = deserializar_mensaje(peticion->mensaje);
	log_info(logger,"ID = %d",platoListo->id_pedido);
	log_info(logger,"Nombre Plato = %s",platoListo->nombre_plato);
	log_info(logger,"Nombre Restaurante = %s",platoListo->nombre_restaurante);

	// Enviar a comanda plato listo
	uint32_t socketComanda = crear_conexion(config->ip_comanda,config->puerto_comanda);
	uint32_t sizeMsg = sizeof(uint32_t)*3 + platoListo->nombre_plato_size + platoListo->nombre_restaurante_size;
	t_buffer mensaje = serializar_mensaje(platoListo,T_PLATO_LISTO,sizeMsg,socketComanda);

	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el mensaje a la Comanda.");
		responderConEstado(false,peticion->socketCliente);
		free(platoListo->nombre_plato);
		free(platoListo->nombre_restaurante);
		free(platoListo);
		free(mensaje.data);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	free(mensaje.data);
	log_debug(logger,"=== Mensaje PLATO LISTO enviado a la Comanda. Esperando respuesta... ===");

	t_buffer respuesta = recibir_mensaje(socketComanda);
	if(respuesta.msj_type != T_RESULTADO_OPERACION){
		log_error(logger, "Hubo un error en la respuesta de la Comanda.");
		responderConEstado(false,peticion->socketCliente);
		free(platoListo->nombre_plato);
		free(platoListo->nombre_restaurante);
		free(platoListo);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	t_resultado_operacion* resultadoGuardado = (t_resultado_operacion*) deserializar_mensaje(&respuesta);
	log_info(logger,"=== Resultado de la operación: %d ===",resultadoGuardado->resultado);
	if(!resultadoGuardado->resultado){
		responderConEstado(false,peticion->socketCliente);
		free(resultadoGuardado);
		free(respuesta.data);
		free(platoListo->nombre_plato);
		free(platoListo->nombre_restaurante);
		free(platoListo);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	free(resultadoGuardado);
	free(respuesta.data);

	// Pedirle a comanda el pedido
	t_pedido_s* obtenerPedido = malloc(sizeof(t_pedido_s));
	obtenerPedido->id_pedido = platoListo->id_pedido;
	obtenerPedido->nombre_restaurante = strdup(platoListo->nombre_restaurante);
	obtenerPedido->nombre_restaurante_size = platoListo->nombre_restaurante_size;
	uint32_t size = sizeof(uint32_t)*2 + obtenerPedido->nombre_restaurante_size;
	t_buffer mensajeComanda = serializar_mensaje(obtenerPedido,T_OBTENER_PEDIDO,size,socketComanda);

	free(platoListo->nombre_plato);

	if(!enviar_mensaje(&mensajeComanda)){
		log_error(logger, "No se pudo enviar el mensaje a la Comanda.");
		responderConEstado(false,peticion->socketCliente);
		free(obtenerPedido->nombre_restaurante);
		free(obtenerPedido);
		free(platoListo->nombre_restaurante);
		free(platoListo);
		free(mensajeComanda.data);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	free(mensajeComanda.data);
	free(obtenerPedido->nombre_restaurante);
	free(obtenerPedido);
	log_debug(logger,"=== Mensaje OBTENER PEDIDO enviado a la Comanda. Esperando respuesta... ===");

	// Recibimos la respuesta de la comanda
	t_buffer respuestaComanda = recibir_mensaje(socketComanda);
	if(respuestaComanda.msj_type != T_OBTENER_PEDIDO_RESPUESTA)
	{
		log_error(logger, "Hubo un error en la respuesta.");
		responderConEstado(false,peticion->socketCliente);
		free(platoListo->nombre_restaurante);
		free(platoListo);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	t_obtener_pedido_s* pedidoObtenido = (t_obtener_pedido_s*) deserializar_mensaje(&respuestaComanda);
	if(pedidoObtenido->estado == -1){
		log_error(logger, "La Comanda no pudo devolver el pedido.");
		responderConEstado(false,peticion->socketCliente);
		free(respuestaComanda.data);
		free(platoListo->nombre_restaurante);
		free(platoListo);
		liberarPedido(pedidoObtenido);
		if(socketComanda > 0) close(socketComanda);
		return;
	}

	free(respuestaComanda.data);
	close(socketComanda);
	imprimirPedido(pedidoObtenido);

	// Comparar
	if(pedidoObtenido->estado == PEDIDO_TERMINADO){
		pthread_mutex_lock(&mutexColaNew);
		pthread_mutex_lock(&mutexColaExit);
		pthread_mutex_lock(&mutexColaBlock);
		pthread_mutex_lock(&mutexColaReady);
		pthread_mutex_lock(&mutexColaExec);

		t_PCB* pedido;
		pedido = obtenerPCBSegunIdPedidoYRestaurante(colaBlock,platoListo->id_pedido,platoListo->nombre_restaurante);
		if(pedido == NULL)
			pedido = obtenerPCBSegunIdPedidoYRestaurante(colaReady,platoListo->id_pedido,platoListo->nombre_restaurante);
		if(pedido == NULL)
			pedido = obtenerPCBSegunIdPedidoYRestaurante(listaExec,platoListo->id_pedido,platoListo->nombre_restaurante);

		log_debug(logger, "=== El pedido %d está LISTO. Si hay un repartidor esperando en la puerta del Restaurante ya puede retirarlo.",pedido->id_pedido);

		// Lo asignamos como LISTO por si el repartidor todavía no llegó
		pedido->listo = true;

		// Si el repartidor está esperando en la puerta del restaurante...
		if(pedido->motivoBloqueo == PEDIDO_SIN_TERMINAR){
			sem_post(pedido->repartidor_asignado->enEspera);
		}

		pthread_mutex_unlock(&mutexColaNew);
		pthread_mutex_unlock(&mutexColaExit);
		pthread_mutex_unlock(&mutexColaBlock);
		pthread_mutex_unlock(&mutexColaReady);
		pthread_mutex_unlock(&mutexColaExec);
	}
	liberarPedido(pedidoObtenido);
	free(platoListo->nombre_restaurante);
	free(platoListo);
	responderConEstado(true,peticion->socketCliente);
}

void finalizarPedido(t_PCB* pedido){

	log_debug(logger, "PEDIDO FINALIZADO: Se avisará a la Comanda y al Cliente dueño del pedido el evento...");
	// Le avisamos a la Comanda....
	t_pedido_s* pedidoTerminado = malloc(sizeof(t_pedido_s));
	pedidoTerminado->id_pedido = pedido->id_pedido;
	pedidoTerminado->nombre_restaurante = strdup(pedido->restaurante);
	pedidoTerminado->nombre_restaurante_size = strlen(pedidoTerminado->nombre_restaurante) + 1;

	uint32_t socketComanda = crear_conexion(config->ip_comanda,config->puerto_comanda);
	uint32_t sizeMsg = sizeof(uint32_t)*2 + pedidoTerminado->nombre_restaurante_size;
	t_buffer mensaje = serializar_mensaje(pedidoTerminado,T_FINALIZAR_PEDIDO,sizeMsg,socketComanda);

	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar el mensaje a la Comanda.");
		free(mensaje.data);
		free(pedidoTerminado->nombre_restaurante);
		free(pedidoTerminado);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	free(mensaje.data);
	log_trace(logger,"=== Mensaje FINALIZAR PEDIDO enviado a la Comanda. Esperando respuesta... ===");

	t_buffer respuesta = recibir_mensaje(socketComanda);
	if(respuesta.msj_type != T_RESULTADO_OPERACION){
		log_error(logger, "Hubo un error en la respuesta de la Comanda.");
		free(pedidoTerminado->nombre_restaurante);
		free(pedidoTerminado);
		if(socketComanda > 0) close(socketComanda);
		return;
	}
	t_resultado_operacion* resultadoGuardado = (t_resultado_operacion*) deserializar_mensaje(&respuesta);
	log_info(logger,"=== Resultado de la operación: %d ===",resultadoGuardado->resultado);
	if(!resultadoGuardado->resultado)
		log_error(logger, "La Comanda no pudo borrar el pedido.");
	else
		log_debug(logger, "La Comanda borró el pedido satisfactoriamente.");

	free(resultadoGuardado);
	free(respuesta.data);
	close(socketComanda);

	// Le avisamos al Cliente
	t_cliente_app* cliente = obtenerClienteSegunId(pedido->id_cliente);
	uint32_t socketCliente = crear_conexion(cliente->host,cliente->puerto);
	t_buffer mensajeCliente = serializar_mensaje(pedidoTerminado,T_FINALIZAR_PEDIDO,sizeMsg,socketCliente);

	if(!enviar_mensaje(&mensajeCliente)){
		log_error(logger, "No se pudo enviar el mensaje al Cliente.");
		free(mensajeCliente.data);
		free(pedidoTerminado->nombre_restaurante);
		free(pedidoTerminado);
		if(socketCliente > 0) close(socketCliente);
		return;
	}
	log_debug(logger,"=== Mensaje FINALIZAR PEDIDO enviado al Cliente.===");

	free(mensajeCliente.data);
	free(pedidoTerminado->nombre_restaurante);
	free(pedidoTerminado);
	close(socketCliente);
}

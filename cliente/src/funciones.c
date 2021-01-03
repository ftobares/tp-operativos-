#include "funciones.h"

void consultarRestaurantes(){
	log_info(logger,"Se ejecuto el comando <consultar_restaurantes>."); 

	t_buffer mensaje = crear_buffer_sin_cuerpo(T_CONSULTAR_RESTAURANTES, socket_conectado.socket);
	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar la consulta de restaurantes a la App.");
		free(mensaje.data);
		return;
	}
	
	free(mensaje.data);

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_LISTADO_RESTAURANTES){
		log_error(logger, "Error al recibir respuesta.");
		return;
	}

	t_listado_restaurantes* restos = deserializar_mensaje(&recibido);
	printf("%s.\n",restos->listado);

	free(recibido.data);
	free(restos->listado);
	free(restos);
}

void seleccionarRestaurante(char* restaurante){
	if(restaurante == NULL ){
		perror("No se ingresaron los parametros correctos\n");
		return;
	}
	log_info(logger,"Se ejecuto el comando <seleccionar_restaurante %s>",restaurante);
	
	t_seleccionar_restaurante* resto_seleccionado = malloc(sizeof(t_seleccionar_restaurante));
	resto_seleccionado->id_cliente_size = strlen(config->id_cliente) + 1;
	resto_seleccionado->id_cliente = config->id_cliente;
	resto_seleccionado->id_restaurante_size = strlen(restaurante) + 1;
	resto_seleccionado->id_restaurante = restaurante;
	uint32_t size = sizeof(uint32_t)*2 + resto_seleccionado->id_restaurante_size + resto_seleccionado->id_cliente_size;

	t_buffer mensaje = serializar_mensaje(resto_seleccionado,T_SELECCIONAR_RESTAURANTE,size,socket_conectado.socket);
	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar la consulta de restaurantes a la App.");
		free(mensaje.data);
		free(resto_seleccionado->id_restaurante);
		free(resto_seleccionado);
		return;
	}

	free(mensaje.data);
	free(resto_seleccionado->id_restaurante);
	free(resto_seleccionado);

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_info(logger,"Error al recibir la respuesta.");
		free(recibido.data);
		return;
	}

	t_resultado_operacion* respuesta = deserializar_mensaje(&recibido);
	if(!respuesta->resultado)
		printf("FAIL\n");
	else
		printf("OK\n");

	free(recibido.data);
	free(respuesta);
}

void consultarPlatos(char* restaurante){

	if(moduloConectado == T_SINDICATO && restaurante == NULL){
		printf("Ingrese el parametro correspondiente.\n");
		return;
	}

	log_info(logger,"Se ejecuto el comando <consultar_platos>");
	t_buffer mensaje;

	if(moduloConectado == T_SINDICATO){

	t_nombre_restaurante_s* _restaurante = malloc(sizeof(t_nombre_restaurante_s));
	_restaurante->nombre_restaurante = strdup(restaurante);
	_restaurante->nombre_restaurante_size = string_length(restaurante);
	uint32_t size = sizeof(_restaurante->nombre_restaurante_size) + _restaurante->nombre_restaurante_size;
	mensaje = serializar_mensaje(_restaurante,T_CONSULTAR_PLATOS,size,socket_conectado.socket);	//	invalid write of size en serializer de este mensaje.
	}
	else
		mensaje = crear_buffer_sin_cuerpo(T_CONSULTAR_PLATOS, socket_conectado.socket);	///MODIFICAR SOLO ID_

	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar la consulta de platos a la App.");
		free(mensaje.data);
		return;
	}

	free(mensaje.data);

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_CONSULTAR_PLATOS_RESPUESTA){
		log_info(logger,"Error al recibir la respuesta.");
		//free(recibido.data);
		return;
	}
	t_single_text_s* listadoPlatos = deserializar_mensaje(&recibido);
	printf("Los platos del restaurante son %s\n",listadoPlatos->text);

	free(recibido.data);
	free(listadoPlatos->text);
	free(listadoPlatos);
}

void crearPedido(){
	log_info(logger,"Se ejecuto el comando <crear_pedido>");

	t_buffer mensaje = crear_buffer_sin_cuerpo(T_CREAR_PEDIDO, socket_conectado.socket);
	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar la solicitud de creación de pedido a la App.");
		free(mensaje.data);
		return;
	}

	free(mensaje.data);
	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_CREAR_PEDIDO_RESPUESTA){
		log_info(logger,"Error al recibir la respuesta.");
		free(recibido.data);
		return;
	}
	t_id_pedido* pedido = deserializar_mensaje(&recibido);
	if(!pedido->id_pedido)
		printf("NO SE PUDO CREAR EL PEDIDO");
	/*else
		printf("%d\n",pedido->id_pedido);*/

	printf("El ID del pedido creado es --> %i.\n",pedido->id_pedido);

	log_info(logger,"Se creo el pedido correctamente.");

	free(recibido.data);
	free(pedido);
}

void aniadirPlato(char* plato, char* id_pedido){

	if(plato == NULL || id_pedido == NULL ){
		perror("No se ingresaron los parametros correctos\n");
		return;
	}
	int * idPedido = malloc(sizeof(int));
	if(!try_string_to_int(id_pedido, idPedido)){
		perror("No se ingreso un ID correcto\n");
		return;
	}

	log_info(logger,"Se ejecuto el comando <aniadir_plato %s %s>",plato,id_pedido);

	t_aniadir_plato* agregarPlato = malloc(sizeof(t_aniadir_plato));
	agregarPlato->id_pedido = *idPedido;
	agregarPlato->plato = plato;
	agregarPlato->plato_size = strlen(agregarPlato->plato) + 1;

	uint32_t size = sizeof(uint32_t)*2 + agregarPlato->plato_size;

	t_buffer mensaje = serializar_mensaje(agregarPlato,T_ANIADIR_PLATO,size,socket_conectado.socket);
	if(!enviar_mensaje(&mensaje)){
		log_error(logger, "No se pudo enviar la solicitud de agregar plato a la App.");
		free(idPedido);
		free(plato);
		free(id_pedido);
		free(agregarPlato);
		free(mensaje.data);
		return;
	}

	free(idPedido);
	free(agregarPlato);
	free(mensaje.data);

	// Esperamos respuesta
	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_info(logger,"Error al recibir la respuesta.");
		free(recibido.data);
		return;
	}
	t_resultado_operacion* respuesta = deserializar_mensaje(&recibido);
	if(respuesta->resultado)
		printf("El plato %s se aniadio correctamente al pedido %s.\n",plato,id_pedido);
	else
		printf("NO SE PUDO AGREGAR EL PLATO AL PEDIDO SOLICITADO.\n");
	
	free(plato);
	free(id_pedido);
	free(respuesta);
	free(recibido.data);
}


void obtenerRestaurante(char* restaurante){
	if(restaurante == NULL ){
		perror("No se ingresaron los parametros correctos\n");
		return -1;
	}
	log_info(logger,"Se ejecuto el comando <obtener_restaurante %s>",restaurante);

	t_nombre_restaurante_s* resto = malloc(sizeof(t_nombre_restaurante_s));
	resto->nombre_restaurante = restaurante;
	resto->nombre_restaurante_size = string_length(restaurante) + 1;
	uint32_t size = resto->nombre_restaurante_size + sizeof(resto->nombre_restaurante_size);
	t_buffer enviar = serializar_mensaje(resto,T_OBTENER_RESTAURANTE,size,socket_conectado.socket);
	if(!enviar_mensaje(&enviar)){
		log_error(logger,"No se pudo enviar el mensaje guardar plato");
		free(enviar.data);
		free(resto);
		return;
	}

	t_buffer recibo = recibir_mensaje(socket_conectado.socket);
	if(recibo.msj_type != T_OBTENER_RESTAURANTE_RESPUESTA){
		log_error(logger,"No se recibio la respuesta del OBTENER_RESTAURANTE");
		printf("No se encontro el restaurante solicitado.\n");
		//free(recibo.data);
		free(resto);
		free(enviar.data);
		return;
	}
	t_obtener_restaurante_respuesta_s* d_resto = deserializar_mensaje(&recibo);

	printf("La informacion del restaurante %s es:\n-ID: %i.\n-Posicion: [%i,%i].\n-CantidadCocineros: %i.\n-CantidadPlatos: %i.\n-CantidadHornos: %i.\n",
			restaurante,d_resto->id,d_resto->posicion->x,d_resto->posicion->y,d_resto->cantCocineros,d_resto->cantPlatos,d_resto->cantidadHornos);
	printf("-AfinidadCocineros: [");

	void printearAfinidad(t_cocinero_s* cocinero){
		printf("%s,",cocinero->afinidad);
	}
	list_iterate(d_resto->cocineros,printearAfinidad);
	printf("]\n");

	printf("-Platos: \n");

	void printearPlatos(t_plato_s* plato){
		printf("\t Nombre: %s, Precio: %i, ID: %i.\n ",plato->nombre,plato->precio,plato->id);
	}
	list_iterate(d_resto->platos,printearPlatos);
	

	log_info(logger,"El mensaje obtenerRestaurante se ejecuto correctamente");
	free(enviar.data);
	free(resto);
	//free(recibo.data);
	free(d_resto);
}

void guardarPlato(char* restaurante, char* id_pedido, char* comida, char* cantidad){
	if(comida == NULL || id_pedido == NULL || restaurante == NULL || cantidad == NULL){
		perror("No se ingresaron los parametros correctos\n");
		return;
	}
	
	t_guardar_plato_s* plato = malloc(sizeof(t_guardar_plato_s));
	plato->nombre_restaurante = string_new();
	string_append(&plato->nombre_restaurante,restaurante);
	plato->nombre_restaurante_size = string_length(plato->nombre_restaurante) + 1;
	plato->id_pedido = malloc(sizeof(uint32_t));
	plato->cantidad = malloc(sizeof(uint32_t));
	plato->id_pedido = atoi(id_pedido);
	plato->cantidad = atoi(cantidad);
	plato->nombre_plato = string_new();
	string_append(&plato->nombre_plato,comida);
	plato->nombre_plato_size = string_length(plato->nombre_plato) + 1;
	uint32_t size = plato->nombre_restaurante_size + 4 * sizeof(plato->nombre_restaurante_size) + plato->nombre_plato_size;
	t_buffer enviar = serializar_mensaje(plato,T_GUARDAR_PLATO,size,socket_conectado.socket); 

	if(!enviar_mensaje(&enviar)){
		log_error(logger,"No se pudo enviar el mensaje guardar plato");
		free(plato);
		//free(enviar.data);
	}
	t_buffer recibido = recibir_mensaje(socket_conectado.socket);

	if(recibido.msj_type == T_ERROR){
		log_error(logger,"Error al recibir la respuesta de guardar plato.");
		//free(recibido.data);
		free(plato);
		free(enviar.data);
		return;
	}
	t_resultado_operacion* resultado = deserializar_mensaje(&recibido);

	if(resultado->resultado){
		printf("El plato pudo agregar correctamente al pedido solicitado.\n");
		log_info(logger,"El mensaje se recibio correctamente");
	}
	else{
		printf("El plato no pudo agregarse al pedido solicitado.\n");
		log_error(logger,"El modulo devolvio ERROR al recibir el mensaje");
	}

	free(resultado);
	free(recibido.data);
	free(plato);
	free(enviar.data);
	return;
}

void confirmarPedido(char* id_pedido, char* restaurante){	
	if( id_pedido == NULL ){
		perror("No se ingresaron los parametros correctos\n");
		return;
	}
	int * idPedido = malloc(sizeof(int));
	if(!try_string_to_int(id_pedido, idPedido)){
		perror("No se ingreso un ID correcto\n");
		return;
	}
	log_info(logger,"Se ejecuto el comando <confirmar_pedido %d>",*idPedido);

	t_buffer envio;

	if(moduloConectado == T_COMANDA || moduloConectado == T_SINDICATO){
	t_pedido_s* confirmoPed = malloc(sizeof(t_id_pedido));
	confirmoPed->id_pedido = *idPedido;
	confirmoPed->nombre_restaurante = string_new();
	string_append(&confirmoPed->nombre_restaurante,restaurante);
	confirmoPed->nombre_restaurante_size = string_length(confirmoPed->nombre_restaurante);
	uint32_t size = 2*sizeof(uint32_t) + confirmoPed->nombre_restaurante_size;
	envio = serializar_mensaje(confirmoPed,T_CONFIRMAR_PEDIDO,size,socket_conectado.socket);
	free(confirmoPed);
	}
	else if(moduloConectado == T_RESTAURANTE || moduloConectado == T_APP){
	t_id_pedido* confirmoPed = malloc(sizeof(t_id_pedido));
	confirmoPed->id_pedido = *idPedido;
	uint32_t size = sizeof(uint32_t);
	envio = serializar_mensaje(confirmoPed,T_CONFIRMAR_PEDIDO_SOLO_ID,size,socket_conectado.socket);
	free(confirmoPed);
	}
	
	if(!enviar_mensaje(&envio)){
		log_error(logger,"No se pudo enviar el mensaje confirmar pedido");
		//free(envio.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	t_resultado_operacion* respuesta = deserializar_mensaje(&recibido);		// LA RESPUESTA ES DEL MISMO TIPO
	if(respuesta->resultado)
		printf("El pedido se confirmo correctamente.\n");
	else{
		printf("Hubo un error al confirmar el pedido\n");
		free(respuesta);
		free(idPedido);
		return;
	}

	free(recibido.data);
	free(respuesta);
	free(idPedido);
}

void platoListo(char* restaurante, char* id_pedido, char* comida){
	if(restaurante == NULL || id_pedido == NULL || comida == NULL){
		perror("No se ingresaron los parametros correctos\n");
		return;
	}

	t_plato_listo_s* plato = malloc(sizeof(t_plato_listo_s));
	plato->id_pedido = atoi(id_pedido);
	plato->nombre_restaurante = string_new();
	string_append(&plato->nombre_restaurante,restaurante);
	plato->nombre_plato = string_new();
	string_append(&plato->nombre_plato,comida);
	plato->nombre_restaurante_size = string_length(plato->nombre_restaurante);
	plato->nombre_plato_size = string_length(plato->nombre_plato);
	uint32_t size = plato->nombre_restaurante_size + plato->nombre_plato_size + 2 * sizeof(plato->nombre_restaurante_size) + sizeof(plato->id_pedido);

	t_buffer enviar = serializar_mensaje(plato,T_PLATO_LISTO,size,socket_conectado.socket);
	if(!enviar_mensaje(&enviar)){
		log_error(logger,"No se pudo enviar el mensaje confirmar pedido");
		free(plato);
		free(enviar.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"Se recibio un resultado erroneo de la operacion por parte del modulo conectado.");
		free(plato);
		free(enviar.data);
		return;
	}
	t_resultado_operacion* resultado = deserializar_mensaje(&recibido);
	if(resultado->resultado){
		printf("El plato %s del pedido %s esta listo.\n",comida,id_pedido);
		log_info(logger,"El mensaje se recibio correctamente");
	}
	else{
		printf("No se pudo dejar el plato listo.\n");
		log_error(logger,"El modulo devolvio ERROR al recibir el mensaje");
		free(plato);
		free(enviar.data);
		free(resultado);
		return;
	}

	free(recibido.data);
	free(plato);
	free(enviar.data);
	free(resultado);
}

void consultarPedido(char* id_pedido){	//ver como quedo
	if(id_pedido == NULL ){
		perror("No se ingresaron los parametros correctos\n");
		return;
	}
	int * idPedido = malloc(sizeof(int));
	if(!try_string_to_int(id_pedido, idPedido)){
		perror("No se ingreso un ID correcto\n");
		return;
	}
	log_info(logger,"Se ejecuto el comando <consultar_pedido %d>",*idPedido);

	t_id_pedido* consultarPed = malloc(sizeof(t_id_pedido));
	consultarPed->id_pedido = *idPedido;
	uint32_t size = sizeof(uint32_t);
	t_buffer envio = serializar_mensaje(consultarPed,T_CONSULTAR_PEDIDO,size,socket_conectado.socket);
	if(!enviar_mensaje(&envio)){
		log_error(logger,"No se pudo enviar el mensaje consultar pedido.");
		free(idPedido);
		free(consultarPed);
		free(envio.data);
		return;
	}

	printf("ID Pedido = %d\n", consultarPed->id_pedido);
	t_buffer recibido = recibir_mensaje(socket_conectado.socket);

	if(recibido.msj_type != T_CONSULTAR_PEDIDO_RESPUESTA){
		log_error(logger,"Hubo un problema en la operacion de consultar pedido.");
		free(idPedido);
		free(consultarPed);
		free(envio.data);
		return;
	}
	t_consultar_pedido_s* respuesta = deserializar_mensaje(&recibido);
	printf("El nombre del restaurante del pedido %s es:\n",respuesta->nombre_restaurante);
	printf("La informacion del estado del pedido %i es:\n",respuesta->estado);
	if(respuesta->estado != -1){
	printf("La informacion del pedido %s es:\n",id_pedido);
	printf("Restaurante: %s.\nEstado: %i.\nCantidadPlatos: %i.\n",respuesta->nombre_restaurante,respuesta->estado,respuesta->cantidadPlatos);
	for(int i = 0; i < respuesta->cantidadPlatos; i++){
		t_obtener_pedido_plato_s* plato = list_get(respuesta->platos,i);
		printf("Cantidad: %i.\nComida:%s \nCantidadLista: %i\n",plato->cantidad,plato->comida,plato->cantidadLista);
		free(plato);
	}
	}
	else{
		printf("El pedido ingresado no existe. Cree un pedido antes de consultarlo\n");
		log_error(logger,"El pedido ingresado no existe.");
		free(recibido.data);
		free(respuesta);
		free(idPedido);
		return;
	}

	free(recibido.data);
	free(respuesta);
	free(idPedido);
}

void obtenerPedido(char* restaurante, char* id_pedido){
	if(restaurante == NULL || id_pedido == NULL ){
		perror("No se ingresaron los parametros correctos\n");
		return -1;
	}

	t_pedido_s* pedido = malloc(sizeof(t_pedido_s));
	pedido->nombre_restaurante = string_new();
	string_append(&pedido->nombre_restaurante,restaurante);
	pedido->nombre_restaurante_size = string_length(pedido->nombre_restaurante) + 1;
	pedido->id_pedido = atoi(id_pedido);
	uint32_t size = sizeof(pedido->nombre_restaurante_size) + pedido->nombre_restaurante_size + sizeof(pedido->id_pedido);
	t_buffer enviar = serializar_mensaje(pedido,T_OBTENER_PEDIDO,size,socket_conectado.socket);

	if(!enviar_mensaje(&enviar)){
		log_error(logger,"El mensaje obtener pedido no pudo ser enviado.");
		free(pedido);
		free(enviar.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_OBTENER_PEDIDO_RESPUESTA){
		log_error(logger,"El tipo de respuesta de obtener pedido no fue la correcta.");
		printf("Hubo un error al obtener el pedido.\n");
		//free(recibido.data);
		free(pedido);
		free(enviar.data);
		return;
	}
	t_obtener_pedido_s* d_pedido = deserializar_mensaje(&recibido);
	if(d_pedido->cantidadPlatos == 0){
		printf("El pedido consultado no tiene ningun plato\n");
		free(recibido.data);
		free(pedido);
		free(enviar.data);
		free(d_pedido);
		return;
	}
	printf("El estado del pedido es %i con una cantidad de platos = %i.\n",d_pedido->estado,d_pedido->cantidadPlatos);
	printf("Los platos son: \n");

	void* mostrarPlatos(t_obtener_pedido_plato_s* plato){
		printf("Nombre: %s. Cantidad: %i. CantidadLista: %i.\n",plato->comida,plato->cantidad,plato->cantidadLista);
	}

	list_iterate(d_pedido->platos,mostrarPlatos);

	log_info(logger,"El mensaje obtener pedido se ejecuto correctamente.");
	return;
}

void finalizarPedido(char* restaurante, char* id_pedido){
	if(restaurante == NULL || id_pedido == NULL ){
		perror("No se ingresaron los parametros correctos\n");
		return;
	}	

	t_pedido_s* pedido = malloc(sizeof(t_pedido_s));
	pedido->nombre_restaurante = string_new();
	string_append(&pedido->nombre_restaurante,restaurante);
	pedido->nombre_restaurante_size = string_length(pedido->nombre_restaurante) + 1;
	pedido->id_pedido = atoi(id_pedido);
	uint32_t size = sizeof(pedido->nombre_restaurante_size) + pedido->nombre_restaurante_size + sizeof(pedido->id_pedido);
	t_buffer enviar = serializar_mensaje(pedido,T_FINALIZAR_PEDIDO,size,socket_conectado.socket);

	if(!enviar_mensaje(&enviar)){
		log_error(logger,"El mensaje obtener pedido no pudo ser enviado.");
		free(pedido);
		free(enviar.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"La respuesta del mensaje obtener pedido no pudo ser recibido.");
		free(pedido);
		free(enviar.data);
		free(recibido.data);
		return;
	}

	t_resultado_operacion* resultado = deserializar_mensaje(&recibido);

	if(resultado->resultado){
		log_info(logger,"El mensaje se recibio correctamente");
	}
	else{
		log_error(logger,"El modulo devolvio ERROR al recibir el mensaje");
	}
	
	free(resultado);
	free(pedido);
	free(enviar.data);
	free(recibido.data);

	
	log_info(logger,"Se ejecuto correctamente el comando <finalizar_pedido %s %s>",restaurante,id_pedido);
}

void terminarPedido(char* restaurante, char* id_pedido){
	if(restaurante == NULL || id_pedido == NULL ){
		perror("No se ingresaron los parametros correctos\n");
		return -1;
	}	

	t_pedido_s* pedido = malloc(sizeof(t_pedido_s));
	pedido->nombre_restaurante = string_new();
	string_append(&pedido->nombre_restaurante,restaurante);
	pedido->nombre_restaurante_size = string_length(pedido->nombre_restaurante) + 1;
	pedido->id_pedido = atoi(id_pedido);
	uint32_t size = sizeof(pedido->nombre_restaurante_size) + pedido->nombre_restaurante_size + sizeof(pedido->id_pedido);
	t_buffer enviar = serializar_mensaje(pedido,T_TERMINAR_PEDIDO,size,socket_conectado.socket);

	if(!enviar_mensaje(&enviar)){
		log_error(logger,"El mensaje obtener pedido no pudo ser enviado.");
		free(pedido);
		free(enviar.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"La respuesta del mensaje obtener pedido no pudo ser recibido.");
		free(pedido);
		free(enviar.data);
		free(recibido.data);
		return;
	}

	t_resultado_operacion* resultado = deserializar_mensaje(&recibido);

	if(resultado->resultado){
		log_info(logger,"El mensaje se recibio correctamente");
	}
	else{
		log_error(logger,"El modulo devolvio ERROR al recibir el mensaje");
	}
	
	free(resultado);
	free(pedido);
	free(enviar.data);
	free(recibido.data);



	log_info(logger,"Se ejecuto el comando <terminar_pedido %s %s>",restaurante,id_pedido);
}

void obtenerReceta(char* plato){
	if(plato == NULL){
		perror("No se ingresaron los parametros correctos\n");
		return -1;
	}

	t_obtener_receta_s* receta = malloc(sizeof(t_obtener_receta_s));
	receta->nombre_plato = string_new();
	string_append(&receta->nombre_plato,plato);
	receta->nombre_plato_size = string_length(receta->nombre_plato) + 1;
	uint32_t size = receta->nombre_plato_size + sizeof(receta->nombre_plato_size);
	
	t_buffer enviar = serializar_mensaje(receta,T_OBTENER_RECETA,size,socket_conectado.socket);
	if(!enviar_mensaje(&enviar)){
		log_error(logger,"No se pudo enviar el mensaje obtener receta.");
		free(receta);
		free(enviar.data);
		return;
	}
	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_OBTENER_RECETA_RESPUESTA){
		log_error(logger,"No se recibieron los datos de la receta.");
		free(receta);
		free(enviar.data);
		free(recibido.data);
		return;
	}

	t_obtener_receta_respuesta* pasos = deserializar_mensaje(&recibido);
	
	if(pasos->cantidadPasos <= 0){
		log_error(logger,"La receta recibida no tiene pasos.");
		free(receta);
		free(enviar.data);
		free(recibido.data);
		free(pasos);
		return;
	}

	printf("La receta del plato %s con %i pasos es:\n",plato,pasos->cantidadPasos);
	for(int i = 0; i < pasos->cantidadPasos; i++){
		t_receta_s* pasoReceta = list_get(pasos->pasos,i);
		printf("Paso numero %i --> %s de tiempo %i\n",i+1,pasoReceta->paso,pasoReceta->tiempo);
		free(pasoReceta);
	}
	printf("\n");
	
	free(receta);
	free(enviar.data);
	//free(recibido.data);
	free(pasos);
	log_info(logger,"Se ejecuto el comando <obtener_receta %s> correctamente.",plato);
}

void guardarPedido(char* restaurante, char* id_pedido){
	
	if(restaurante == NULL || id_pedido == NULL){
		perror("No se ingresaron los parametros correctos\n");
		return -1;
	}

	t_pedido_s* pedido = malloc(sizeof(t_pedido_s));
	pedido->nombre_restaurante = string_new();
	string_append(&pedido->nombre_restaurante,restaurante);
	pedido->nombre_restaurante_size = string_length(pedido->nombre_restaurante);
	pedido->id_pedido = atoi(id_pedido);
	uint32_t size = sizeof(pedido->nombre_restaurante_size) + pedido->nombre_restaurante_size + sizeof(pedido->id_pedido);
	t_buffer enviar = serializar_mensaje(pedido,T_GUARDAR_PEDIDO,size,socket_conectado.socket);
	printf("NOMBRE RESTO --> %s.\n",pedido->nombre_restaurante);
	if(!enviar_mensaje(&enviar)){
		log_error(logger,"El mensaje obtener pedido no pudo ser enviado.");
		free(pedido);
		free(enviar.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socket_conectado.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"La respuesta del mensaje obtener pedido no pudo ser recibido.");
		free(pedido);
		free(enviar.data);
		free(recibido.data);
		return;
	}

	t_resultado_operacion* resultado = deserializar_mensaje(&recibido);

	if(resultado->resultado){
		printf("Se pudo guardar el pedido correctamente.\n");
		log_info(logger,"El mensaje se recibio correctamente");
	}
	else{
		printf("El pedido no se pudo guardar.\n");
		log_error(logger,"El modulo devolvio ERROR al recibir el mensaje");
	}
	
	free(resultado);
	free(pedido);
	free(enviar.data);
	free(recibido.data);

	log_info(logger,"El mensaje guardar pedido se concreto correctamente.");
}
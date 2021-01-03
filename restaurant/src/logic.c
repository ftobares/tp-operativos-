#include "logic.h"

void* consultarPlatos(t_buffer* buffer) {
	
	printf("Consultando platos a Sindicato\n");
	t_nombre_restaurante_s* restaurante = malloc(sizeof(t_nombre_restaurante_s));
	restaurante->nombre_restaurante = string_new();
	string_append(&restaurante->nombre_restaurante,config->nombre_restaurante);
	restaurante->nombre_restaurante_size = string_length(restaurante->nombre_restaurante) + 1;
	uint32_t size = sizeof(restaurante->nombre_restaurante_size) + restaurante->nombre_restaurante_size;
	
	printf("Mensaje OK. Creando socket de conexión\n");

	t_socket socketSindicatoNuevo = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

	printf("Conectandose con sindicato...\n");
	
	conectar_socket(socketSindicatoNuevo);

	t_buffer enviar = serializar_mensaje(restaurante,T_CONSULTAR_PLATOS,size,socketSindicatoNuevo.socket);
	printf("Serialización OK\n");
	if(!enviar_mensaje(&enviar)){
		log_error(logger,"No se pudo enviar el mensaje consultar platos a la comanda.");	//TODO AGREGAR LOGGER EN logic.h
		free(restaurante);
		free(enviar.data);
		return;
	}

	free(restaurante);
	free(enviar.data);
	log_info(logger,"El mensaje consultar_platos se envio correctamente a la comanda.");

	t_buffer recibido = recibir_mensaje(socketSindicatoNuevo.socket);
	if(recibido.msj_type != T_CONSULTAR_PLATOS_RESPUESTA){
		log_error(logger,"No se recibio la informacion de consultar platos de sindicato.");
		free(recibido.data);
		return;
	}

	printf("Llegó respuesta de sindicato\n");
	t_single_text_s* i_recibo = deserializar_mensaje(&recibido);

	printf("Deserializada respuesta sindicato: %d %s\n", i_recibo->text_size, i_recibo->text);

	int size_respuesta = sizeof(t_single_text_s) + i_recibo->text_size;

	t_buffer buff_platos = serializar_mensaje(i_recibo,T_CONSULTAR_PLATOS_RESPUESTA,size_respuesta,buffer->socket);
	
	printf("Serializada respuesta a app\n");
	if(!enviar_mensaje(&buff_platos)){
		log_error(logger,"No se envio la respuesta del consultar platos.");
		free(i_recibo);
		free(buff_platos.data);
		return;
	}

	free(i_recibo);
	free(buff_platos.data);

	log_info(logger,"Se envio la respuesta de consultar platos correctamente a APP.");

}

void* respuestaConsultarPlatos(t_buffer* buffer) {
}

void* crearPedido(t_buffer* buffer){

	// Respuesta mockeada
//	uint32_t idPedido = time(NULL);

	uint32_t idPedido = obtenerIdPedido();

	// Enviar GUARDAR_PEDIDO a Sindicato (idGenerado, elNombreDeEsteRestaurante)
	t_pedido_s* pedido = malloc(sizeof(t_pedido_s));
	pedido->nombre_restaurante = string_new();
	string_append(&pedido->nombre_restaurante,config->nombre_restaurante);
	pedido->nombre_restaurante_size = string_length(pedido->nombre_restaurante) + 1;
	pedido->id_pedido = idPedido;
	uint32_t size = sizeof(pedido->nombre_restaurante_size) + pedido->nombre_restaurante_size + sizeof(pedido->id_pedido);

	printf("Mensaje OK. Creando socket de conexión\n");

	t_socket socketSindicatoNuevo = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

	printf("Conectandose con sindicato...\n");
	
	conectar_socket(socketSindicatoNuevo);

	t_buffer enviar = serializar_mensaje(pedido,T_GUARDAR_PEDIDO,size,socketSindicatoNuevo.socket);
	
	if(!enviar_mensaje(&enviar)){
		log_error(logger,"No se pudo enviar el mensaje GuardarPedido al modulo sindicato");
		free(pedido);
		free(enviar.data);
		return;
	}

	free(pedido);
	
	//CONFIRMACION DE GUARDAR PEDIDO
	t_buffer recibido = recibir_mensaje(socketSindicatoNuevo.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"No se recibio confirmacion de crear pedido por parte del sindicato");
		free(recibido.data);
		free(enviar.data);
	}
	t_resultado_operacion* resultado = deserializar_mensaje(&recibido);

	if(resultado->resultado){
		log_info(logger,"El mensaje se recibio correctamente");
	}
	else{
		log_error(logger,"El modulo devolvio ERROR al recibir el mensaje");
	}


	//ENVIO ID A MODULO SOLICITANTE
	t_id_pedido* i_pedido = malloc(sizeof(t_id_pedido));
	if(resultado->resultado){
		i_pedido->id_pedido = idPedido;
	}
	else{
		i_pedido->id_pedido = -1;
	}
	size = sizeof(i_pedido->id_pedido);
	t_buffer enviar_id = serializar_mensaje(i_pedido,T_CREAR_PEDIDO_RESPUESTA,size,buffer->socket);
	if(!enviar_mensaje(&enviar_id)){
		log_error(logger,"No se pudo enviar el ID del pedido creado al Modulo APP.");
		free(i_pedido);
		free(enviar.data);
		free(enviar_id.data);
		free(resultado);
		free(recibido.data);
		return;
	}

	free(i_pedido);
	free(enviar.data);
	free(enviar_id.data);
	free(resultado);
	free(recibido.data);

	//log_info(logger,"El mensaje guardar pedido hacia sindicato (por CREAR_PEDIDO) se concreto correctamente.");

	//printf("Recibo pedido de App. Genero ID y envio Guardar Pedido a Sindicato");
}

void* respuestaCrearPedido(t_buffer* buffer){		// ver mensaje tema tipo msj
}

void* aniadirPlato(t_buffer* buffer){			
	//printf("Recibo de App un plato y el ID del pedido. Envio Guardar Pedido a sindicato con la info del plato");

	t_aniadir_plato* agregarPlato = deserializar_mensaje(buffer);
	//	ENVIO GUARDAR PLATO A SINDICATO	//
	t_guardar_plato_s* plato = malloc(sizeof(t_guardar_plato_s));
	plato->nombre_restaurante = string_new();
	string_append(&plato->nombre_restaurante,config->nombre_restaurante);
	plato->nombre_restaurante_size = string_length(plato->nombre_restaurante) + 1;
	plato->id_pedido = agregarPlato->id_pedido;
	plato->cantidad = 1;
	plato->nombre_plato = string_new();
	string_append(&plato->nombre_plato,agregarPlato->plato);
	plato->nombre_plato_size = string_length(plato->nombre_plato) + 1;
	uint32_t size = plato->nombre_restaurante_size + 4 * sizeof(plato->nombre_restaurante_size) + plato->nombre_plato_size;

	printf("Mensaje OK. Creando socket de conexión\n");

	t_socket socketSindicatoNuevo = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

	printf("Conectandose con sindicato...\n");
	
	conectar_socket(socketSindicatoNuevo);

	t_buffer enviar = serializar_mensaje(plato,T_GUARDAR_PLATO,size,socketSindicatoNuevo.socket); 

	if(!enviar_mensaje(&enviar)){
		//log_error(logger,"No se pudo enviar el mensaje guardar plato");
		free(plato);
		free(enviar.data);
		free(agregarPlato);
		return;
	}
	//log_info(logger,"El mensaje aniadir plato se envio correctamente al sindicato.");

	free(plato);
	free(enviar.data);
	free(agregarPlato);

	//	RECIBO CONFIRMACION DE SINDICATO POR GUARDAR PLATO	//
	t_buffer recibido = recibir_mensaje(socketSindicatoNuevo.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"El mensaje aniadir plato enviado a sindicato no recibio confirmacion");
		free(recibido.data);
		return;
	}

	t_resultado_operacion* resultado = deserializar_mensaje(&recibido);
	
	if(resultado->resultado){
		log_info(logger,"El mensaje aniadir plato se recibio correctamente");
	}
	else{
		log_error(logger,"El modulo devolvio ERROR al recibir el mensaje");
	}
	//	ENVIO CONFIRMACION DE ANIADIR PLATO AL MODULO SOLICITANTE	//
	t_buffer respuesta = serializar_mensaje(resultado,T_RESULTADO_OPERACION,recibido.size,buffer->socket);
	if(!enviar_mensaje(&respuesta)){
		log_error(logger,"La confirmacion del mensaje aniadir plato no se pudo enviar al modulo correspondiente.");
		free(resultado);
		free(respuesta.data);
		return;
	}
	
	free(resultado);
	free(respuesta.data);
	log_info(logger,"El mensaje aniadir plato se concreto correctamente.");

	return;
}

void* respuestaAniadirPlato(t_buffer* buffer){
}

void* confirmarPedido(t_buffer* buffer) {

	uint32_t error = 0;
	t_id_pedido* confirmoPed = deserializar_mensaje(buffer);

	//	envio obtener pedido a sindicato	//
	
	t_pedido_s* pedido = malloc(sizeof(t_pedido_s));
	pedido->nombre_restaurante = string_new();
	string_append(&pedido->nombre_restaurante,config->nombre_restaurante);
	pedido->nombre_restaurante_size = string_length(pedido->nombre_restaurante) + 1;
	pedido->id_pedido = confirmoPed->id_pedido;
	uint32_t size = sizeof(pedido->nombre_restaurante_size) + pedido->nombre_restaurante_size + sizeof(pedido->id_pedido);

	printf("Mensaje OK. Creando socket de conexión\n");

	t_socket socketSindicatoNuevo = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

	printf("Conectandose con sindicato...\n");
	
	conectar_socket(socketSindicatoNuevo);

	t_buffer enviar = serializar_mensaje(pedido,T_OBTENER_PEDIDO,size,socketSindicatoNuevo.socket);

	if(!enviar_mensaje(&enviar)){
		//log_error(logger,"El mensaje obtener pedido no pudo ser enviado.");
		free(pedido);
		free(enviar.data);
		return;
	}
	//	RECIBO OBTENER PEDIDO DE SINDICATO	//
	t_buffer recibido = recibir_mensaje(socketSindicatoNuevo.socket);
	if(recibido.msj_type != T_OBTENER_PEDIDO_RESPUESTA){
		log_error(logger,"El tipo de respuesta de obtener pedido no fue la correcta.");
	//	free(recibido.data);
		free(pedido);
	//	free(enviar.data);
		return;
	}
	t_obtener_pedido_s* d_pedido = deserializar_mensaje(&recibido);
	if(d_pedido->cantidadPlatos == 0){
		log_info(logger,"El pedido no tiene ningun plato");
		//free(recibido.data);
		//free(pedido);
		//free(enviar.data);
		//free(d_pedido);
		//return;
	}

	printf("El pedido tiene estado %i con %i platos.\n ",d_pedido->estado,d_pedido->cantidadPlatos);

	//	GENERO PCB DEL PLATO, GUARDO ID DEL PEDIDO Y ENVIAR A PLANIFICADOR	//

	//	TODO	//


	//	FOR EACH PLATO OBTENGO LA RECETA	//

	for(int i = 0; i < d_pedido->cantidadPlatos; i++){
		t_obtener_pedido_plato_s* plato = list_get(d_pedido->platos,i);
		printf("Plato %i: %s.\n",i,plato->comida);
		//		OBTENER RECETA		//
		t_obtener_receta_s* receta = malloc(sizeof(t_obtener_receta_s));
		receta->nombre_plato = string_new();
		string_append(&receta->nombre_plato,plato->comida);
		receta->nombre_plato_size = string_length(receta->nombre_plato) + 1;
		uint32_t size = receta->nombre_plato_size + sizeof(receta->nombre_plato_size);

		printf("Mensaje OK. Creando socket de conexión\n");

		t_socket socketSindicatoReceta = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

		printf("Conectandose con sindicato...\n");
	
		conectar_socket(socketSindicatoReceta);

		t_buffer enviar = serializar_mensaje(receta,T_OBTENER_RECETA,size,socketSindicatoReceta.socket);
		if(!enviar_mensaje(&enviar)){
			log_error(logger,"No se pudo enviar el mensaje obtener receta.");
			free(receta);
			free(enviar.data);
			return;
		}
		t_buffer recibido = recibir_mensaje(socketSindicatoReceta.socket);
		if(recibido.msj_type != T_OBTENER_RECETA_RESPUESTA){
			log_error(logger,"No se recibieron los datos de la receta.");
			free(receta);
			free(enviar.data);
		//	free(recibido.data);
			error = 1;
			break;
		}

		t_obtener_receta_respuesta* recetta = deserializar_mensaje(&recibido);

		if(string_equals_ignore_case(recetta->nombre_plato,"ERROR")){
			log_error(logger,"No se pudo obtener la receta solicitada");
			error = 1;
			break;
		}
		printf("La receta del plato %s tiene %i pasos.\n",recetta->nombre_plato,recetta->cantidadPasos);
		if(recetta->cantidadPasos <= 0){
			log_error(logger,"La receta recibida no tiene pasos.");
			//free(receta);
		/*	free(enviar.data);
			free(recibido.data);
			free(recetta);*/
			//return;
		}

		t_list* pasitos = list_create();
		void* crearPaso(t_paso_s* paso){
			t_paso* pasin = malloc(sizeof(t_paso));
			pasin->tiempo = paso->tiempo;
			pasin->nombre = strdup(paso->nombre_paso);
			list_add(pasitos,pasin);
		}
		list_iterate(recetta->pasos,crearPaso);
		log_info(logger,"El nombre del plato en la iteracion %i es %s.",i,recetta->nombre_plato);
		
		for(int i=0; i<plato->cantidad; i++){
		t_pcb* pcb_new = crearPCB(pedido->id_pedido,time(NULL) + i ,recetta->nombre_plato,pasitos);
		encolarNuevoPlato(pcb_new);
		}
			
		//printf("La receta del plato %s tiene %i pasos.\n",pasos->nombre_plato,pasos->cantidadPasos);
	}

		//	 ENVIO CONFIRMAR PEDIDO A SINDICATO	//

		t_pedido_s* confirmoPedido = malloc(sizeof(t_pedido_s));

		confirmoPedido->id_pedido = pedido->id_pedido;
		confirmoPedido->nombre_restaurante = strdup(config->nombre_restaurante);
		confirmoPedido->nombre_restaurante_size = strlen(confirmoPedido->nombre_restaurante) + 1;
		uint32_t sizeConfirmo = sizeof(uint32_t) * 2 + confirmoPedido->nombre_restaurante_size;

		printf("Mensaje OK. Creando socket de conexión\n");

		t_socket socketSindicatoConfirmando = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

		printf("Conectandose con sindicato...\n");
	
		conectar_socket(socketSindicatoConfirmando);

		t_buffer confirmarlo = serializar_mensaje(confirmoPedido,T_CONFIRMAR_PEDIDO,sizeConfirmo,socketSindicatoConfirmando.socket);
		if(!enviar_mensaje(&confirmarlo)){
		log_error(logger,"La confirmacion del mensaje CONFIRMAR PEDIDO NO SE pudo enviar al SINDICATO.");
		}
	
		//	ENVIO CONFIRMACION DE CONFIRMAR PEDIDO AL MODULO SOLICITANTE	//
	t_resultado_operacion* resultado = malloc(sizeof(t_resultado_operacion));
	if(error == 1)
		resultado->resultado = false;
	else
		resultado->resultado = true;
	
	uint32_t size2 = sizeof(resultado->resultado);
	t_buffer respuesta = serializar_mensaje(resultado,T_RESULTADO_OPERACION,size2 ,buffer->socket);
	if(!enviar_mensaje(&respuesta)){
		log_error(logger,"La confirmacion del mensaje aniadir plato no se pudo enviar al modulo correspondiente.");
		free(resultado);
		free(respuesta.data);
		return;
	}
	free(confirmarlo.data);
	free(confirmoPedido);
	log_info(logger,"SE ENVIO EL RESULTADO %i EN CONFIRMAR PEDIDO.\n",resultado->resultado);
/*
	// Respuesta mockeada
	t_resultado_operacion* operacion = malloc(sizeof(t_resultado_operacion));
	operacion->resultado = 1;
	t_buffer mensaje = serializar_mensaje(operacion,T_RESULTADO_OPERACION,sizeof(uint32_t),buffer->socket);
	printf("\nCODIGO = %d\n", mensaje.msj_type);

	if(!enviar_mensaje(&mensaje)){
		printf("No se pudo enviar el mensaje de confirmación al cliente.");
	}

	free(mensaje.data);
	free(operacion);
*/
}

//Ver si hace falta
void* respuestaConfirmarPedido(t_buffer* buffer){
}


void* consultarPedido(t_buffer* buffer){	//conflicto con confirmar_pedido
	/*printf("Recibo el ID de pedido");
	printf("Consulto a Sindicato mediante Obtener Pedido");
	printf("Devuelvo estado del pedido");*/

	t_id_pedido* ped_id = deserializar_mensaje(buffer);


	t_pedido_s* pedido = malloc(sizeof(t_pedido_s));
	pedido->nombre_restaurante = string_new();
	string_append(&pedido->nombre_restaurante,config->nombre_restaurante);
	pedido->nombre_restaurante_size = string_length(pedido->nombre_restaurante) + 1;
	pedido->id_pedido = ped_id->id_pedido;
	uint32_t size = sizeof(pedido->nombre_restaurante_size) + pedido->nombre_restaurante_size + sizeof(pedido->id_pedido);

	printf("Mensaje OK. Creando socket de conexión\n");

	t_socket socketSindicatoNuevo = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

	printf("Conectandose con sindicato...\n");
	
	conectar_socket(socketSindicatoNuevo);

	t_buffer enviar = serializar_mensaje(pedido,T_OBTENER_PEDIDO,size,socketSindicatoNuevo.socket);

	if(!enviar_mensaje(&enviar)){
		//log_error(logger,"El mensaje obtener pedido no pudo ser enviado.");
		free(pedido);
		free(enviar.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socketSindicatoNuevo.socket);
	if(recibido.msj_type != T_OBTENER_PEDIDO_RESPUESTA){
		//log_error(logger,"El tipo de respuesta de obtener pedido no fue la correcta.");
		free(recibido.data);
		free(pedido);
		free(enviar.data);
		return;
	}
	t_obtener_pedido_s* d_pedido = deserializar_mensaje(&recibido);
	if(d_pedido->cantidadPlatos == 0){
		log_info(logger,"El pedido no tiene ningun plato");
	}

	t_consultar_pedido_s* i_pedido = malloc(sizeof(t_consultar_pedido_s));
	i_pedido->nombre_restaurante = string_new();
	string_append(&i_pedido->nombre_restaurante,config->nombre_restaurante);
	i_pedido->nombre_restaurante_size = string_length(i_pedido->nombre_restaurante) + 1;
	i_pedido->estado = d_pedido->estado;
	i_pedido->cantidadPlatos = d_pedido->cantidadPlatos;
	i_pedido->platos = list_create();
	i_pedido->platos = d_pedido->platos;
	size = i_pedido->nombre_restaurante_size + sizeof(i_pedido->nombre_restaurante_size) 
			+ sizeof(i_pedido->cantidadPlatos) + sizeof(i_pedido->estado); //+ sizeof(i_pedido->platos);	//ver size lista.
	
	void* sumoSize(t_plato_s* plato){
		size = size + sizeof(plato->precio) + sizeof(plato->nombre_size) + sizeof(plato->id) + plato->nombre_size;
		return;
	}
	list_iterate(i_pedido->platos,sumoSize);


	t_buffer enviar_pedido = serializar_mensaje(i_pedido,T_CONSULTAR_PEDIDO_RESPUESTA,size,buffer->socket);
	if(!enviar_mensaje(&enviar_pedido)){
		//log_error(logger,"No se pudieron enviar los datos del pedido consultado.");
		free(d_pedido);
		free(i_pedido);	
		free(enviar_pedido.data);
		return;
	}

	free(enviar_pedido.data);
	free(d_pedido);
	free(i_pedido);

	log_info(logger,"El mensaje consultar pedido fue ejecutado correctamente");
}

//Ver si hace falta
void* respuestaConsultarPedido(t_buffer* buffer){
}

void* platoListo(char* restaurante, uint32_t id_pedido, char* comida){
	
	t_plato_listo_s* plato = malloc(sizeof(t_plato_listo_s));
	plato->nombre_restaurante = string_new();
	string_append(&plato->nombre_restaurante,restaurante);
	plato->nombre_restaurante_size = string_length(plato->nombre_restaurante);
	plato->id_pedido = id_pedido;
	plato->nombre_plato = strdup(comida);
	plato->nombre_plato_size = string_length(plato->nombre_plato);
	uint32_t size = plato->nombre_restaurante_size + sizeof(plato->nombre_restaurante_size)
					+	sizeof(plato->id_pedido)
					+ 	plato->nombre_plato_size + sizeof(plato->nombre_plato_size);

	printf("Mensaje OK. Creando socket de conexión\n");

	t_socket socketSindicatoNuevo = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

	printf("Conectandose con sindicato...\n");
	
	conectar_socket(socketSindicatoNuevo);

	t_buffer plato_enviar = serializar_mensaje(plato,T_PLATO_LISTO,size,socketSindicatoNuevo.socket);

	if(!enviar_mensaje(&plato_enviar)){
		log_error(logger,"No se pudo enviar el mensaje plato listo al modulo sindicato.");
		free(plato);
		free(plato_enviar.data);
		return;
	}

	t_buffer recibidoSindicato = recibir_mensaje(socketSindicatoNuevo.socket);
	if(recibidoSindicato.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"La respuesta del mensaje obtener pedido no pudo ser recibido.");
		free(plato);
		free(plato_enviar.data);
		free(recibidoSindicato.data);
		return;
	}

	t_resultado_operacion* resultadoSindicato = deserializar_mensaje(&recibidoSindicato);

	if(resultadoSindicato->resultado){
		log_info(logger,"El mensaje se recibio correctamente");
	}
	else{
		log_error(logger,"El modulo APP devolvio ERROR al recibir el mensaje");
	}

	free(resultadoSindicato);

	t_buffer plato_enviar_app = serializar_mensaje(plato,T_PLATO_LISTO,size,socketApp.socket);

	if(!enviar_mensaje(&plato_enviar_app)){
		log_error(logger,"No se pudo enviar el mensaje plato listo al modulo sindicato.");
		free(plato);
		free(plato_enviar.data);
		free(plato_enviar_app.data);
		free(recibidoSindicato.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socketApp.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"La respuesta del mensaje obtener pedido no pudo ser recibido.");
		free(plato);
		free(plato_enviar.data);
		free(plato_enviar_app.data);
		free(recibido.data);
		free(recibidoSindicato.data);
		return;
	}

	t_resultado_operacion* resultado = deserializar_mensaje(&recibido);

	if(resultado->resultado){
		log_info(logger,"El mensaje se recibio correctamente");
	}
	else{
		log_error(logger,"El modulo APP devolvio ERROR al recibir el mensaje");
		free(resultado);
		free(plato);
		free(plato_enviar.data);
		free(plato_enviar_app.data);
		free(recibidoSindicato.data);
		return;
	}

	free(resultado);
	free(plato);
	free(plato_enviar.data);
	free(plato_enviar_app.data);
	free(recibidoSindicato.data);

	log_info(logger,"El mensaje plato listo se ejecuto correctamente.");
}

void* terminarPedido(uint32_t id_pedido){
	t_pedido_s* terminarPed = malloc(sizeof(t_pedido_s));
	terminarPed->nombre_restaurante = string_new();
	string_append(&terminarPed->nombre_restaurante,config->nombre_restaurante);
	terminarPed->nombre_restaurante_size = string_length(terminarPed->nombre_restaurante);
	terminarPed->id_pedido = id_pedido;
	uint32_t size = terminarPed->nombre_restaurante_size + sizeof(terminarPed->nombre_restaurante_size)
					+	sizeof(terminarPed->id_pedido);

	printf("Mensaje OK. Creando socket de conexión\n");

	t_socket socketSindicatoNuevo = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

	printf("Conectandose con sindicato...\n");
	
	conectar_socket(socketSindicatoNuevo);

	t_buffer enviar = serializar_mensaje(terminarPed,T_TERMINAR_PEDIDO,size,socketSindicatoNuevo.socket);

	if(!enviar_mensaje(&enviar)){
		log_error(logger,"El mensaje terminar pedido no se pudo enviar al modulo sindicato.");
		free(terminarPed);
		free(enviar.data);
		return;
	}

	t_buffer recibido = recibir_mensaje(socketSindicatoNuevo.socket);
	if(recibido.msj_type != T_RESULTADO_OPERACION){
		log_error(logger,"La respuesta del mensaje obtener pedido no pudo ser recibido.");
		free(terminarPed);
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
	free(terminarPed);
	free(enviar.data);
	free(recibido.data);
}

void* manejarCliente(t_buffer* buffer){
	t_cliente* cliente = deserializar_mensaje(buffer);

	//hago algo con estos datos?

	t_tipo_modulo* modulo = malloc(sizeof(t_tipo_modulo));

	modulo->modulo = T_RESTAURANTE;
	uint32_t size = sizeof(modulo->modulo);
	t_buffer enviar = serializar_mensaje(modulo,T_DATOS_CLIENTE_RESPUESTA,size,buffer->socket);

	if(!enviar_mensaje(&enviar)){
		log_error(logger,"No se pudo enviar el tipo de modulo al cliente.");
	}
	free(modulo);
	free(enviar.data);
}

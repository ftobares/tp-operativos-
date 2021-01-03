#include "server.h"

int runServer(void)
{
    log_info(logger, "Iniciando server de sindicato");
    log_info(logger, "Creando socket de escucha");	

	t_list* threads = list_create();
	
	t_socket socket = crear_socket_de_escucha(config->puerto);

	if (bind_listen_socket_escucha(socket)) 
	{
		int accepting = 1;
		
		printf("Socket abierto correctamente. Esperando mensajes entrantes...\n");
		
		while (accepting) 
		{
			t_socket socketCliente = aceptando_conexiones(socket);
			
			if (socketCliente.socket != -1) 
			{
				log_info(logger, "Mensaje recibido. Socket: %d", socketCliente.socket);

				pthread_t thread;
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

				t_socket* pSocket = malloc(sizeof(t_socket));
				*pSocket = socketCliente;
	
				log_info(logger, "Enviando mensaje a hilo de procesamiento");

				pthread_create(&thread, &attr, procesarMensaje, pSocket);

				log_info(logger, "Mensaje enviado a hilo de procesamiento correctamente");

				list_add(threads, thread);
			} 
			else 
			{
				accepting = 0;
			}
		}
	}
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}

	log_info(logger, "Fin Server");
}

void procesarMensaje(t_socket* socketCliente) 
{
	log_info(logger, "Procesando mensaje");

	t_buffer buffer = recibir_mensaje(socketCliente->socket);

	log_info(logger, "Buffer obtenido correctamente.");
	log_info(logger, "Message type: %d", buffer.msj_type);

	void* buffer_to_free = buffer.data;

	switch(buffer.msj_type)
	{
		case SOCKET_CLOSED:
			//Habria que hacer algo??
			break;
		
		case T_DATOS_CLIENTE:
			responder_handshake(&buffer);
			break;

		case T_GUARDAR_PLATO:
			guardarPlatoRestaurante(&buffer);
			break;

		case T_CONSULTAR_PLATOS:
			consultarPlatos(&buffer);
			break;

		case T_GUARDAR_PEDIDO:
			guardarPedido(&buffer);
			break;

		case T_CONFIRMAR_PEDIDO:
			confirmarPedido(&buffer);
			break;

		case T_OBTENER_PEDIDO:
			obtenerPedido(&buffer);
			break;

		case T_OBTENER_RESTAURANTE:
			obtenerRestaurante(&buffer);
			break;

		case T_PLATO_LISTO:
			platoListo(&buffer);
			break;

		case T_OBTENER_RECETA:
			obtenerReceta(&buffer);
			break;

		case T_TERMINAR_PEDIDO:
			terminarPedido(&buffer);
			break;

		default: perror("Tipo de mensaje invalido");
	}

	log_info(logger, "Mensaje procesado OK, liberando buffer");

	free(buffer_to_free);
	log_info(logger, "Mensaje procesado OK, liberado buffer OK");

}

void responder_handshake(t_buffer* buffer)
{
	log_info(logger, "Respondiendo handshake");
	
	t_tipo_modulo* respuesta = malloc(sizeof(t_tipo_modulo));
	respuesta->modulo = T_SINDICATO; 

	t_buffer mensaje = serializar_mensaje(respuesta,T_DATOS_CLIENTE_RESPUESTA,sizeof(uint32_t),buffer->socket);
	
	if(!enviar_mensaje(&mensaje)) log_error(logger, "No se pudo responder el handshake correctamente.");
	
	else log_debug(logger, "Handshake respondido OK.");

	free(respuesta);
	free(mensaje.data);
}

void enviar_respuesta_resultado_operacion(t_buffer* buffer, uint32_t _resultado)
{
	log_info(logger, "Enviando respuesta resultado operación %d", _resultado);
	
	t_resultado_operacion* resultado = malloc(sizeof(t_resultado_operacion));

	resultado->resultado = _resultado;

	t_buffer mensaje = serializar_mensaje(resultado, T_RESULTADO_OPERACION, sizeof(uint32_t),buffer->socket);
	
	if(!enviar_mensaje(&mensaje)) log_error(logger, "No se pudo enviar respuesta de resultado.");
	
	else log_info(logger, "Respuesta de resultado enviada correctamente.");

	free(resultado);
	free(mensaje.data);
}
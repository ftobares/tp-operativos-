#include "serializer.h"

t_buffer crear_buffer(uint32_t msj_type, uint32_t socket, int32_t size,void* data) {
	t_buffer tmpBuffer;
	tmpBuffer.msj_type = msj_type;
	tmpBuffer.socket = socket;
	tmpBuffer.size = size;
	tmpBuffer.data = data;
	return tmpBuffer;
}

t_buffer crear_buffer_sin_cuerpo(uint32_t msj_type, uint32_t socket){
	t_buffer buffer;
	buffer.msj_type = msj_type;
	buffer.socket = socket;
	buffer.size = 0;
	buffer.data = NULL;
	return buffer;
}

/**
 * @NAME: serializar mensaje
 * @DESC: Recibe un paquete (mensaje), un tipo de mensaje, un tamanio y el socket del servidor
 * 		  Retorna un buffer para ser enviado.
 */
t_buffer serializar_mensaje(void* paqueteSinSerializar,
		t_tipo_mensaje tipoMensaje, uint32_t size, uint32_t un_socket) {

	void* paqueteSerializado = malloc(size);

	int offset = 0;
	int size_to_send;

	switch (tipoMensaje) {
	case T_LISTADO_RESTAURANTES:;
		t_listado_restaurantes* restaurantes = (t_listado_restaurantes*) paqueteSinSerializar;
		size_to_send = sizeof(restaurantes->listado_size);
		memcpy(paqueteSerializado + offset, &(restaurantes->listado_size), size_to_send);
		offset += size_to_send;
		size_to_send = restaurantes->listado_size;
		memcpy(paqueteSerializado + offset, restaurantes->listado, strlen(restaurantes->listado) + 1);
		break;

	case T_SELECCIONAR_RESTAURANTE:{
		t_seleccionar_restaurante* paquete = (t_seleccionar_restaurante*) paqueteSinSerializar;

		// Id Cliente Size
		size_to_send = sizeof(paquete->id_cliente_size);
		memcpy(paqueteSerializado + offset, &(paquete->id_cliente_size), size_to_send);
		offset += size_to_send;

		// Id Cliente
		size_to_send = paquete->id_cliente_size;
		memcpy(paqueteSerializado + offset, paquete->id_cliente, paquete->id_cliente_size);
		offset += size_to_send;

		// Id Restaurante Size
		size_to_send = sizeof(paquete->id_restaurante_size);
		memcpy(paqueteSerializado + offset, &(paquete->id_restaurante_size), size_to_send);
		offset += size_to_send;

		// Id Restaurante
		size_to_send = paquete->id_restaurante_size;
		memcpy(paqueteSerializado + offset, paquete->id_restaurante, paquete->id_restaurante_size);
		break;
	}
	case T_OBTENER_RESTAURANTE:{
		t_nombre_restaurante_s* paqueteObtenerRestaurante = (t_nombre_restaurante_s*) paqueteSinSerializar;
		serializar_nombre_restaurante(paqueteObtenerRestaurante, paqueteSerializado);
		break;
	}
	case T_OBTENER_RESTAURANTE_RESPUESTA:;
		t_obtener_restaurante_respuesta_s* paqueteObtenerRestauranteRespuesta = (t_obtener_restaurante_respuesta_s*) paqueteSinSerializar;
		serializar_datos_restaurante(paqueteObtenerRestauranteRespuesta, paqueteSerializado);
		break;

	case T_CONSULTAR_PLATOS:{
		t_nombre_restaurante_s* paqueteConsultarPlatos = (t_nombre_restaurante_s*) paqueteSinSerializar;
		serializar_nombre_restaurante(paqueteConsultarPlatos, paqueteSerializado);
		break;
	}
      
	case T_CONSULTAR_PLATOS_RESPUESTA:;
		t_single_text_s* paqueteRespuesta = (t_single_text_s*) paqueteSinSerializar;
		serializar_single_text(paqueteRespuesta, paqueteSerializado);
		break;

	case T_CREAR_PEDIDO_RESPUESTA:{
		t_id_pedido* pedido = (t_id_pedido*) paqueteSinSerializar;
		size_to_send = sizeof(pedido->id_pedido);
		memcpy(paqueteSerializado + offset, &(pedido->id_pedido), size_to_send);
		break;
	}
	case T_GUARDAR_PEDIDO:;
		t_pedido_s* paqueteGuardarPedido = (t_pedido_s*) paqueteSinSerializar;
		serializar_pedido(paqueteGuardarPedido, paqueteSerializado);
		break;

	case T_ANIADIR_PLATO:{
		t_aniadir_plato* paquete = (t_aniadir_plato*) paqueteSinSerializar;

		// Plato Size
		size_to_send = sizeof(paquete->plato_size);
		memcpy(paqueteSerializado + offset, &(paquete->plato_size), size_to_send);
		offset += size_to_send;

		// Plato
		size_to_send = paquete->plato_size;
		memcpy(paqueteSerializado + offset, paquete->plato, paquete->plato_size);
		offset += size_to_send;

		// Id Pedido
		size_to_send = sizeof(paquete->id_pedido);
		memcpy(paqueteSerializado + offset, &(paquete->id_pedido), size_to_send);

		break;
	}
	case T_PING:{
		t_ping_s* paquetePing = (t_ping_s*) paqueteSinSerializar;
		size_to_send = sizeof(paquetePing->result);
		memcpy(paqueteSerializado, &(paquetePing->result), size_to_send);
		break;
    }
	
	case T_RESULTADO_OPERACION:{
		t_resultado_operacion* operacion = (t_resultado_operacion*) paqueteSinSerializar;
		size_to_send = sizeof(operacion->resultado);
		memcpy(paqueteSerializado,&(operacion->resultado), size_to_send);
		break;
	}

	case T_DATOS_CLIENTE_RESPUESTA:{
		t_tipo_modulo* modulo = (t_tipo_modulo*) paqueteSinSerializar;
		size_to_send = sizeof(modulo->modulo);
		memcpy(paqueteSerializado,&(modulo->modulo), size_to_send);
		break;
	}

	case T_DATOS_RESTAURANTE:{
		t_restaurante_handshake* paquete_res = (t_cliente*) paqueteSinSerializar;

		// Id Restaurante Size
		size_to_send = sizeof(paquete_res->nombre_restaurante_size);
		memcpy(paqueteSerializado + offset, &(paquete_res->nombre_restaurante_size), size_to_send);
		offset += size_to_send;

		// Id Restaurante
		size_to_send = paquete_res->nombre_restaurante_size;
		memcpy(paqueteSerializado + offset, paquete_res->nombre_restaurante, size_to_send);
		offset += size_to_send;

		// Posicion X
		size_to_send = sizeof(paquete_res->posicion_x);
		memcpy(paqueteSerializado + offset, &(paquete_res->posicion_x), size_to_send);
		offset += size_to_send;

		// Posicion Y
		size_to_send = sizeof(paquete_res->posicion_y);
		memcpy(paqueteSerializado + offset, &(paquete_res->posicion_y), size_to_send);
		offset += size_to_send;

		// IP Size
		size_to_send = sizeof(paquete_res->ip_size);
		memcpy(paqueteSerializado + offset, &(paquete_res->ip_size), size_to_send);
		offset += size_to_send;

		// IP
		size_to_send = paquete_res->ip_size;
		memcpy(paqueteSerializado + offset, paquete_res->ip, size_to_send);
		offset += size_to_send;

		// PORT Size
		size_to_send = sizeof(paquete_res->puerto_size);
		memcpy(paqueteSerializado + offset, &(paquete_res->puerto_size), size_to_send);
		offset += size_to_send;

		// PORT
		size_to_send = paquete_res->puerto_size;
		memcpy(paqueteSerializado + offset, paquete_res->puerto, size_to_send);
		offset += size_to_send;

		break;
	}

	case T_GUARDAR_PLATO:;
		t_guardar_plato_s* paqueteGuardarPlato = (t_guardar_plato_s*) paqueteSinSerializar;

		size_to_send = sizeof(paqueteGuardarPlato->nombre_restaurante_size);
		memcpy(paqueteSerializado + offset, &(paqueteGuardarPlato->nombre_restaurante_size),
				size_to_send);
		offset += size_to_send;

		size_to_send = paqueteGuardarPlato->nombre_restaurante_size;
		memcpy(paqueteSerializado + offset, paqueteGuardarPlato->nombre_restaurante,
				strlen(paqueteGuardarPlato->nombre_restaurante) + 1);
		offset += size_to_send;

		size_to_send = sizeof(paqueteGuardarPlato->id_pedido);
		memcpy(paqueteSerializado + offset, &(paqueteGuardarPlato->id_pedido),
				size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(paqueteGuardarPlato->nombre_plato_size);
		memcpy(paqueteSerializado + offset, &(paqueteGuardarPlato->nombre_plato_size),
				size_to_send);
		offset += size_to_send;

		size_to_send = paqueteGuardarPlato->nombre_plato_size;
		memcpy(paqueteSerializado + offset, paqueteGuardarPlato->nombre_plato,
				strlen(paqueteGuardarPlato->nombre_plato) + 1);
		offset += size_to_send;

		size_to_send = sizeof(paqueteGuardarPlato->cantidad);
		memcpy(paqueteSerializado + offset, &(paqueteGuardarPlato->cantidad), size_to_send);

		break;

	case T_CONFIRMAR_PEDIDO:;
		t_pedido_s* paqueteConfirmarPedido = (t_pedido_s*) paqueteSinSerializar;
		serializar_pedido(paqueteConfirmarPedido, paqueteSerializado);
		break;

	case T_CONFIRMAR_PEDIDO_SOLO_ID:{
		t_id_pedido* pedido = (t_id_pedido*) paqueteSinSerializar;
		size_to_send = sizeof(pedido->id_pedido);
		memcpy(paqueteSerializado + offset, &(pedido->id_pedido), size_to_send);
		break;
	}
	case T_PLATO_LISTO:;
		
		t_plato_listo_s* paquetePlatoListo = (t_plato_listo_s*) paqueteSinSerializar;
		
		size_to_send = sizeof(paquetePlatoListo->nombre_restaurante_size);
		memcpy(paqueteSerializado + offset, &(paquetePlatoListo->nombre_restaurante_size),
				size_to_send);
		offset += size_to_send;

		printf("PlatoListo NRS %d\n", paquetePlatoListo->nombre_restaurante_size);

		size_to_send = paquetePlatoListo->nombre_restaurante_size;
		memcpy(paqueteSerializado + offset, paquetePlatoListo->nombre_restaurante,
				paquetePlatoListo->nombre_restaurante_size);
		offset += size_to_send;

		printf("PlatoListo NR %s\n", paquetePlatoListo->nombre_restaurante);

		size_to_send = sizeof(paquetePlatoListo->id_pedido);
		memcpy(paqueteSerializado + offset, &(paquetePlatoListo->id_pedido),
				size_to_send);
		offset += size_to_send;

		printf("PlatoListo ID PEDIDO %d\n", paquetePlatoListo->id_pedido);

		size_to_send = sizeof(paquetePlatoListo->nombre_plato_size);
		memcpy(paqueteSerializado + offset, &(paquetePlatoListo->nombre_plato_size),
				size_to_send);
		offset += size_to_send;

		printf("PlatoListo NPS %d\n", paquetePlatoListo->nombre_plato_size);

		size_to_send = paquetePlatoListo->nombre_plato_size;
		memcpy(paqueteSerializado + offset, paquetePlatoListo->nombre_plato,
				paquetePlatoListo->nombre_plato_size);
		
		printf("PlatoListo NP %s\n", paquetePlatoListo->nombre_plato);

		break;

	case T_CONSULTAR_PEDIDO:{
		t_id_pedido* pedido = (t_id_pedido*) paqueteSinSerializar;
		size_to_send = sizeof(pedido->id_pedido);
		memcpy(paqueteSerializado + offset, &(pedido->id_pedido), size_to_send);
		break;
	}
	case T_CONSULTAR_PEDIDO_RESPUESTA:{
		t_consultar_pedido_s* pedido = (t_consultar_pedido_s*) paqueteSinSerializar;

		size_to_send = sizeof(pedido->nombre_restaurante_size);
		memcpy(paqueteSerializado + offset, &(pedido->nombre_restaurante_size),size_to_send);
		offset += size_to_send;

		size_to_send = pedido->nombre_restaurante_size;
		memcpy(paqueteSerializado + offset, pedido->nombre_restaurante, pedido->nombre_restaurante_size);
		offset += size_to_send;

		size_to_send = sizeof(pedido->estado);
		memcpy(paqueteSerializado + offset, &(pedido->estado), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(pedido->cantidadPlatos);
		memcpy(paqueteSerializado + offset, &(pedido->cantidadPlatos), size_to_send);
		offset += size_to_send;

		// serializo la lista de platos
		for(int i = 0; i<pedido->cantidadPlatos; i++)
		{
			t_obtener_pedido_plato_s* plato = list_get(pedido->platos, i);

			size_to_send = sizeof(plato->cantidad);
			memcpy(paqueteSerializado + offset, &(plato->cantidad), size_to_send);
			offset += size_to_send;

			size_to_send = sizeof(plato->cantidadLista);
			memcpy(paqueteSerializado + offset, &(plato->cantidadLista), size_to_send);
			offset += size_to_send;

			size_to_send = sizeof(plato->comida);
			memcpy(paqueteSerializado + offset, &(plato->comida), size_to_send);
			offset += size_to_send;
		}
		break;

	}
	case T_OBTENER_PEDIDO:;
		t_pedido_s* paqueteObtenerPedido = (t_pedido_s*) paqueteSinSerializar;
		serializar_pedido(paqueteObtenerPedido, paqueteSerializado);
		break;

	case T_FINALIZAR_PEDIDO:;
		t_pedido_s* paqueteFinalizarPedido = (t_pedido_s*) paqueteSinSerializar;
		serializar_pedido(paqueteFinalizarPedido, paqueteSerializado);
		break;

	case T_TERMINAR_PEDIDO:;
		t_pedido_s* paqueteTerminarPedido = (t_pedido_s*) paqueteSinSerializar;
		serializar_pedido(paqueteTerminarPedido, paqueteSerializado);
		break;

	case T_OBTENER_RECETA:;

		/* Se utiliza para poder obtener la receta de un plato - Ejemplo: Restaurante a Sindicato */

		t_obtener_receta_s* plato = (t_obtener_receta_s*) paqueteSinSerializar;

		size_to_send = sizeof(plato->nombre_plato_size);
		memcpy(paqueteSerializado + offset, &(plato->nombre_plato_size), size_to_send);
		offset += size_to_send;

		size_to_send = plato->nombre_plato_size;
		memcpy(paqueteSerializado + offset, plato->nombre_plato, strlen(plato->nombre_plato) + 1);

		break;

	case T_OBTENER_RECETA_RESPUESTA:;

		t_obtener_receta_respuesta* receta = (t_obtener_receta_respuesta*) paqueteSinSerializar;

		size_to_send = sizeof(receta->nombre_plato_size);
		memcpy(paqueteSerializado + offset, &(receta->nombre_plato_size),size_to_send);
		offset += size_to_send;

		size_to_send = receta->nombre_plato_size;
		memcpy(paqueteSerializado + offset, receta->nombre_plato, receta->nombre_plato_size);
		offset += size_to_send;

		size_to_send = sizeof(receta->cantidadPasos);
		memcpy(paqueteSerializado + offset, &(receta->cantidadPasos), size_to_send);
		offset += size_to_send;

		// serializo la lista de platos
		for(int i = 0; i<receta->cantidadPasos; i++)
		{
			t_receta_s* paso = list_get(receta->pasos, i);

			size_to_send = sizeof(paso->paso_size);
			memcpy(paqueteSerializado + offset, &(paso->paso_size), size_to_send);
			offset += size_to_send;
	
			size_to_send = paso->paso_size;
			memcpy(paqueteSerializado + offset, paso->paso, size_to_send);
			offset += size_to_send;

			size_to_send = sizeof(paso->tiempo);
			memcpy(paqueteSerializado + offset, &(paso->tiempo), size_to_send);
			offset += size_to_send;
		}

		break;

	case T_DATOS_CLIENTE:{

		t_cliente* paquete_cli = (t_cliente*) paqueteSinSerializar;

		// Id Cliente Size
		size_to_send = sizeof(paquete_cli->id_cliente_size);
		memcpy(paqueteSerializado + offset, &(paquete_cli->id_cliente_size), size_to_send);
		offset += size_to_send;

		// Id Cliente
		size_to_send = paquete_cli->id_cliente_size;
		memcpy(paqueteSerializado + offset, paquete_cli->id_cliente, size_to_send);
		offset += size_to_send;

		// Posicion X
		size_to_send = sizeof(paquete_cli->posicion_x);
		memcpy(paqueteSerializado + offset, &(paquete_cli->posicion_x), size_to_send);
		offset += size_to_send;

		// Posicion Y
		size_to_send = sizeof(paquete_cli->posicion_y);
		memcpy(paqueteSerializado + offset, &(paquete_cli->posicion_y), size_to_send);
		offset += size_to_send;

		// Puerto Escucha Size
		size_to_send = sizeof(paquete_cli->puerto_escucha_size);
		memcpy(paqueteSerializado + offset, &(paquete_cli->puerto_escucha_size), size_to_send);
		offset += size_to_send;

		// Puerto Escucha
		size_to_send = paquete_cli->puerto_escucha_size;
		memcpy(paqueteSerializado + offset, paquete_cli->puerto_escucha, size_to_send);
		offset += size_to_send;

		// IP Escucha Size
		size_to_send = sizeof(paquete_cli->ip_escucha_size);
		memcpy(paqueteSerializado + offset, &(paquete_cli->ip_escucha_size), size_to_send);
		offset += size_to_send;

		// IP Escucha
		size_to_send = paquete_cli->ip_escucha_size;
		memcpy(paqueteSerializado + offset, paquete_cli->ip_escucha, size_to_send);
		offset += size_to_send;

		break;
	}
	case T_OBTENER_PEDIDO_RESPUESTA: {

		t_obtener_pedido_s* paqueteObtenerPedidoRta = (t_obtener_pedido_s*) paqueteSinSerializar;

		size_to_send = sizeof(paqueteObtenerPedidoRta->estado);
		memcpy(paqueteSerializado + offset, &(paqueteObtenerPedidoRta->estado), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(paqueteObtenerPedidoRta->cantidadPlatos);
		memcpy(paqueteSerializado + offset, &(paqueteObtenerPedidoRta->cantidadPlatos), size_to_send);
		offset += size_to_send;

		// serializo la lista de platos
		for(int i = 0; i<paqueteObtenerPedidoRta->cantidadPlatos; i++)
		{
			t_obtener_pedido_plato_s* plato = list_get(paqueteObtenerPedidoRta->platos, i);			

			size_to_send = sizeof(plato->cantidad);
			memcpy(paqueteSerializado + offset, &(plato->cantidad), size_to_send);
			offset += size_to_send;

			size_to_send = sizeof(plato->cantidadLista);
			memcpy(paqueteSerializado + offset, &(plato->cantidadLista), size_to_send);
			offset += size_to_send;

			size_to_send = sizeof(plato->comida);
			memcpy(paqueteSerializado + offset, &(plato->comida), size_to_send);
			offset += size_to_send;
		}
		break;
	}
	default:
		perror("ERROR cargando configuracion tipo de archivo invalido");
	}

	// Temporalmente dejamos el envío sin validación
	t_buffer buffer = crear_buffer(tipoMensaje, un_socket, size, paqueteSerializado);
	return buffer;

/*	t_buffer buffer;
	if (strlen(paqueteSerializado) > 0) {
		buffer = crear_buffer(tipoMensaje, un_socket, size, paqueteSerializado);
		return buffer;
	} else {
		perror("Mensaje no pudo ser serializado");
		buffer.msj_type = -1;
		return buffer;
	}*/

}

/**
 * @NAME: deserializar mensaje
 * @DESC: Recibe un puntero a un buffer.
 * 		  Devuelve un puntero a un paquete.
 * 		  El paquete debe ser casteado al paquete esperado
 * 		  y realizar las validaciones que sean necesarias
 */
void* deserializar_mensaje(t_buffer* buffer) {

	int buffer_size;
	int offset = 0;

	switch (buffer->msj_type) 
	{
		case T_LISTADO_RESTAURANTES:{

			t_listado_restaurantes* restaurantes = malloc(sizeof(t_listado_restaurantes));
			memcpy(&(restaurantes->listado_size), buffer->data, sizeof(restaurantes->listado_size));
			offset += sizeof(restaurantes->listado_size);

			restaurantes->listado = malloc(restaurantes->listado_size);
			memcpy(restaurantes->listado, buffer->data + offset, restaurantes->listado_size);

			return restaurantes;
			break;
		}
		case T_SELECCIONAR_RESTAURANTE:{
			t_seleccionar_restaurante* paquete = malloc(sizeof(t_seleccionar_restaurante));

			memcpy(&(paquete->id_cliente_size), buffer->data, sizeof(paquete->id_cliente_size));
			offset += sizeof(paquete->id_cliente_size);

			paquete->id_cliente = malloc(paquete->id_cliente_size);
			memcpy(paquete->id_cliente, buffer->data + offset, paquete->id_cliente_size);
			offset += paquete->id_cliente_size;

			memcpy(&(paquete->id_restaurante_size), buffer->data + offset, sizeof(paquete->id_restaurante_size));
			offset += sizeof(paquete->id_restaurante_size);

			paquete->id_restaurante = malloc(paquete->id_restaurante_size);
			memcpy(paquete->id_restaurante, buffer->data + offset, paquete->id_restaurante_size);

			return paquete;
			break;
		}
		case T_OBTENER_RESTAURANTE:
			return deserializar_nombre_restaurante(buffer);
			break;

		case T_OBTENER_RESTAURANTE_RESPUESTA:
			return deserializar_datos_restaurante(buffer);
			break;

		case T_CONSULTAR_PLATOS:;
			return deserializar_nombre_restaurante(buffer);
			break;

		case T_CONSULTAR_PLATOS_RESPUESTA:;
			return deserializar_single_text(buffer);
			break;
		
		case T_CREAR_PEDIDO_RESPUESTA:{
			t_id_pedido* pedido = malloc(sizeof(t_id_pedido));
			memcpy(&(pedido->id_pedido), buffer->data, sizeof(pedido->id_pedido));
			return pedido;
			break;
		}
		case T_GUARDAR_PEDIDO:;
			return deserializar_pedido(buffer);
			break;

		case T_ANIADIR_PLATO:{
			t_aniadir_plato* paquete = malloc(sizeof(t_aniadir_plato));

			memcpy(&(paquete->plato_size), buffer->data, sizeof(paquete->plato_size));
			offset += sizeof(paquete->plato_size);

			paquete->plato = malloc(paquete->plato_size);
			memcpy(paquete->plato, buffer->data + offset, paquete->plato_size);
			offset += paquete->plato_size;

			memcpy(&(paquete->id_pedido), buffer->data + offset, sizeof(paquete->id_pedido));
			return paquete;
			break;
		}

		case T_PING:{
			t_ping_s* paquetePing = malloc(sizeof(t_ping_s));
			memcpy(&(paquetePing->result), buffer->data, sizeof(paquetePing->result));
			return paquetePing;
			break;
		}
		case T_RESULTADO_OPERACION:{
			t_resultado_operacion* operacion = malloc(sizeof(t_resultado_operacion));
			memcpy(&(operacion->resultado), buffer->data, sizeof(operacion->resultado));
			return operacion;
			break;
		}
		case T_DATOS_CLIENTE_RESPUESTA:{
			t_tipo_modulo* modulo = malloc(sizeof(t_tipo_modulo));
			memcpy(&(modulo->modulo), buffer->data, sizeof(modulo->modulo));
			return modulo;
			break;
		}
		case T_DATOS_RESTAURANTE:{
			t_restaurante_handshake* paquete_res = malloc(sizeof(t_restaurante_handshake));

			memcpy(&(paquete_res->nombre_restaurante_size), buffer->data, sizeof(paquete_res->nombre_restaurante_size));
			offset += sizeof(paquete_res->nombre_restaurante_size);

			paquete_res->nombre_restaurante = malloc(paquete_res->nombre_restaurante_size);
			memcpy(paquete_res->nombre_restaurante, buffer->data + offset, paquete_res->nombre_restaurante_size);
			offset += paquete_res->nombre_restaurante_size;

			memcpy(&(paquete_res->posicion_x), buffer->data + offset, sizeof(paquete_res->posicion_x));
			offset += sizeof(paquete_res->posicion_x);

			memcpy(&(paquete_res->posicion_y), buffer->data + offset, sizeof(paquete_res->posicion_y));
			offset += sizeof(paquete_res->posicion_y);

			memcpy(&(paquete_res->ip_size), buffer->data + offset, sizeof(paquete_res->ip_size));
			offset += sizeof(paquete_res->ip_size);

			paquete_res->ip = malloc(paquete_res->ip_size);
			memcpy(paquete_res->ip, buffer->data + offset, paquete_res->ip_size);
			offset += paquete_res->ip_size;

			memcpy(&(paquete_res->puerto_size), buffer->data + offset, sizeof(paquete_res->puerto_size));
			offset += sizeof(paquete_res->puerto_size);

			paquete_res->puerto = malloc(paquete_res->puerto_size);
			memcpy(paquete_res->puerto, buffer->data + offset, paquete_res->puerto_size);

			return paquete_res;
		}
		case T_GUARDAR_PLATO:{
			return deserializar_t_guardar_plato(buffer);
			break;
		}
		case T_CONFIRMAR_PEDIDO:;
			return deserializar_pedido(buffer);
			break;

		case T_CONFIRMAR_PEDIDO_SOLO_ID:{
			t_id_pedido* pedido = malloc(sizeof(t_id_pedido));
			memcpy(&(pedido->id_pedido), buffer->data, sizeof(pedido->id_pedido));
			return pedido;
			break;
		}

		case T_PLATO_LISTO:;
			t_plato_listo_s* paquetePlatoListo = malloc(sizeof(t_plato_listo_s));

			paquetePlatoListo->nombre_restaurante_size = deserializar_int(buffer);
			paquetePlatoListo->nombre_restaurante = deserializar_string(buffer, paquetePlatoListo->nombre_restaurante_size);
			paquetePlatoListo->id_pedido = deserializar_int(buffer);
			paquetePlatoListo->nombre_plato_size =  deserializar_int(buffer);
			paquetePlatoListo->nombre_plato =  deserializar_string(buffer, paquetePlatoListo->nombre_plato_size);

			return paquetePlatoListo;
			break;

		case T_CONSULTAR_PEDIDO:{
			t_id_pedido* pedido = malloc(sizeof(t_id_pedido));
			memcpy(&(pedido->id_pedido), buffer->data, sizeof(pedido->id_pedido));
			return pedido;
			break;
		}
		case T_CONSULTAR_PEDIDO_RESPUESTA:{
			t_consultar_pedido_s* paqueteConsultarPedido = malloc(sizeof(t_consultar_pedido_s));

			paqueteConsultarPedido->platos = list_create();

			memcpy(&(paqueteConsultarPedido->nombre_restaurante_size), buffer->data + offset,sizeof(paqueteConsultarPedido->nombre_restaurante_size));
			offset += sizeof(paqueteConsultarPedido->nombre_restaurante_size);

			paqueteConsultarPedido->nombre_restaurante = malloc(paqueteConsultarPedido->nombre_restaurante_size);
			memcpy(paqueteConsultarPedido->nombre_restaurante, buffer->data + offset, paqueteConsultarPedido->nombre_restaurante_size);
			offset += paqueteConsultarPedido->nombre_restaurante_size;

			memcpy(&(paqueteConsultarPedido->estado), buffer->data + offset, sizeof(paqueteConsultarPedido->estado));
			offset += sizeof(paqueteConsultarPedido->estado);

			memcpy(&(paqueteConsultarPedido->cantidadPlatos), buffer->data + offset, sizeof(paqueteConsultarPedido->cantidadPlatos));
			offset += sizeof(paqueteConsultarPedido->cantidadPlatos);

			for(int i = 0; i<paqueteConsultarPedido->cantidadPlatos;i++)
			{
				t_obtener_pedido_plato_s* plato = malloc(sizeof(t_obtener_pedido_plato_s));

				memcpy(&(plato->cantidad), buffer->data + offset, sizeof(plato->cantidad));
				offset += sizeof(plato->cantidad);

				memcpy(&(plato->cantidadLista), buffer->data + offset, sizeof(plato->cantidadLista));
				offset += sizeof(plato->cantidadLista);

				memcpy(&(plato->comida), buffer->data + offset, sizeof(plato->comida));
				offset += sizeof(plato->comida);

				list_add(paqueteConsultarPedido->platos, plato);
			}
			return paqueteConsultarPedido;

		}
		case T_OBTENER_PEDIDO:;
			return deserializar_pedido(buffer);
			break;

		case T_FINALIZAR_PEDIDO:;
			return deserializar_pedido(buffer);
			break;

		case T_TERMINAR_PEDIDO:
			return deserializar_pedido(buffer);
			break;

		case T_OBTENER_RECETA:;
		
			t_obtener_receta_s* recetaAObtener = malloc(sizeof(t_obtener_receta_s));

			recetaAObtener->nombre_plato_size = deserializar_int(buffer);
			recetaAObtener->nombre_plato = deserializar_string(buffer, recetaAObtener->nombre_plato_size);

			return recetaAObtener;

		case T_OBTENER_RECETA_RESPUESTA:;
			
			t_obtener_receta_respuesta* paqueteObtenerRecetaRta = malloc(sizeof(t_obtener_receta_respuesta));

			paqueteObtenerRecetaRta->nombre_plato_size = deserializar_int(buffer);
			paqueteObtenerRecetaRta->nombre_plato = deserializar_string(buffer, paqueteObtenerRecetaRta->nombre_plato_size);
			paqueteObtenerRecetaRta->cantidadPasos = deserializar_int(buffer);
			paqueteObtenerRecetaRta->pasos = list_create();			

			for(int i = 0; i < paqueteObtenerRecetaRta->cantidadPasos; i++)
			{
				t_paso_s* paso = malloc(sizeof(t_paso_s));

				paso->nombre_paso_size = deserializar_int(buffer);
				paso->nombre_paso = deserializar_string(buffer, paso->nombre_paso_size);
				paso->tiempo = deserializar_int(buffer);

				list_add(paqueteObtenerRecetaRta->pasos, paso);
			}
			
			return paqueteObtenerRecetaRta;
			break;

		case T_DATOS_CLIENTE:{
			printf("Deserializar DatosCliente\n");

			t_cliente* paquete_cli = malloc(sizeof(t_cliente));

			memcpy(&(paquete_cli->id_cliente_size), buffer->data, sizeof(paquete_cli->id_cliente_size));
			offset += sizeof(paquete_cli->id_cliente_size);

			paquete_cli->id_cliente = malloc(paquete_cli->id_cliente_size);
			memcpy(paquete_cli->id_cliente, buffer->data + offset, paquete_cli->id_cliente_size);
			offset += paquete_cli->id_cliente_size;

			memcpy(&(paquete_cli->posicion_x), buffer->data + offset, sizeof(paquete_cli->posicion_x));
			offset += sizeof(paquete_cli->posicion_x);

			memcpy(&(paquete_cli->posicion_y), buffer->data + offset, sizeof(paquete_cli->posicion_y));
			offset += sizeof(paquete_cli->posicion_y);

			memcpy(&(paquete_cli->puerto_escucha_size), buffer->data + offset, sizeof(paquete_cli->puerto_escucha_size));
			offset += sizeof(paquete_cli->puerto_escucha_size);

			paquete_cli->puerto_escucha = malloc(paquete_cli->puerto_escucha_size);
			memcpy(paquete_cli->puerto_escucha, buffer->data + offset, paquete_cli->puerto_escucha_size);
			offset += paquete_cli->puerto_escucha_size;

			memcpy(&(paquete_cli->ip_escucha_size), buffer->data + offset, sizeof(paquete_cli->ip_escucha_size));
			offset += sizeof(paquete_cli->ip_escucha_size);

			paquete_cli->ip_escucha = malloc(paquete_cli->ip_escucha_size);
			memcpy(paquete_cli->ip_escucha, buffer->data + offset, paquete_cli->ip_escucha_size);

			return paquete_cli;
		}
		case T_OBTENER_PEDIDO_RESPUESTA: {
			t_obtener_pedido_s* paqueteObtenerPedido = malloc(sizeof(t_obtener_pedido_s));

			

			memcpy(&(paqueteObtenerPedido->estado), buffer->data + offset, sizeof(paqueteObtenerPedido->estado));
			offset += sizeof(paqueteObtenerPedido->estado);

			memcpy(&(paqueteObtenerPedido->cantidadPlatos), buffer->data + offset, sizeof(paqueteObtenerPedido->cantidadPlatos));
			offset += sizeof(paqueteObtenerPedido->cantidadPlatos);

			if (paqueteObtenerPedido->cantidadPlatos != 0) {
				paqueteObtenerPedido->platos = list_create();
				for(int i = 0; i<paqueteObtenerPedido->cantidadPlatos;i++)
				{
					t_obtener_pedido_plato_s* plato = malloc(sizeof(t_obtener_pedido_plato_s));

					memcpy(&(plato->cantidad), buffer->data + offset, sizeof(plato->cantidad));
					offset += sizeof(plato->cantidad);

					memcpy(&(plato->cantidadLista), buffer->data + offset, sizeof(plato->cantidadLista));
					offset += sizeof(plato->cantidadLista);

					memcpy(&(plato->comida), buffer->data + offset, sizeof(plato->comida));
					offset += sizeof(plato->comida);

					list_add(paqueteObtenerPedido->platos, plato);
				}
			}
		
			return paqueteObtenerPedido;
		}
		default:
			perror("ERROR cargando configuracion tipo de archivo invalido");
	}

	printf("ATENCION no se deserializó el mensaje y devolvio NULL");
	return NULL; //FIXME
}

/**
 * @NAME: deserializar un mensaje a tipo t_pedido_s
 * @DESC: Recibe un puntero a un buffer.
 * 		  Devuelve un puntero a t_pedido_s
 */
t_pedido_s* deserializar_pedido(t_buffer* buffer) 
{
	t_pedido_s* paquetePedido = malloc(sizeof(t_pedido_s));
	
	paquetePedido->nombre_restaurante_size = deserializar_int(buffer);
	paquetePedido->nombre_restaurante = deserializar_string(buffer, paquetePedido->nombre_restaurante_size);
	paquetePedido->id_pedido = deserializar_int(buffer);

	return paquetePedido;
}

t_nombre_restaurante_s* deserializar_nombre_restaurante(t_buffer* buffer)
{
	t_nombre_restaurante_s* paqueteNombreRestaurante = malloc(sizeof(t_nombre_restaurante_s));

	memcpy(&(paqueteNombreRestaurante->nombre_restaurante_size), buffer->data,
			sizeof(paqueteNombreRestaurante->nombre_restaurante_size));
	
	buffer->data += sizeof(paqueteNombreRestaurante->nombre_restaurante_size);
	
	paqueteNombreRestaurante->nombre_restaurante = malloc(paqueteNombreRestaurante->nombre_restaurante_size);
	memcpy(paqueteNombreRestaurante->nombre_restaurante, buffer->data,
			paqueteNombreRestaurante->nombre_restaurante_size);
	
	buffer->data += paqueteNombreRestaurante->nombre_restaurante_size;
	
	return paqueteNombreRestaurante;
}

t_single_text_s* deserializar_single_text(t_buffer* buffer)
{
	int offset = 0;
	t_single_text_s* paqueteText = malloc(sizeof(t_single_text_s));

	memcpy(&(paqueteText->text_size), buffer->data + offset,sizeof(paqueteText->text));
	
	offset += sizeof(paqueteText->text_size);
	
	paqueteText->text = malloc(paqueteText->text_size);
	memcpy(paqueteText->text, buffer->data + offset, paqueteText->text_size);
	
	return paqueteText;
}
/**
 * @NAME: serializar pedido
 * @DESC: Recibe el pedido a serializar y puntero en donde volcar la serializacion
 */
void serializar_pedido(t_pedido_s* paquetePedido, char* paqueteSerializado) 
{
	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(paquetePedido->nombre_restaurante_size);
	memcpy(paqueteSerializado + offset, &(paquetePedido->nombre_restaurante_size),size_to_send);
	offset += size_to_send;

	size_to_send = paquetePedido->nombre_restaurante_size;
	memcpy(paqueteSerializado + offset, paquetePedido->nombre_restaurante, paquetePedido->nombre_restaurante_size);
	offset += size_to_send;

	size_to_send = sizeof(paquetePedido->id_pedido);
	memcpy(paqueteSerializado + offset, &(paquetePedido->id_pedido), size_to_send);
}

/**
 * @NAME: serializar nombre de restaurante
 * @DESC: Recibe el nombre del restaurante y puntero en donde volcar la serializacion
 */
void serializar_nombre_restaurante(t_nombre_restaurante_s* paqueteNombreRestaurante, char* paqueteSerializado) 
{
	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(paqueteNombreRestaurante->nombre_restaurante_size);
	
	memcpy(paqueteSerializado + offset, &(paqueteNombreRestaurante->nombre_restaurante_size),
			size_to_send);

	offset += size_to_send;

	size_to_send = paqueteNombreRestaurante->nombre_restaurante_size;
	
	memcpy(paqueteSerializado + offset, paqueteNombreRestaurante->nombre_restaurante,
			strlen(paqueteNombreRestaurante->nombre_restaurante) + 1);
	
	offset += size_to_send;
}

/**
 * @NAME: serializar_datos_restaurante
 * @DESC: Recibe el los datos del restaurante y puntero en donde volcar la serializacion
 */
void serializar_datos_restaurante(t_obtener_restaurante_respuesta_s* paqueteDatosRestaurante, char* paqueteSerializado)
{
	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(paqueteDatosRestaurante->id);
	memcpy(paqueteSerializado + offset, &(paqueteDatosRestaurante->id), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(paqueteDatosRestaurante->cantidadHornos);
	memcpy(paqueteSerializado + offset, &(paqueteDatosRestaurante->cantidadHornos), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(paqueteDatosRestaurante->cantCocineros);
	memcpy(paqueteSerializado + offset, &(paqueteDatosRestaurante->cantCocineros), size_to_send);
	offset += size_to_send;

	for(int i = 0; i < paqueteDatosRestaurante->cantCocineros; i++){

		t_cocinero_s* cocinero = (t_cocinero_s*) list_get(paqueteDatosRestaurante->cocineros, i);

		size_to_send = sizeof(cocinero->id);
		memcpy(paqueteSerializado + offset, &(cocinero->id), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(cocinero->afinidad_size);
		memcpy(paqueteSerializado + offset, &(cocinero->afinidad_size), size_to_send);
		offset += size_to_send;

		if (cocinero->afinidad_size > 0)
		{
			size_to_send = cocinero->afinidad_size;
			memcpy(paqueteSerializado + offset, cocinero->afinidad, strlen(cocinero->afinidad) + 1);
			offset += size_to_send;
		}
	}

	size_to_send = sizeof(paqueteDatosRestaurante->cantPlatos);
	memcpy(paqueteSerializado + offset, &(paqueteDatosRestaurante->cantPlatos), size_to_send);
	offset += size_to_send;

	for(int j = 0; j < paqueteDatosRestaurante->cantPlatos; j++){

		t_plato_s* plato = (t_plato_s*) list_get(paqueteDatosRestaurante->platos, j);

		size_to_send = sizeof(plato->id);
		memcpy(paqueteSerializado + offset, &(plato->id), size_to_send);
		offset += size_to_send;

		size_to_send = sizeof(plato->nombre_size);
		memcpy(paqueteSerializado + offset, &(plato->nombre_size), size_to_send);
		offset += size_to_send;

		size_to_send = plato->nombre_size;
		memcpy(paqueteSerializado + offset, plato->nombre, strlen(plato->nombre) + 1);
		offset += size_to_send;

		size_to_send = sizeof(plato->precio);
		memcpy(paqueteSerializado + offset, &(plato->precio), size_to_send);
		offset += size_to_send;	
	}

	size_to_send = sizeof(paqueteDatosRestaurante->nombre_size);
	memcpy(paqueteSerializado + offset, &(paqueteDatosRestaurante->nombre_size), size_to_send);
	offset += size_to_send;
	
	size_to_send = paqueteDatosRestaurante->nombre_size;
	memcpy(paqueteSerializado + offset, paqueteDatosRestaurante->nombre, strlen(paqueteDatosRestaurante->nombre) + 1);
	offset += size_to_send;
	
	size_to_send = sizeof(paqueteDatosRestaurante->posicion->x);
	memcpy(paqueteSerializado + offset, &(paqueteDatosRestaurante->posicion->x), size_to_send);
	offset += size_to_send;
		
	size_to_send = sizeof(paqueteDatosRestaurante->posicion->y);
	memcpy(paqueteSerializado + offset, &(paqueteDatosRestaurante->posicion->y), size_to_send);
	offset += size_to_send;

	size_to_send = sizeof(uint32_t);
	memcpy(paqueteSerializado + offset, &(paqueteDatosRestaurante->cantidadPedidos), size_to_send);
	offset += size_to_send;		
}

//Revisar se esta pisando el primer cocinero con el segundo
t_cocinero_s* deserializar_cocinero(t_buffer* buffer) 
{
	t_cocinero_s* cocinero = malloc(sizeof(t_cocinero_s));

	cocinero->id = deserializar_int(buffer);
	cocinero->afinidad_size = deserializar_int(buffer);

	if (cocinero->afinidad_size > 0)
		cocinero->afinidad = deserializar_string(buffer, cocinero->afinidad_size);
	else
		cocinero->afinidad = NULL;

	return cocinero;
}

//Revisar se esta pisando el primer plato con el segundo
t_plato_s* deserializar_plato(t_buffer* buffer) 
{
	t_plato_s* plato = malloc(sizeof(t_plato_s));

	plato->id = deserializar_int(buffer);
	plato->nombre_size = deserializar_int(buffer);
	plato->nombre = deserializar_string(buffer, plato->nombre_size);
	plato->precio = deserializar_int(buffer);
	
	return plato;
}

/**
 * @NAME: deserializar datos_del restaurante
 * @DESC: Recibe un puntero a un buffer devuelve un puntero al struct con los datos del restaurante
 */
t_obtener_restaurante_respuesta_s* deserializar_datos_restaurante(t_buffer* buffer)
{
	t_obtener_restaurante_respuesta_s* paqueteDatosRestaurante = malloc(sizeof(t_obtener_restaurante_respuesta_s));
	
	paqueteDatosRestaurante->id = deserializar_int(buffer);
	paqueteDatosRestaurante->cantidadHornos = deserializar_int(buffer);
	paqueteDatosRestaurante->cantCocineros = deserializar_int(buffer);
	
	t_list* listaCocineros = list_create();

	for(int i = 0; i < paqueteDatosRestaurante->cantCocineros; i++)
	{
		t_cocinero_s* cocinero = deserializar_cocinero(buffer);

		list_add(listaCocineros, cocinero);
	}

	paqueteDatosRestaurante->cocineros = listaCocineros;
	paqueteDatosRestaurante->cantPlatos = deserializar_int(buffer);

	t_list* listaPlatos = list_create();

	for(int i = 0; i < paqueteDatosRestaurante->cantPlatos; i++)
	{
		t_plato_s* plato = deserializar_plato(buffer);

		list_add(listaPlatos, plato);
	}

	paqueteDatosRestaurante->platos = listaPlatos;
	paqueteDatosRestaurante->nombre_size = deserializar_int(buffer);;
	paqueteDatosRestaurante->nombre = deserializar_string(buffer, paqueteDatosRestaurante->nombre_size); 
	
	printf("restaurante size %d \n", paqueteDatosRestaurante->nombre_size);
	printf("restaurante nombre %s \n", paqueteDatosRestaurante->nombre);

	t_posicion_s* posicion = malloc(sizeof(t_posicion_s));

	posicion->x = deserializar_int(buffer);
	posicion->y = deserializar_int(buffer);

	paqueteDatosRestaurante->posicion = posicion;
	paqueteDatosRestaurante->cantidadPedidos = deserializar_int(buffer);
	
	return paqueteDatosRestaurante;
}

void serializar_single_text(t_single_text_s* paqueteText, char* paqueteSerializado) 
{
	int offset = 0;
	int size_to_send;

	size_to_send = sizeof(paqueteText->text_size);
	
	memcpy(paqueteSerializado + offset, &(paqueteText->text_size),
			size_to_send);

	offset += size_to_send;

	size_to_send = paqueteText->text_size;
	
	memcpy(paqueteSerializado + offset, paqueteText->text,
			strlen(paqueteText->text) + 1);
	
	offset += size_to_send;
}

t_guardar_plato_s* deserializar_t_guardar_plato(t_buffer* buffer)
{
	t_guardar_plato_s* paquete = malloc(sizeof(t_guardar_plato_s));

	paquete->nombre_restaurante_size = deserializar_int(buffer);
	paquete->nombre_restaurante = deserializar_string(buffer, paquete->nombre_restaurante_size);
	paquete->id_pedido = deserializar_int(buffer);	
	paquete->nombre_plato_size = deserializar_int(buffer);
	paquete->nombre_plato = deserializar_string(buffer, paquete->nombre_plato_size);
	paquete->cantidad = deserializar_int(buffer);
	
	return paquete;
}

uint32_t deserializar_int (t_buffer* buffer)
{
	uint32_t value;

	memcpy(&value, buffer->data, sizeof(uint32_t));

	buffer->data += sizeof(uint32_t);

	return value;
}

char* deserializar_string(t_buffer* buffer, int string_size)
{
	char* string = malloc(sizeof(char) * string_size + 1);

	memcpy(string, buffer->data, string_size);

	string[string_size] = '\0';

	buffer->data += string_size;

	return string;
}

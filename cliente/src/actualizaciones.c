#include "actualizaciones.h"

void recibir_actualizacion(void* socket_cliente){
	t_socket* socketCli = (t_socket*) socket_cliente;
	t_buffer recibido = recibir_mensaje(socketCli->socket);

	switch(recibido.msj_type){
		case T_FINALIZAR_PEDIDO:{
			t_pedido_s* pedido = deserializar_mensaje(&recibido);
			finalizar_pedido(pedido->id_pedido,pedido->nombre_restaurante);
			free(pedido);
			break;
		}
		default:{}
	}
}

void* finalizar_pedido(uint32_t id_pedido, char* nombre_restaurante){

	printf("El pedido %i del restaurante %s finalizo y se entrego.\n",id_pedido,nombre_restaurante);
}
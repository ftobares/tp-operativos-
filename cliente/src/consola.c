#include "consola.h"

void* manejar_consola(){

		while (1) {
		char* entrada = string_new();
		entrada = leerlinea();
		if(string_equals_ignore_case(entrada,"EXIT")){
			printf("Finalizando el programa...\n");
			free(entrada);
			finalizar(socket_cliente_app);
			return -1;
		}
		ejecutarComando(entrada);
		free(entrada);
		}
}


void ejecutarComando(char* input) {

	char ** split;

	split = string_split(input," ");

	char * comando = split[0];

	enum API comandoApi = obtenerComando(comando);

	if(moduloConectado == T_COMANDA || moduloConectado == T_SINDICATO || moduloConectado == T_RESTAURANTE){
		log_info(logger,"Creando socket de conexión.");

		socket_conectado = crear_socket_de_conexion(config->ip, string_itoa(config->puerto));

		log_info(logger,"Conectandose con sindicato...");
	
		conectar_socket(socket_conectado);
	}

	switch(comandoApi){
		case CREAR_PEDIDO:
			printf("crearPedido\n");
			crearPedido();
			free(comando);
			free(split);
			break;
		case CONSULTAR_RESTAURANTES:
			consultarRestaurantes();
			free(comando);
			free(split);
			break;
		case SELECCIONAR_RESTAURANTE:
			seleccionarRestaurante(split[1]); // TODO recibir solo el restaurante, y enviar el ID del cliente por socket
			free(comando);
			free(split);
			break;
		case OBTENER_RESTAURANTE:
			obtenerRestaurante(split[1]);
			free(comando);
			free(split);// Recibe un restaurante
			break;
		case CONSULTAR_PLATOS:
			printf("El restaurante es --> %s.\n\n\n\n",split[1]);
			consultarPlatos(split[1]);
			free(split);
			free(comando);
			break;
		case GUARDAR_PEDIDO:
			guardarPedido(split[1],split[2]);
			free(split);// Recibe un restaurante y un id de pedido
			free(comando);
			break;
		case ANIADIR_PLATO:
			aniadirPlato(split[1],split[2]);
			free(split);// Recibe un plato y un id de pedido
			free(comando);
			break;
		case GUARDAR_PLATO:
			guardarPlato(split[1],split[2],split[3],split[4]);
			free(split); // Recibe un plato y un id de pedido
			free(comando);
			break;
		case CONFIRMAR_PEDIDO:
			if(moduloConectado == T_COMANDA || moduloConectado == T_SINDICATO)
				confirmarPedido(split[1],split[2]);	
			else
				confirmarPedido(split[1]);
			free(split); // Recibe un id de pedido
			free(comando);
			break;
		case PLATO_LISTO:
			platoListo(split[1],split[2],split[3]);
			free(split); // Recibe restaurante, id pedido, y comida
			free(comando);
			break;
		case CONSULTAR_PEDIDO:
			consultarPedido(split[1]);
			free(split);// Recibe id de pedido
			free(comando);
			break;
		case OBTENER_PEDIDO:
			obtenerPedido(split[1],split[2]);
			free(split);// Recibe un restaurante y un id de pedido
			free(comando);
			break;
		case FINALIZAR_PEDIDO:
			finalizarPedido(split[1],split[2]);
			free(split); // Recibe un restaurante y un id de pedido
			free(comando);
			break;
		case TERMINAR_PEDIDO:
			terminarPedido(split[1],split[2]);
			free(split); // Recibe un restaurante y un id de pedido
			free(comando);
			break;
		case OBTENER_RECETA:
			obtenerReceta(split[1]);
			free(split); // Recibe un plato
			free(comando);
			break;
		case ERROR:
			printf("El comando %s no existe.\n",comando);
			char* warning_mensaje = string_new();
			string_append(&warning_mensaje,"El comando ingresado no es valido: ");
			string_append(&warning_mensaje,comando);
			log_warning(logger,warning_mensaje);
			free(warning_mensaje);
			free(split);
			free(comando);
			break;
		case EXIT:
			free(split);
			free(comando);
			break;
	}

	if(moduloConectado == T_COMANDA || moduloConectado == T_SINDICATO || moduloConectado == T_RESTAURANTE){
		//close(socket_conectado.socket);
	}
}

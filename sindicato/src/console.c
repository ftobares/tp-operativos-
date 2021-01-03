#include "console.h"

int runConsole()
{
    printf("Bienvenido al Sindicato. ¿En qué podemos ayudarlo?\n");

	while(1)
	{
		char** instruccion = leerInstruccion();

		if (string_equals_ignore_case(instruccion[0],"EXIT"))
		{
			free(instruccion[0]);
			free(instruccion);
			break;
		}

		else if (string_equals_ignore_case(instruccion[0],"CrearRestaurante"))
			cmd_crearRestaurante(&instruccion[1]);

		else if (string_equals_ignore_case(instruccion[0],"CrearReceta"))
			cmd_crearReceta(&instruccion[1]);

		else if (string_equals_ignore_case(instruccion[0],"help"))
			printf("Las instrucciones disponibles son: CrearRestaurante, CrearReceta\n");

		else if (string_equals_ignore_case(instruccion[0],"obtenerPedido"))
			cmd_testObtenerPedido(instruccion[1], instruccion[2]);

		else if (string_equals_ignore_case(instruccion[0],"guardarPlato"))
			cmd_testGuardarPlato(instruccion[1], instruccion[2],instruccion[3],instruccion[4]);

		else if (string_equals_ignore_case(instruccion[0],"guardarPedido"))
			cmd_testGuardarPedido(instruccion[1], instruccion[2]);

		else if (string_equals_ignore_case(instruccion[0],"confirmarPedido"))
			cmd_testConfirmarPedido(instruccion[1], instruccion[2]);

		else if (string_equals_ignore_case(instruccion[0],"terminarPedido"))
			cmd_testTerminarPedido(instruccion[1], instruccion[2]);

		else if (string_equals_ignore_case(instruccion[0],"platoListo"))
			cmd_testPlatoListo(instruccion[1], instruccion[2], instruccion[3]);

		else if (string_equals_ignore_case(instruccion[0],"obtenerReceta"))
			cmd_testObtenerReceta(instruccion[1]);
		
		else if (string_equals_ignore_case(instruccion[0],"obtenerRestaurante"))
			cmd_testObtenerRestaurante(instruccion[1]);

		else
			printf("No se entendió la instrucción. Para ver las instrucciones disponibles, ingrese 'help'\n");

		free(instruccion[0]);
		free(instruccion);
	} 

	printf("Gracias por confiar en este sindicato\n");

	return 0;
}

void cmd_crearRestaurante(char** parametros)
{
	log_info(logger,"CREANDO RESTAURANTE - Nombre: %s", parametros[0]);

	t_restaurante* restaurante = parseRestauranteFromParams(parametros);

	if (restaurante == NULL)
	{
		log_error(logger, "No se pudo parsear restaurante. Revise los parámetros ingresados.");
	}
	else
	{
		log_info(logger, "Restaurante parseado OK. Guardando en FS.");
		crearRestaurante(restaurante);
	}
}

t_restaurante* parseRestauranteFromParams(char** parametros)
{
	t_restaurante* restaurante = malloc(sizeof(t_restaurante));

	restaurante->nombre = strdup(parametros[0]);
	restaurante->cocineros = create_cocineros_list(parametros[1], parametros[3]);
	restaurante->posicion = parsePosicion(parametros[2]);
	restaurante->platos = create_platos_list(parametros[4], parametros[5]);

	if (restaurante->platos == NULL)
	{
		return NULL;
	}

	restaurante->cantidadHornos = atoi(parametros[6]);

	for (int i = 0; i < 7; i++) free(parametros[i]);

	return restaurante;
}


void cmd_crearReceta(char** parametros)
{
	log_info(logger, "CREANDO RECETA");

	t_receta* receta = parseRecetaFromParams(parametros);

	if (receta == NULL)
	{
		log_info(logger, "No se pudo crear receta");	
		return;
	}

	crearReceta(receta);

	free_receta(receta);
}

t_receta* parseRecetaFromParams(char** parametros)
{
	t_receta* receta = malloc(sizeof(t_receta));

	receta->plato = strdup(parametros[0]);
	receta->pasos = create_pasos_list(parametros[1], parametros[2]);

	if (receta->pasos == NULL) 
	{
		log_error(logger, "No se pudo crear la lista de pasos de la receta.");
		return NULL;
	}
	
	for (int i = 0; i < 3; i++) free(parametros[i]);

	return receta;
}


void cmd_testObtenerPedido(char* nombre_restaurante, char* id_pedido)
{
	log_info(logger, "Testeando obtener pedido");

	char* IP = malloc(sizeof(char) * 10);
	char* PUERTO = malloc(sizeof(char) * 5);

	IP = "127.0.0.1";
	PUERTO = "5003";

	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	
	log_info(logger, "Socket creado");

	if (conectar_socket(conn_socket)) 
	{
		printf("Conectado. Ingrese un número para continuar.");
		
		int seguir_enviando = 1;
		
		t_pedido_s* paquete = malloc(sizeof(t_pedido_s));
		
		log_info(logger, "Enviando");
		
		paquete->nombre_restaurante = strdup(nombre_restaurante);
		paquete->nombre_restaurante_size = strlen(paquete->nombre_restaurante) + 1;
		paquete->id_pedido = atoi(id_pedido);

		free(nombre_restaurante);
		free(id_pedido);

		//Seteo el header
		t_header* header = malloc(sizeof(t_header));
			
		//Defino que tipo de mensaje se envia
		header->msj_type = T_OBTENER_PEDIDO;

		//Seteo el tamanio total del mensaje
		header->size = (sizeof(uint32_t) * 2)
			+ paquete->nombre_restaurante_size;

		t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

		printf("Mando mensaje de tamanio %d\n", strlen(buffer.data));

		if (enviar_mensaje(&buffer)) 
		{
			log_info(logger, "Mensaje enviado");
			log_info(logger, "Recibiendo respuesta");

			t_buffer buffer_rta = recibir_mensaje(conn_socket.socket);

			void* buffer_to_free = buffer_rta.data;

			log_info(logger, "Respuesta recibida");
			log_info(logger, "Buffer size %d", buffer_rta.size);

			t_obtener_pedido_s* pedido = (t_obtener_pedido_s*) deserializar_mensaje(&buffer_rta);

			log_info(logger, "Estado: %d", pedido->estado);
			log_info(logger, "Platos: %d", pedido->cantidadPlatos);

			for (int i = 0; i < pedido->cantidadPlatos; i++)
			{
				t_obtener_pedido_plato_s* plato = list_get(pedido->platos, i); 
				
				log_info(logger, "\tPlato nombre: %s", plato->comida);
				log_info(logger, "\tPlato cant pedida: %d", plato->cantidad);
				log_info(logger, "\tPlato cant lista: %d", plato->cantidadLista);
			}
						
			free(pedido);
			free(buffer_to_free);
		} 
		else 
		{
			perror("Error mandando mensaje\n");
		}

		free(paquete->nombre_restaurante);
		free(paquete);
		free(header);

		return 0;
	} 
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}
}

void cmd_testGuardarPedido(char* nombre_restaurante, char* id_pedido)
{
	log_info(logger, "Testeando crear pedido");

	char* IP = malloc(sizeof(char) * 10);
	char* PUERTO = malloc(sizeof(char) * 5);

	IP = "127.0.0.1";
	PUERTO = "5003";

	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	
	log_info(logger, "Socket creado");

	if (conectar_socket(conn_socket)) 
	{
		int seguir_enviando = 1;
		
		t_pedido_s* paquete = malloc(sizeof(t_pedido_s));
		
		log_info(logger, "Enviando");
		
		paquete->nombre_restaurante = strdup(nombre_restaurante);
		paquete->nombre_restaurante_size = strlen(paquete->nombre_restaurante) + 1;
		paquete->id_pedido = atoi(id_pedido);

		free(id_pedido);
		free(nombre_restaurante);

		//Seteo el header
		t_header* header = malloc(sizeof(t_header));
			
		//Defino que tipo de mensaje se envia
		header->msj_type = T_GUARDAR_PEDIDO;

		//Seteo el tamanio total del mensaje
		header->size = (sizeof(uint32_t) * 2)
			+ paquete->nombre_restaurante_size;

		t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

		printf("Mando mensaje de tamanio %d\n", strlen(buffer.data));

		if (enviar_mensaje(&buffer)) 
		{
			log_info(logger, "Mensaje enviado");
			log_info(logger, "Recibiendo respuesta");

			t_buffer buffer_rta = recibir_mensaje(conn_socket.socket);

			void* buffer_to_free = buffer_rta.data;

			log_info(logger, "Respuesta recibida");
			log_info(logger, "Buffer size %d", buffer_rta.size);

			t_resultado_operacion* resultado = (t_resultado_operacion*) deserializar_mensaje(&buffer_rta);

			log_info(logger, "Resultado: %d", resultado->resultado);

			free(resultado);
			free(buffer_to_free);
		}

		return 0;
	} 
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}
}

void cmd_testConfirmarPedido(char* restaurante, char* id_pedido)
{
	log_info(logger, "Testeando crear pedido");

	char* IP = malloc(sizeof(char) * 10);
	char* PUERTO = malloc(sizeof(char) * 5);

	IP = "127.0.0.1";
	PUERTO = "5003";

	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	
	log_info(logger, "Socket creado");

	if (conectar_socket(conn_socket)) 
	{
		printf("Conectado. Ingrese un número para continuar.");
		
		int seguir_enviando = 1;
		
		t_pedido_s* paquete = malloc(sizeof(t_pedido_s));
		
		log_info(logger, "Enviando");
		
		paquete->nombre_restaurante = strdup(restaurante);
		paquete->nombre_restaurante_size = strlen(restaurante) + 1;
		paquete->id_pedido = atoi(id_pedido);

		free(restaurante);
		free(id_pedido);

		//Seteo el header
		t_header* header = malloc(sizeof(t_header));
			
		//Defino que tipo de mensaje se envia
		header->msj_type = T_CONFIRMAR_PEDIDO;

		//Seteo el tamanio total del mensaje
		header->size = (sizeof(uint32_t) * 2)
			+ paquete->nombre_restaurante_size;

		t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

		printf("Mando mensaje de tamanio %d\n", strlen(buffer.data));

		if (enviar_mensaje(&buffer)) 
		{
			log_info(logger, "Mensaje enviado");
			log_info(logger, "Recibiendo respuesta");

			t_buffer buffer_rta = recibir_mensaje(conn_socket.socket);

			void* buffer_to_free = buffer_rta.data;

			log_info(logger, "Respuesta recibida");
			log_info(logger, "Buffer size %d", buffer_rta.size);

			t_resultado_operacion* resultado = (t_resultado_operacion*) deserializar_mensaje(&buffer_rta);

			log_info(logger, "Resultado: %d", resultado->resultado);

			free(resultado);
			free(buffer_to_free);
		}

		return 0;
	} 
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}
}

void cmd_testTerminarPedido(char* restaurante, char* id_pedido)
{
	log_info(logger, "Testeando crear pedido");

	char* IP = malloc(sizeof(char) * 10);
	char* PUERTO = malloc(sizeof(char) * 5);

	IP = "127.0.0.1";
	PUERTO = "5003";

	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	
	log_info(logger, "Socket creado");

	if (conectar_socket(conn_socket)) 
	{
		printf("Conectado. Ingrese un número para continuar.");

		int seguir_enviando = 1;
		
		t_pedido_s* paquete = malloc(sizeof(t_pedido_s));
		
		log_info(logger, "Enviando");
		
		paquete->nombre_restaurante = strdup(restaurante);
		paquete->nombre_restaurante_size = strlen(restaurante) + 1;
		paquete->id_pedido = atoi(id_pedido);

		free(restaurante);
		free(id_pedido);

		//Seteo el header
		t_header* header = malloc(sizeof(t_header));
			
		//Defino que tipo de mensaje se envia
		header->msj_type = T_TERMINAR_PEDIDO;

		//Seteo el tamanio total del mensaje
		header->size = (sizeof(uint32_t) * 2)
			+ paquete->nombre_restaurante_size;

		t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

		printf("Mando mensaje de tamanio %d\n", strlen(buffer.data));

		if (enviar_mensaje(&buffer)) 
		{
			log_info(logger, "Mensaje enviado");
			log_info(logger, "Recibiendo respuesta");

			t_buffer buffer_rta = recibir_mensaje(conn_socket.socket);

			void* buffer_to_free = buffer_rta.data;

			log_info(logger, "Respuesta recibida");
			log_info(logger, "Buffer size %d", buffer_rta.size);

			t_resultado_operacion* resultado = (t_resultado_operacion*) deserializar_mensaje(&buffer_rta);

			log_info(logger, "Resultado: %d", resultado->resultado);

			free(resultado);
			free(buffer_to_free);
		}

		return 0;
	} 
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}
}

void cmd_testPlatoListo(char* restaurante, char* id_pedido, char* plato)
{
	log_info(logger, "Testeando plato listo");

	char* IP = strdup("127.0.0.1");
	char* PUERTO = strdup("5003");

	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	
	free(IP);
	free(PUERTO);

	log_info(logger, "Socket creado");

	if (conectar_socket(conn_socket)) 
	{
		int seguir_enviando = 1;
		
		t_plato_listo_s* paquete = malloc(sizeof(t_plato_listo_s));
		
		log_info(logger, "Enviando");
		
		paquete->nombre_restaurante = strdup(restaurante);
		paquete->nombre_restaurante_size = strlen(restaurante) + 1;
		paquete->id_pedido = atoi(id_pedido);
		paquete->nombre_plato = strdup(plato);
		paquete->nombre_plato_size = strlen(plato) + 1;

		free(restaurante);
		free(id_pedido);
		free(plato);

		//Seteo el header
		t_header* header = malloc(sizeof(t_header));
			
		//Defino que tipo de mensaje se envia
		header->msj_type = T_PLATO_LISTO;

		//Seteo el tamanio total del mensaje
		header->size = sizeof(t_plato_listo_s)
			+ paquete->nombre_restaurante_size
			+ paquete->nombre_plato_size;

		t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

		printf("Mando mensaje de tamanio %d\n", strlen(buffer.data));

		if (enviar_mensaje(&buffer)) 
		{
			log_info(logger, "Mensaje enviado");
			log_info(logger, "Recibiendo respuesta");

			t_buffer buffer_rta = recibir_mensaje(conn_socket.socket);

			void* buffer_to_free = buffer_rta.data;

			log_info(logger, "Respuesta recibida");
			log_info(logger, "Buffer size %d", buffer_rta.size);

			t_resultado_operacion* resultado = (t_resultado_operacion*) deserializar_mensaje(&buffer_rta);

			log_info(logger, "Resultado: %d", resultado->resultado);

			free(resultado);
			free(buffer_to_free);
		}

		return 0;
	} 
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}
}

void cmd_testGuardarPlato(char* restaurante, char* id_pedido, char* plato, char* cantidad)
{
	log_info(logger, "Testeando GuardarPlato");

	char* IP = malloc(sizeof(char) * 10);
	char* PUERTO = malloc(sizeof(char) * 5);

	IP = "127.0.0.1";
	PUERTO = "5003";

	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	
	log_info(logger, "Socket creado");

	if (conectar_socket(conn_socket)) 
	{
		int seguir_enviando = 1;
		
		t_guardar_plato_s* paquete = malloc(sizeof(t_guardar_plato_s));
		
		log_info(logger, "Enviando");
		
		paquete->nombre_restaurante = strdup(restaurante);
		paquete->nombre_restaurante_size = strlen(restaurante) + 1;
		paquete->nombre_plato = strdup(plato);
		paquete->nombre_plato_size = strlen(plato) + 1;
		paquete->id_pedido = atoi(id_pedido);
		paquete->cantidad = atoi(cantidad);

		free(restaurante);
		free(id_pedido);
		free(plato);
		free(cantidad);

		//Seteo el header
		t_header* header = malloc(sizeof(t_header));
			
		//Defino que tipo de mensaje se envia
		header->msj_type = T_GUARDAR_PLATO;

		//Seteo el tamanio total del mensaje
		header->size = sizeof(t_guardar_plato_s)
			+ paquete->nombre_restaurante_size
			+ paquete->nombre_plato_size;

		t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

		printf("Mando mensaje de tamanio %d\n", strlen(buffer.data));

		if (enviar_mensaje(&buffer)) 
		{
			log_info(logger, "Mensaje enviado");
			log_info(logger, "Recibiendo respuesta");

			t_buffer buffer_rta = recibir_mensaje(conn_socket.socket);

			void* buffer_to_free = buffer_rta.data;

			log_info(logger, "Respuesta recibida");
			log_info(logger, "Buffer size %d", buffer_rta.size);

			t_resultado_operacion* resultado = (t_resultado_operacion*) deserializar_mensaje(&buffer_rta);

			log_info(logger, "Resultado: %d", resultado->resultado);

			free(resultado);
			free(buffer_to_free);
		} 
		else 
		{
			perror("Error mandando mensaje\n");
		}

		free(paquete->nombre_restaurante);
		free(paquete->nombre_plato);
		free(paquete);
		free(header);

		return 0;
	} 
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}
}

void cmd_testObtenerReceta(char* parametro)
{
	log_info(logger, "Testeando obtener receta");

	char* IP = malloc(sizeof(char) * 10);
	char* PUERTO = malloc(sizeof(char) * 5);

	IP = "127.0.0.1";
	PUERTO = "5003";

	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	
	log_info(logger, "Socket creado");

	if (conectar_socket(conn_socket)) 
	{
		printf("Conectado. Ingrese un número para continuar.");

		int seguir_enviando = 1;
		
		t_obtener_receta_s* paquete = malloc(sizeof(t_obtener_receta_s));
		
		log_info(logger, "Enviando");
		
		paquete->nombre_plato = strdup(parametro);
		paquete->nombre_plato_size = strlen(paquete->nombre_plato) + 1;

		free(parametro);

		//Seteo el header
		t_header* header = malloc(sizeof(t_header));
			
		//Defino que tipo de mensaje se envia
		header->msj_type = T_OBTENER_RECETA;

		//Seteo el tamanio total del mensaje
		header->size = sizeof(t_obtener_receta_s)
			+ paquete->nombre_plato_size;

		t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

		printf("Mando mensaje de tamanio %d\n", strlen(buffer.data));

		if (enviar_mensaje(&buffer)) 
		{
			log_info(logger, "Mensaje enviado");
			log_info(logger, "Recibiendo respuesta");

			t_buffer buffer_rta = recibir_mensaje(conn_socket.socket);

			void* buffer_to_free = buffer_rta.data;

			log_info(logger, "Respuesta recibida");
			log_info(logger, "Buffer size %d", buffer_rta.size);

			t_obtener_receta_respuesta* receta = (t_obtener_receta_respuesta*) deserializar_mensaje(&buffer_rta);

			log_info(logger, "Receta: %s", receta->nombre_plato);
			log_info(logger, "Pasos: %d", receta->cantidadPasos);

			for (int i = 0; i < receta->cantidadPasos; i++)
			{
				t_paso_s* paso = list_get(receta->pasos, i);

				log_info(logger, "Paso: %s", paso->nombre_paso);
				log_info(logger, "Tiempo: %d", paso->tiempo);
			}

			free(receta);
			free(buffer_to_free);
		}

		return 0;
	} 
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}
}

void cmd_testObtenerRestaurante(char* parametro)
{
	log_info(logger, "Testeando obtener restaurante");

	char* IP = malloc(sizeof(char) * 10);
	char* PUERTO = malloc(sizeof(char) * 5);

	IP = "127.0.0.1";
	PUERTO = "5003";

	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	
	log_info(logger, "Socket creado");

	if (conectar_socket(conn_socket)) 
	{
		int seguir_enviando = 1;
		
		t_nombre_restaurante_s* paquete = malloc(sizeof(t_nombre_restaurante_s));
		
		log_info(logger, "Enviando");
		
		int size = strlen(parametro) + 1;

		paquete->nombre_restaurante = strdup(parametro);
		paquete->nombre_restaurante_size = size;

		free(parametro);

		//Seteo el header
		t_header* header = malloc(sizeof(t_header));
			
		//Defino que tipo de mensaje se envia
		header->msj_type = T_OBTENER_RESTAURANTE;

		//Seteo el tamanio total del mensaje
		header->size = sizeof(t_nombre_restaurante_s)
			+ paquete->nombre_restaurante_size;

		t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

		printf("Mando mensaje de tamanio %d\n", strlen(buffer.data));

		if (enviar_mensaje(&buffer)) 
		{
			log_info(logger, "Mensaje enviado");
			log_info(logger, "Recibiendo respuesta");

			t_buffer buffer_rta = recibir_mensaje(conn_socket.socket);

			void* buffer_to_free = buffer_rta.data;

			log_info(logger, "Respuesta recibida");
			log_info(logger, "Buffer size %d", buffer_rta.size);

			t_obtener_restaurante_respuesta_s* restaurante = (t_obtener_restaurante_respuesta_s*) deserializar_mensaje(&buffer_rta);

			log_info(logger, "Restaurante: %s", restaurante->nombre);
			log_info(logger, "Cocineros: %d", restaurante->cantCocineros);

			for (int i = 0; i < restaurante->cantCocineros; i++)
			{
				t_cocinero_s* cocinero = list_get(restaurante->cocineros, i);

				log_info(logger, "ID: %d", cocinero->id);
				if (cocinero->afinidad != NULL) log_info(logger, "Afinidad: %s", cocinero->afinidad);
			}
			
			log_info(logger, "Platos: %d", restaurante->cantPlatos);
			log_info(logger, "Pedidos: %d", restaurante->cantidadPedidos);
			
			free(restaurante->nombre);
			free(restaurante->posicion);
			for (int i = 0; i < restaurante->cantCocineros; i++)
			{
				t_cocinero_s* cocinero = list_get(restaurante->cocineros, i); 
				if (cocinero->afinidad != NULL) free(cocinero->afinidad);
			}
			list_destroy_and_destroy_elements(restaurante->cocineros, &free);
			for (int i = 0; i < restaurante->cantPlatos; i++)
			{
				t_plato_s* plato = list_get(restaurante->platos, i); 
				free(plato->nombre);
			}
			list_destroy_and_destroy_elements(restaurante->platos, &free);
			free(restaurante);
			free(buffer_to_free);
		}

		return 0;
	} 
	else 
	{
		perror("No se pudo conectar\n");
		return 1;
	}
}
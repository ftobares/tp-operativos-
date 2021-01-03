# TP Operativos
## Grupo: Snoop (tp-2020-2c-Snoop-)

Index

  - Definiciones de diseño
  - Instalación
  - Librerias

# Definiciones de diseño

En esta sección explicaremos el diseño y la implementación que hemos definido teniendo en cuenta el marco del [enunciado del trabajo practico](https://docs.google.com/document/d/13JCJawPWfL2y6wGTBYykUTvQY5Uso0iYpXSFDAQFbFw/edit)

> blablabla
> blablabla

### Definicion 1

Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.

# Instalación

-- Agregar explicación sobre como instalar el proyecto, makefiles y como ejecutarlo

### Makefiles

```sh
TARGET = cliente
CC = gcc
UTILS_PATH = "../utiles"
LIBS = -lcommons -lutiles -lreadline
CFLAGS = -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF
```

### Dependencias

Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy.

| Libreria | Funcionalidad |
| ------ | ------ |
| utiles/[models](librerias/models/README.md) | ... |
| utiles/[serializer](librerias/models/README.md) | ... |
| utiles/[utiles_config](librerias/models/README.md) | ... |
| utiles/[utiles](librerias/models/README.md) | ... |

# Librerias

### Sockets

### Serializer

### Models

### Utiles_Config

### Utiles

### Ejemplo completo (Sockets + Serializacion)

Con el fin de simplificar el ejemplo, el código no esta modularizado y solamente se acepta como entrada el mensaje **guardar_plato**:

#### Cliente
```c
int main() {
	t_socket conn_socket = crear_socket_de_conexion(IP, PUERTO);
	if (conectar_socket(conn_socket)) {
		int seguir_enviando = 1;
		printf("Conectado. Ingrese guardar_plato seguido de nombre restaurante, id del pedido, plato y cantidad\n");
		printf("Ejemplo: guardar_plato rodizio 10 milanesa 2\n");
		printf("Escriba exit para salir\n");

		while (seguir_enviando) {
			char * comando;
			char ** parametros;

			leerInstruccion(&comando, &parametros);
			if (string_equals_ignore_case(comando,"EXIT"))
				seguir_enviando = 0;

			printf("Comando %s\n", comando);
			if (seguir_enviando) {
				t_guardar_plato_s* paquete = malloc(sizeof(t_guardar_plato_s));

				//Seteo valores en el paquete y reservo memoria
				paquete->nombre_restaurante = malloc(strlen(parametros[0]));
				strcpy(paquete->nombre_restaurante, strcat(parametros[0],"\0"));
				paquete->nombre_restaurante_size = strlen(paquete->nombre_restaurante) + 1;

				paquete->id_pedido = atoi(parametros[1]);

				paquete->nombre_plato = malloc(strlen(parametros[2]));
				strcpy(paquete->nombre_plato, strcat(parametros[2],"\0"));
				paquete->nombre_plato_size = strlen(paquete->nombre_plato) + 1;

				paquete->cantidad = atoi(parametros[3]);

				//Seteo el header
				t_header* header = malloc(sizeof(t_header));
				//Defino que tipo de mensaje se envia
				header->msj_type = T_GUARDAR_PLATO;

				//Seteo el tamanio total del mensaje
				header->size = (sizeof(uint32_t) * 4)
						+ paquete->nombre_restaurante_size
						+ paquete->nombre_plato_size;

				t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

				printf("Mando mensaje de tamanio %i\n", strlen(buffer.data));

				if (enviar_mensaje(&buffer)) {
					printf("Mensaje enviado\n");
				} else {
					perror("Error mandando mensaje\n");
				}

				free(paquete->nombre_plato);
				free(paquete->nombre_restaurante);
				free(paquete);
				free(header);
			}
		}

		return 0;
	} else {
		perror("No se pudo conectar\n");
		return 1;
	}
}
```

#### Servidor
```c
int main() {

	printf("Inicio Server");

	t_socket socket = crear_socket_de_escucha(PUERTO);
	printf("Socket de escucha creado\n");

	if (bind_listen_socket_escucha(socket)) {
		int accepting = 1;
		printf(
				"Bind & Listen socket de escucha. Esperando conexiones entrantes...\n");
		while (accepting) {
			t_socket socketCliente = aceptando_conexiones(socket);
			if (socketCliente.socket != -1) {
				printf("Mensaje entrante\n");
				t_buffer buffer = recibir_mensaje(socketCliente.socket);

				printf("Mensaje recibido y guardado en buffer, deserializando...\n");
				t_guardar_plato_s* paquete = (t_guardar_plato_s*) deserializar_mensaje(&buffer);

				printf("ID Pedido: %s\n", paquete->id_pedido);
				printf("Nombre plato: %s\n", paquete->nombre_plato);
				printf("Nombre Restaurante: %s\n", paquete->nombre_restaurante);
				printf("Cantidad del plato: %s\n", paquete->cantidad);

				free(paquete->nombre_plato);
				free(paquete->nombre_restaurante);
				free(paquete);
			} else {
				accepting = 0;
			}
		}
	} else {
		perror("No se pudo conectar\n");
		return 1;
	}

	printf("Fin Server");

	return 0;
}
```
OBTENER PEDIDO, GUARDAR PEDIDO, CONFIRMAR PEDIDO Y FINALIZAR PEDIDO:
char* host = "127.0.0.1";
char* puerto = "5001";
t_socket conn_socket = crear_socket_de_conexion(host, puerto);
if (conectar_socket(conn_socket)) {

    printf("Nombre del restaurante a enviar:\n");
    char* nombreRestaurante = leerlinea();
    printf("ID del pedido (numerico):\n");
    int idPedido = atoi(leerlinea());

    t_pedido_s* paquete = malloc(sizeof(t_pedido_s));
    paquete->nombre_restaurante = malloc(strlen(nombreRestaurante));
    strcpy(paquete->nombre_restaurante, strcat(nombreRestaurante,"\0"));
    paquete->nombre_restaurante_size = strlen(paquete->nombre_restaurante) + 1;		
    paquete->id_pedido = idPedido;
    t_header* header = malloc(sizeof(t_header));
    header->msj_type = T_OBTENER_PEDIDO/T_GUARDAR_PEDIDO/T_CONFIRMAR_PEDIDO/T_FINALIZAR_PEDIDO;
    header->size = (sizeof(uint32_t) * 2) + paquete->nombre_restaurante_size;
    t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);
    if (enviar_mensaje(&buffer)) {
        printf("Mensaje enviado\n");
    } else {
        perror("Error mandando mensaje\n");
    }
    free(paquete->nombre_restaurante);
    free(nombreRestaurante);
    free(paquete);
    free(header);
} else {
    perror("No se pudo conectar\n");
}


PLATO LISTO:
char* host = "127.0.0.1";
char* puerto = "5001";
t_socket conn_socket = crear_socket_de_conexion(host, puerto);
if (conectar_socket(conn_socket)) {

    printf("Nombre del restaurante a enviar:\n");
    char* nombreRestaurante = leerlinea();
    printf("ID del pedido (numerico):\n");
    int idPedido = atoi(leerlinea());
    printf("Nombre del plato:\n");
    char* nombrePlato = leerlinea();

    t_plato_listo_s* paquete = malloc(sizeof(t_plato_listo_s));

    paquete->nombre_restaurante = malloc(strlen(nombreRestaurante));
    strcpy(paquete->nombre_restaurante, strcat(nombreRestaurante,"\0"));
    paquete->nombre_restaurante_size = strlen(paquete->nombre_restaurante) + 1;		

    paquete->id_pedido = idPedido;

    paquete->nombre_plato = malloc(strlen(nombrePlato));
    strcpy(paquete->nombre_plato, strcat(nombrePlato,"\0"));
    paquete->nombre_plato_size = strlen(paquete->nombre_plato) + 1;

    t_header* header = malloc(sizeof(t_header));
    header->msj_type = T_PLATO_LISTO;
    header->size = (sizeof(uint32_t) * 3) + 
                    paquete->nombre_restaurante_size + 
                    paquete->nombre_plato_size;

    t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

    if (enviar_mensaje(&buffer)) {
        printf("Mensaje enviado\n");
    } else {
        perror("Error mandando mensaje\n");
    }
    free(paquete);
    free(header);
} else {
    perror("No se pudo conectar\n");
}


GUARDAR PLATO:
char* host = "127.0.0.1";
char* puerto = "5001";
t_socket conn_socket = crear_socket_de_conexion(host, puerto);
if (conectar_socket(conn_socket)) {

    printf("Nombre del restaurante a enviar:\n");
    char* nombreRestaurante = leerlinea();
    printf("ID del pedido (numerico):\n");
    int idPedido = atoi(leerlinea());
    printf("Nombre del plato:\n");
    char* nombrePlato = leerlinea();
    printf("Cantidad del plato:\n");
    int cantidad = atoi(leerlinea());

    t_guardar_plato_s* paquete = malloc(sizeof(t_guardar_plato_s));

    paquete->nombre_restaurante = malloc(strlen(nombreRestaurante));
    strcpy(paquete->nombre_restaurante, strcat(nombreRestaurante,"\0"));
    paquete->nombre_restaurante_size = strlen(paquete->nombre_restaurante) + 1;		

    paquete->id_pedido = idPedido;
    paquete->cantidad = cantidad;

    paquete->nombre_plato = malloc(strlen(nombrePlato));
    strcpy(paquete->nombre_plato, strcat(nombrePlato,"\0"));
    paquete->nombre_plato_size = strlen(paquete->nombre_plato) + 1;

    t_header* header = malloc(sizeof(t_header));
    header->msj_type = T_GUARDAR_PLATO;
    header->size = (sizeof(uint32_t) * 4) + 
                    paquete->nombre_restaurante_size + 
                    paquete->nombre_plato_size;

    t_buffer buffer = serializar_mensaje(paquete, header->msj_type, header->size, conn_socket.socket);

    if (enviar_mensaje(&buffer)) {
        printf("Mensaje enviado\n");
    } else {
        perror("Error mandando mensaje\n");
    }
    free(paquete);
    free(header);
} else {
    perror("No se pudo conectar\n");
}

#include "fileSystem.h"

int runFileSystem()
{
    return inicializar();
}

int inicializar()
{
    log_info(logger, "Inicializando File System AFIP");

    iniciar_semaforos();

    iniciar_constantes();

    iniciar_metadata_config();

    iniciar_directorios();

    iniciar_bloques();

    iniciar_bitmap();

    return 0; 
}

int finalizar_file_system()
{
    guardar_bitmap();

    bitarray_destroy(bitmap);
    free(bit_array);

    free(file_system_metadata->magic_number);
    free(file_system_metadata);

    finalizar_semaforos();
}

t_receta* get_receta(t_obtener_receta_s* paquete)
{
    log_info(logger, "Obteniendo receta de %s", paquete->nombre_plato);

    char* info_path = get_receta_path(paquete->nombre_plato);

    char* receta_text = read_file(info_path);
    
    log_info(logger, "Receta leída: %s", receta_text);

    free(info_path);

    if (receta_text == NULL)
    {
        log_info(logger, "No se encontró receta del plato %s", paquete->nombre_plato);

        return NULL;
    }

    t_receta* receta = receta_text_to_struct(paquete->nombre_plato, receta_text);

    free(receta_text);

    return receta;
}

t_restaurante* get_restaurante(char* nombre)
{
    log_info(logger, "Obteniendo restaurante %s", nombre);

    char* info_path = get_restaurante_info_path(nombre);

    char* restaurante_text = read_file(info_path);

    free(info_path);    

    if (restaurante_text == NULL)
    {
        log_info(logger, "No se encontró restaurante %s", nombre);
        return NULL;
    }

    log_info(logger, "Restaurante: %s", restaurante_text);

    t_restaurante* restaurante = restaurante_text_to_struct(nombre, restaurante_text); 

    free(restaurante_text);

    return restaurante;
}

t_pedido* get_pedido(t_pedido_s* _pedido)
{
    log_info(logger, "Obteniendo pedido %s %d", _pedido->nombre_restaurante, _pedido->id_pedido);

    char* info_path = get_restaurante_pedido_path(_pedido->nombre_restaurante, _pedido->id_pedido);
    char* pedido_text = read_file(info_path);

    free(info_path);    

    if (pedido_text == NULL)
    {
        log_info(logger, "No se encontró pedido %s %d", _pedido->nombre_restaurante, _pedido->id_pedido);
        return NULL;
    }

    log_info(logger, "Pedido: %s", pedido_text);

    t_pedido* pedido = pedido_text_to_struct(_pedido->nombre_restaurante, _pedido->id_pedido, pedido_text); 

    log_info(logger, "Pedido tts ok");

    free(pedido_text);

    return pedido;
}

char* read_file(char* info_path)
{
    t_file_info* file_info = get_file_info(info_path);

    if (file_info == NULL) return NULL;

    char* file_content = malloc(sizeof(char) * (file_info->size + 1));
    
    if (read_file_content(file_content, file_info) == -1) return NULL;

    file_content[file_info->size] = '\0';

    free(file_info);

    return file_content;
}

int guardar_receta(t_receta* receta)
{
    log_info(logger, "Guardando receta");

    char* receta_text = get_receta_text(receta);
    
    log_info(logger, "Receta size: %d", strlen(receta_text));

    int bloque_inicial = pedir_bloque();

    log_info(logger, "Bloque inicial: %d", bloque_inicial);

    char* receta_path = get_receta_path(receta->plato);
    char* receta_metadata_text = get_file_metadata_text(strlen(receta_text), bloque_inicial);

    write_text_file(receta_path, receta_metadata_text);

    free(receta_metadata_text);
    free(receta_path);

    write_file_content(receta_text, bloque_inicial);

    free(receta_text);

    log_info(logger, "Receta guardada correctamente");

    return 0;
}

int guardar_restaurante(t_restaurante* restaurante)
{
    log_info(logger, "Guardando restaurante");

    char* restaurante_text = get_restaurante_text(restaurante);
    
    log_info(logger, "Restaurante size: %d", strlen(restaurante_text));
    log_info(logger, "Restaurante text: %s", restaurante_text);

    int bloque_inicial = pedir_bloque();

    log_info(logger, "Bloque inicial: %d", bloque_inicial);

    char* restaurante_directory_path = get_restaurante_directory_path(restaurante->nombre);
    
    log_info(logger, "Comprobando directorio %s", restaurante_directory_path);

    crear_directorio_si_no_existe(restaurante_directory_path);

    free(restaurante_directory_path);

    char* restaurante_info_path = get_restaurante_info_path(restaurante->nombre);

    char* restaurante_metadata_text = get_file_metadata_text(strlen(restaurante_text), bloque_inicial);

    log_info(logger, "Restaurante metadata text: %s", restaurante_metadata_text);

    write_text_file(restaurante_info_path, restaurante_metadata_text);

    free(restaurante_metadata_text);
    free(restaurante_info_path);

    write_file_content(restaurante_text, bloque_inicial);

    free(restaurante_text);

    log_info(logger, "Restaurante guardado correctamente");

    return 0;
}

int guardar_pedido(t_pedido_s* _pedido)
{
    log_info(logger, "Guardando Pedido");

    t_restaurante* restaurante = get_restaurante(_pedido->nombre_restaurante);

    if (restaurante == NULL)
    {
        log_error(logger, "No se pudo guardar pedido. No se encontró restaurante.");
        return -1;
    }

    free_restaurante(restaurante);

    t_pedido* pedido_existente = get_pedido(_pedido);

    if (pedido_existente != NULL)
    {
        log_error(logger, "No se pudo guardar pedido. Se encontró uno con el mismo ID.");

        free_pedido(pedido_existente);

        return -1;
    }

    t_pedido* pedido = malloc(sizeof(t_pedido));

    pedido->id = _pedido->id_pedido;
    pedido->nombre_restaurante = strdup(_pedido->nombre_restaurante);
    pedido->estado = PEDIDO_PENDIENTE;
    pedido->precio = 0;
    pedido->platos = list_create();

    char* pedido_text = get_pedido_text(pedido);

    log_info(logger, "Pedido size: %d", strlen(pedido_text));
    log_info(logger, "Pedido text: %s", pedido_text);

    int bloque_inicial = pedir_bloque();

    log_info(logger, "Bloque inicial: %d", bloque_inicial);

    char* restaurante_pedido_path = get_restaurante_pedido_path(pedido->nombre_restaurante, pedido->id);
    char* pedido_metadata_text = get_file_metadata_text(strlen(pedido_text), bloque_inicial);

    free_pedido(pedido);

    log_info(logger, "Pedido metadata text: %s", pedido_metadata_text);

    write_text_file(restaurante_pedido_path, pedido_metadata_text);

    free(pedido_metadata_text);
    free(restaurante_pedido_path);

    write_file_content(pedido_text, bloque_inicial);

    free(pedido_text);

    log_info(logger, "Pedido guardado correctamente");

    return 0;
}

int guardar_plato(t_guardar_plato_s* paquete_plato)
{
    t_restaurante* restaurante = get_restaurante(paquete_plato->nombre_restaurante);

    if (restaurante == NULL)
    {
        log_error(logger, "No se pudo guardar plato porque no se encontró el restaurante.");

        return -1;
    }

    t_pedido_s* _pedido = malloc(sizeof(t_pedido_s));

    _pedido->nombre_restaurante = strdup(paquete_plato->nombre_restaurante);
    _pedido->id_pedido = paquete_plato->id_pedido;

    t_pedido* pedido = get_pedido(_pedido);

    free(_pedido->nombre_restaurante);
    free(_pedido);

    if (pedido == NULL)
    {
        log_error(logger, "No se pudo guardar plato porque no se encontró el pedido.");

        free_restaurante(restaurante);

        return -1;
    }

    if (pedido->estado != PEDIDO_PENDIENTE)
    {
        log_error(logger, "No se pudo guardar plato porque el pedido no está pendiente");

        free_restaurante(restaurante);
        free_pedido(pedido);

        return -1;
    }

    int precio;
    bool existe_plato_en_restaurante = false;

    for (int i = 0; i < list_size(restaurante->platos); i++)
    {
        t_plato* plato_aux = list_get(restaurante->platos, i);

        if (string_equals_ignore_case(paquete_plato->nombre_plato, plato_aux->nombre))
        {
            precio = plato_aux->precio;
            existe_plato_en_restaurante = true;
        }
    }

    if (!existe_plato_en_restaurante) 
    {
        log_error(logger, "No se pudo guardar plato porque no se encontró el plato en el restaurante indicado");

        free_restaurante(restaurante);
        free_pedido(pedido);

        return -1;
    }

    pedido->precio += precio * paquete_plato->cantidad; 

    free_restaurante(restaurante);

    t_pedido_plato* plato = NULL;

    for (int i = 0; i < list_size(pedido->platos); i++)
    {
        t_pedido_plato* plato_aux = list_get(pedido->platos, i);

        if (string_equals_ignore_case(paquete_plato->nombre_plato, plato_aux->nombre_plato))
        {
            plato = plato_aux;
            break;
        } 
    }

    if (plato == NULL)
    {
        t_pedido_plato* plato = malloc(sizeof(t_pedido_plato));

        plato->nombre_plato = strdup(paquete_plato->nombre_plato);
        plato->cantidad_pedida = paquete_plato->cantidad;
        plato->cantidad_lista = 0;

        list_add(pedido->platos, plato);
    }
    else
    {
        plato->cantidad_pedida += paquete_plato->cantidad;
    }

    char* info_path = get_restaurante_pedido_path(pedido->nombre_restaurante, pedido->id);
    char* pedido_text = get_pedido_text(pedido);

    free_pedido(pedido);

    update_file_content(info_path, pedido_text);

    free(info_path);
    free(pedido_text);

    return 0;
}

int confirmar_pedido(t_pedido_s* _pedido)
{
    t_restaurante* restaurante = get_restaurante(_pedido->nombre_restaurante);

    if (restaurante == NULL)
    {
        log_error(logger, "No se pudo confirmar pedido porque no se encontró el restaurante.");

        return -1;
    }

    free_restaurante(restaurante);

    t_pedido* pedido = get_pedido(_pedido);

    if (pedido == NULL)
    {
        log_error(logger, "No se pudo confirmar pedido porque no se encontró el pedido.");

        return -1;
    }

    if (pedido->estado != PEDIDO_PENDIENTE)
    {
        log_error(logger, "No se pudo confirmar pedido porque el pedido no está pendiente");

        free_pedido(pedido);

        return -1;
    }

    pedido->estado = PEDIDO_CONFIRMADO;

    char* info_path = get_restaurante_pedido_path(pedido->nombre_restaurante, pedido->id);
    char* pedido_text = get_pedido_text(pedido);

    free_pedido(pedido);

    update_file_content(info_path, pedido_text);

    free(info_path);
    free(pedido_text);

    return 0;
}

int terminar_pedido(t_pedido_s* _pedido)
{
    t_restaurante* restaurante = get_restaurante(_pedido->nombre_restaurante);

    if (restaurante == NULL)
    {
        log_error(logger, "No se pudo terminar pedido porque no se encontró el restaurante.");

        return -1;
    }

    free_restaurante(restaurante);

    t_pedido* pedido = get_pedido(_pedido);

    if (pedido == NULL)
    {
        log_error(logger, "No se pudo terminar pedido porque no se encontró el pedido.");

        return -1;
    }

    if (pedido->estado != PEDIDO_CONFIRMADO)
    {
        log_error(logger, "No se pudo terminar pedido porque el pedido no está confirmado");

        free_pedido(pedido);

        return -1;
    }

    pedido->estado = PEDIDO_TERMINADO;

    char* info_path = get_restaurante_pedido_path(pedido->nombre_restaurante, pedido->id);
    char* pedido_text = get_pedido_text(pedido);

    free_pedido(pedido);

    update_file_content(info_path, pedido_text);

    free(info_path);
    free(pedido_text);

    return 0;
}

int plato_listo(t_plato_listo_s* paquete)
{
    t_restaurante* restaurante = get_restaurante(paquete->nombre_restaurante);

    if (restaurante == NULL)
    {
        log_error(logger, "No se pudo cargar plato listo porque no se encontró el restaurante.");

        return -1;
    }

    free_restaurante(restaurante);

    t_pedido_s* _pedido = malloc(sizeof(t_pedido_s));

    _pedido->nombre_restaurante = strdup(paquete->nombre_restaurante);
    _pedido->id_pedido = paquete->id_pedido;

    t_pedido* pedido = get_pedido(_pedido);

    free(_pedido->nombre_restaurante);
    free(_pedido);

    if (pedido == NULL)
    {
        log_error(logger, "No se pudo cargar plato listo porque no se encontró el pedido.");

        return -1;
    }

    if (pedido->estado != PEDIDO_CONFIRMADO)
    {
        log_error(logger, "No se pudo cargar plato listo porque el pedido no está confirmado");

        free_pedido(pedido);

        return -1;
    }

    t_pedido_plato* plato = NULL;

    for (int i = 0; i < list_size(pedido->platos); i++)
    {
        t_pedido_plato* plato_aux = list_get(pedido->platos, i);

        if (string_equals_ignore_case(paquete->nombre_plato, plato_aux->nombre_plato))
        {
            plato = plato_aux;
            break;
        } 
    }

    if (plato == NULL)
    {
        log_error(logger, "No se pudo cargar plato listo porque no se encontró el plato en el pedido");

        free_pedido(pedido);
        return -1;
    }
    
    plato->cantidad_lista++;
    
    if (plato->cantidad_lista > plato->cantidad_pedida)
    {
        log_error(logger, "No se pudo cargar plato listo porque la cantidad lista supera la pedida");

        free_pedido(pedido);
        return -1;
    }

    char* info_path = get_restaurante_pedido_path(pedido->nombre_restaurante, pedido->id);
    char* pedido_text = get_pedido_text(pedido);

    free_pedido(pedido);

    update_file_content(info_path, pedido_text);

    free(info_path);
    free(pedido_text);

    return 0;
}

int get_cantidad_pedidos(char* restaurante)
{
    char* directory_path = get_restaurante_directory_path(restaurante);

    int cantidad = 0;

    struct dirent *dp;

	DIR *dfd = opendir(directory_path);
	
    free(directory_path);

    if(dfd != NULL) 
    {
		while((dp = readdir(dfd)) != NULL)
            if (string_contains(dp->d_name, "Pedido")) 
                cantidad++;

        closedir(dfd);
	}

    free(dp);

    return cantidad;
}

t_file_info* get_file_info(char* info_path)
{
    t_config* info_config = config_create(info_path);

    if (info_config == NULL) return NULL;

    t_file_info* file_info = malloc(sizeof(t_file_info));

    file_info->size = config_get_int_value(info_config, "SIZE");
    file_info->initial_block = config_get_int_value(info_config, "INITIAL_BLOCK");

    config_destroy(info_config);

    return file_info;
}

int read_file_content(char* file_content, t_file_info* file_info)
{
    log_info(logger, "Reading file content - Size: %d - Initial block: %d", file_info->size, file_info->initial_block);
    
    char* block_path = get_block_path(file_info->initial_block);

    FILE* block_file = fopen(block_path, "r");

    free(block_path);

    if (file_info->size > file_system_metadata->block_size)
    {
        int bytes_to_read = file_system_metadata->block_size - 4;

        fread(file_content, sizeof(char), bytes_to_read, block_file);
        
        uint32_t next_block;

        fread(&next_block, sizeof(uint32_t), 1, block_file);

        t_file_info* next_block_info = malloc(sizeof(t_file_info));

        next_block_info->size = file_info->size - bytes_to_read;
        next_block_info->initial_block = next_block;

        read_file_content(&(file_content[bytes_to_read]), next_block_info);
        
        free(next_block_info);
    }
    else
    {
        fread(file_content, sizeof(char), file_info->size, block_file);

        file_content[file_info->size] = '\0';  
    }

    fclose(block_file);

    return 0;
}

int update_file_content(char* info_path, char* new_text)
{
    log_info(logger, "Actualizando archivo");

    t_file_info* file_info = get_file_info(info_path);

    t_list* file_blocks = list_create();

    get_file_blocks(file_blocks, file_info);

    free(file_info);

    //LIBERO TODOS LOS BLOQUES MENOS EL INICIAL
    for (int i = 1; i < list_size(file_blocks); i++)
    {
        liberar_bloque(list_get(file_blocks, i));
    }

    write_file_content(new_text, list_get(file_blocks, 0));

    char* file_info_new_text = get_file_metadata_text(strlen(new_text), list_get(file_blocks, 0));
    
    list_destroy(file_blocks);
    
    write_text_file(info_path, file_info_new_text);

    free(file_info_new_text);

    return 0;
}

int get_file_blocks(t_list* block_list, t_file_info* file_info)
{
    char* block_path = get_block_path(file_info->initial_block);

    FILE* block_file = fopen(block_path, "r");

    free(block_path);

    list_add(block_list, file_info->initial_block);

    if (file_info->size > file_system_metadata->block_size)
    {
        int bytes_to_read = file_system_metadata->block_size - 4; 
        fseek(block_file, bytes_to_read, 0);

        uint32_t next_block;

        fread(&next_block, sizeof(uint32_t), 1, block_file);

        t_file_info* next_block_info = malloc(sizeof(t_file_info));

        next_block_info->size = file_info->size - bytes_to_read;
        next_block_info->initial_block = next_block;

        get_file_blocks(block_list, next_block_info);
        
        free(next_block_info);
    }

    fclose(block_file);

    return 0;
}

int blocks_for_size(int size)
{
    int blocks = 0;

    if (size > file_system_metadata->block_size)
    {
        blocks += blocks_for_size(size - (file_system_metadata->block_size - sizeof(uint32_t)));
    }

    blocks++;

    return blocks;
}

int write_file_content(char* file_content, uint32_t initial_block)
{
    log_info(logger, "Writing file content - Length: %d - Initial block: %d", strlen(file_content), initial_block);
    
    bool final_block = strlen(file_content) <= file_system_metadata->block_size;

    uint32_t next_block = -1;

    char* content_to_write;

    if (!final_block)
    {
        int length_to_write = file_system_metadata->block_size - 4;

        content_to_write = string_substring(file_content, 0, length_to_write);

        next_block = pedir_bloque();

        int remaining_length = strlen(file_content) - length_to_write;

        char* remaining_content = string_substring(file_content, length_to_write, remaining_length);

        write_file_content(remaining_content, next_block);

        free(remaining_content);
    }
    else
    {
        content_to_write = strdup(file_content);
    }

    char* block_path = get_block_path(initial_block);

    FILE* block_file = fopen(block_path, "w+");

    fwrite(content_to_write, sizeof(char), strlen(content_to_write), block_file);
    
    if (next_block != -1) fwrite(&next_block, sizeof(uint32_t), 1, block_file);

    fclose(block_file);
    
    free(block_path);
    free(content_to_write);
}

char* get_block_full_string(int block)
{
    char* block_string = int_to_string(block);

    int filling_chars = 4 - strlen(block_string);

    if (filling_chars > 0)
    {
        char* block_full_string = malloc(sizeof(char) * 4);

        char* filling = malloc(sizeof(char) * filling_chars);

        strcpy(filling, "0");

        for (int i = 1; i < filling_chars; i++)
        {
            strcat(filling, "0");
        }

        strcpy(block_full_string, filling);
        strcat(block_full_string, block_string);

        return block_full_string;
    }

    return block_string;
}

char* get_file_metadata_text(int size, int initial_block)
{
    char* size_str = int_to_string(size);
    char* initial_block_str = int_to_string(initial_block);
    
    char* metadata_text = strdup("SIZE=");

    string_append(&metadata_text, size_str);
    string_append(&metadata_text, "\nINITIAL_BLOCK=");
    string_append(&metadata_text, initial_block_str);

    free(size_str);
    free(initial_block_str);

    return metadata_text;
}

char* get_receta_text(t_receta* receta)
{
    log_info(logger, "Getting receta text");

    t_list* lista_pasos = list_create();
    t_list* lista_tiempos = list_create();

    for (int i = 0; i < list_size(receta->pasos); i++)
    {
        t_paso* paso = list_get(receta->pasos, i);

        char* tiempo_str = int_to_string(paso->tiempo);

        list_add(lista_pasos, paso->nombre);
        list_add(lista_tiempos, tiempo_str);
    }

    char* lista_pasos_str = lista_to_text(lista_pasos);
    char* lista_tiempos_str = lista_to_text(lista_tiempos);

    list_destroy(lista_pasos);
    list_destroy_and_destroy_elements(lista_tiempos, &free);

    char* receta_text = strdup("PASOS=");

    string_append(&receta_text, lista_pasos_str);
    string_append(&receta_text, "\nTIEMPO_PASOS=");
    string_append(&receta_text, lista_tiempos_str);
    
    free(lista_pasos_str);
    free(lista_tiempos_str);

    return receta_text;
}

char* get_pedido_text(t_pedido* pedido)
{
    log_info(logger, "Getting pedido text");

    char* pedido_text = strdup("ESTADO_PEDIDO=");
    
    char* estado_str = int_to_string(pedido->estado);
    
    string_append(&pedido_text, estado_str);

    free(estado_str);

    t_list* lista_platos = list_create();
    t_list* lista_cantidad_pedida = list_create();
    t_list* lista_cantidad_lista = list_create();

    for (int i = 0; i < list_size(pedido->platos); i++)
    {
        t_pedido_plato* plato = list_get(pedido->platos, i);
        
        char* cantidad_pedida = int_to_string(plato->cantidad_pedida);
        char* cantidad_lista = int_to_string(plato->cantidad_lista);

        list_add(lista_platos, plato->nombre_plato);
        list_add(lista_cantidad_pedida, cantidad_pedida);
        list_add(lista_cantidad_lista, cantidad_lista);
    }

    char* lista_platos_str = lista_to_text(lista_platos);
    char* lista_cantidad_pedida_str = lista_to_text(lista_cantidad_pedida);
    char* lista_cantidad_lista_str = lista_to_text(lista_cantidad_lista);

    list_destroy(lista_platos);
    list_destroy_and_destroy_elements(lista_cantidad_pedida, &free);
    list_destroy_and_destroy_elements(lista_cantidad_lista, &free);

    string_append(&pedido_text, "\nLISTA_PLATOS=");
    string_append(&pedido_text, lista_platos_str);
    string_append(&pedido_text, "\nCANTIDAD_PLATOS=");
    string_append(&pedido_text, lista_cantidad_pedida_str);
    string_append(&pedido_text, "\nCANTIDAD_LISTA=");
    string_append(&pedido_text, lista_cantidad_lista_str);

    free(lista_platos_str);
    free(lista_cantidad_pedida_str);
    free(lista_cantidad_lista_str);

    char* precio_string = int_to_string(pedido->precio);

    string_append(&pedido_text, "\nPRECIO=");
    string_append(&pedido_text, precio_string);

    free(precio_string);

    return pedido_text;
}

char* get_restaurante_text(t_restaurante* restaurante)
{
    char* restaurante_text = strdup("CANTIDAD_COCINEROS=");
    
    int cantidad_cocineros = list_size(restaurante->cocineros); 

    char* cantidad_cocineros_str = int_to_string(cantidad_cocineros);

    string_append(&restaurante_text, cantidad_cocineros_str);
    string_append(&restaurante_text, "\nPOSICION=[");
    
    free(cantidad_cocineros_str);
    
    char* posicion_x_str = int_to_string(restaurante->posicion->x);
    char* posicion_y_str = int_to_string(restaurante->posicion->y);
    
    string_append(&restaurante_text, posicion_x_str);
    string_append(&restaurante_text, ",");
    string_append(&restaurante_text, posicion_y_str);
    string_append(&restaurante_text, "]");
    
    free(posicion_x_str);
    free(posicion_y_str);

    t_list* lista_afinidades = list_create();


    for (int i = 0; i < cantidad_cocineros; i++)
    {
        t_cocinero* cocinero = list_get(restaurante->cocineros, i);

        if (cocinero->afinidad != NULL)
        {
            list_add(lista_afinidades, cocinero->afinidad);
        }
    }

    char* lista_afinidades_str = lista_to_text(lista_afinidades);

    list_destroy(lista_afinidades);

    string_append(&restaurante_text, "\nAFINIDAD_COCINEROS=");
    string_append(&restaurante_text, lista_afinidades_str);

    free(lista_afinidades_str);

    t_list* lista_platos = list_create();
    t_list* lista_precios = list_create();

    for (int i = 0; i < list_size(restaurante->platos); i++)
    {
        t_plato* plato = list_get(restaurante->platos, i);

        char* precio_str = int_to_string(plato->precio);

        list_add(lista_platos, plato->nombre);
        list_add(lista_precios, precio_str);
    }

    char* lista_platos_str = lista_to_text(lista_platos);
    char* lista_precios_str = lista_to_text(lista_precios);

    list_destroy(lista_platos);
    list_destroy_and_destroy_elements(lista_precios, &free);

    string_append(&restaurante_text, "\nPLATOS=");
    string_append(&restaurante_text, lista_platos_str);
    string_append(&restaurante_text, "\nPRECIO_PLATOS=");
    string_append(&restaurante_text, lista_precios_str);

    free(lista_platos_str);
    free(lista_precios_str);

    char* cantidad_hornos_str = int_to_string(restaurante->cantidadHornos);

    string_append(&restaurante_text, "\nCANTIDAD_HORNOS=");
    string_append(&restaurante_text, cantidad_hornos_str);

    free(cantidad_hornos_str);
    
    return restaurante_text;
}

int iniciar_directorios()
{
    log_info(logger, "Inicializando directorios File System");

    char* files_path = get_files_path();

    crear_directorio_si_no_existe(files_path);

    free(files_path);

    char* restaurantes_path = get_restaurantes_directory_path();

    crear_directorio_si_no_existe(restaurantes_path);

    free(restaurantes_path);

    char* recetas_path = get_recetas_directory_path();

    crear_directorio_si_no_existe(recetas_path);

    free(recetas_path);

    char* blocks_path = get_blocks_directory_path();

    crear_directorio_si_no_existe(blocks_path);

    free(blocks_path);

    log_info(logger, "Directorios File System inicializados OK");

    return 0;
}

char* get_files_path()
{
    char* path = strdup(config->punto_montaje);
    
    string_append(&path, FS_FILES_PATH);

    return path;
}

char* get_block_path(int block)
{
    char* block_str = int_to_string(block);
    char* directory_path = get_blocks_directory_path();
    
    char* path = strdup(directory_path);

    string_append(&path, block_str);
    string_append(&path, ".AFIP");

    free(block_str);
    free(directory_path);

    return path;
}

char* get_blocks_directory_path()
{
    char* path = strdup(config->punto_montaje);

    string_append(&path, FS_FILES_PATH);
    string_append(&path, FS_BLOCKS_PATH);

    return path;
}

char* get_receta_path(char* plato)
{
    char* directory_path = get_recetas_directory_path();
    
    char* path = strdup(directory_path);

    string_append(&path, plato);
    string_append(&path, ".AFIP");

    free(directory_path);

    return path;
}

char* get_recetas_directory_path()
{
    char* path = strdup(config->punto_montaje);

    string_append(&path, FS_FILES_PATH);
    string_append(&path, FS_RECETAS_PATH);

    return path;
}

char* get_restaurantes_directory_path()
{
    char* path = strdup(config->punto_montaje);

    string_append(&path, FS_FILES_PATH);
    string_append(&path, FS_RESTAURANTES_PATH);

    return path;
}

char* get_restaurante_directory_path(char* restaurante)
{
    char* directory_path = get_restaurantes_directory_path();
    
    char* path = strdup(directory_path);

    string_append(&path, restaurante);
    string_append(&path, "/");

    free(directory_path);

    return path;
}

char* get_restaurante_info_path(char* restaurante)
{
    char* directory_path = get_restaurante_directory_path(restaurante);

    char* path = strdup(directory_path);
    
    string_append(&path, "Info.AFIP");

    free(directory_path);

    return path;
}

char* get_restaurante_pedido_path(char* restaurante, uint32_t id_pedido)
{
    char* directory_path = get_restaurante_directory_path(restaurante);
    char* id_str = int_to_string(id_pedido);

    char* file_name = strdup("Pedido");
    string_append(&file_name, id_str);
    string_append(&file_name, ".AFIP");
    
    free(id_str);

    char* path = strdup(directory_path);
    string_append(&path, file_name);

    free(directory_path);
    free(file_name);

    return path;
}

int iniciar_bloques()
{
    log_info(logger, "Completando bloques");

    for (int i = 0; i < file_system_metadata->blocks; i++)
    {
        char* block_path = get_block_path(i);

        FILE* block = fopen(block_path, "r");

        if (block == NULL)
        {
            log_info(logger, "Bloque %s", block_path);
        
            block = fopen(block_path, "w");
        }

        fclose(block);

        free(block_path);
    }

    log_info(logger, "Bloques completados correctamente");
}
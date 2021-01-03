#include "fs_metadata.h"

int iniciar_semaforos()
{
    if (pthread_mutex_init(&mutex_bitmap, NULL) != 0) return -1;
    
    pthread_mutex_lock(&mutex_bitmap); //Este semáforo nace bloqueado y se desbloquea cuando se inicializa bitmap
    
    return 0;
}

int iniciar_constantes()
{
    FS_BLOCK_SIZE = 64;
    FS_BLOCKS = 8192;
    FS_MAGIC_NUMBER = strdup("AFIP");
    FS_METADATA_PATH = strdup("Metadata/");
    FS_METADATA_FILE_NAME = strdup("Metadata.AFIP");
    FS_METADATA_BITMAP_NAME = strdup("bitmap.bin");
    FS_FILES_PATH = strdup("Files/");
    FS_RECETAS_PATH = strdup("Recetas/");
    FS_RESTAURANTES_PATH = strdup("Restaurantes/");
    FS_BLOCKS_PATH = strdup("Blocks/");
}

int liberar_constantes()
{
    free(FS_MAGIC_NUMBER);
    free(FS_METADATA_PATH);
    free(FS_METADATA_FILE_NAME);
    free(FS_METADATA_BITMAP_NAME);
    free(FS_FILES_PATH);
    free(FS_RECETAS_PATH);
    free(FS_RESTAURANTES_PATH);
    free(FS_BLOCKS_PATH);
}

int finalizar_semaforos()
{
    pthread_mutex_destroy(&mutex_bitmap);

    return 0;
}

int crear_archivo_metadata()
{
	log_info(logger, "Creando archivo de Metadata AFIP");

    char* directoryPath = get_metadata_directory_path();

    crear_directorio_si_no_existe(directoryPath);

    free(directoryPath);

    char* filePath = get_metadata_file_path();

	log_info(logger, "Creando archivo %s", filePath);

    char* block_size_str = int_to_string(FS_BLOCK_SIZE);
    char* blocks_number_str = int_to_string(FS_BLOCKS);

    char* metadataText = strdup("BLOCK_SIZE=");
    string_append(&metadataText, block_size_str);
    string_append(&metadataText, "\nBLOCKS=");
    string_append(&metadataText, blocks_number_str);
    string_append(&metadataText, "\nMAGIC_NUMBER=");
    string_append(&metadataText, FS_MAGIC_NUMBER);

    free(block_size_str);
    free(blocks_number_str);

    FILE* metadataFile = fopen(filePath, "w+");
    
    fprintf(metadataFile, metadataText);

    fclose(metadataFile);

    free(filePath);
    free(metadataText); 
}

char* get_metadata_file_path()
{
    char* path = strdup(config->punto_montaje);

    string_append(&path, FS_METADATA_PATH);
    string_append(&path, FS_METADATA_FILE_NAME);

    return path;
}

char* get_metadata_bitmap_path()
{
    char* path = strdup(config->punto_montaje);

    string_append(&path, FS_METADATA_PATH);
    string_append(&path, FS_METADATA_BITMAP_NAME);

    return path;
}

char* get_metadata_directory_path()
{
    char* path = strdup(config->punto_montaje);
    
    string_append(&path, FS_METADATA_PATH);

    return path;
}

int iniciar_metadata_config()
{
    log_info(logger, "Inicializando Metadata AFIP");

    char* metadataPath = get_metadata_file_path();

    log_info(logger, "Buscando archivo de metadata en %s", metadataPath);

    FILE* metadataFile = fopen(metadataPath, "r");

    t_config* metadata_config;

    if (metadataFile == NULL)
    {
        log_info(logger, "No se encontró archivo metadata.afip, se creará el mismo");
    
	    crear_archivo_metadata();

        metadata_config = config_create(metadataPath);
    }
    else
    {
        log_info(logger, "Se encontró archivo metadata.afip, comprobando magic number");

        metadata_config = config_create(metadataPath);

        if (metadata_config == NULL)
        {
            log_error(logger, "Archivo Metadata.AFIP encontrado es inválido");
        }

        log_info(logger, "Metadata config instanciada");

        char* magic_number = strdup(config_get_string_value(metadata_config,"MAGIC_NUMBER"));

        log_info(logger, "Magic number: %s", magic_number);

        if(strcmp(FS_MAGIC_NUMBER, magic_number) == 0)
        {
            log_info(logger, "Magic number OK");
        }
        else
        {
            log_info(logger, "No se comprobó exitosamente magic number. Creando nuevo archivo Metadata Afip");

            config_destroy(metadata_config);
            remove(metadataPath);

            crear_archivo_metadata();

            metadata_config = config_create(metadataPath);
        }

        free(magic_number);
    }

    free(metadataPath);

    file_system_metadata = malloc(sizeof(t_metadata_config));
	file_system_metadata->block_size = config_get_int_value(metadata_config, "BLOCK_SIZE");
	file_system_metadata->blocks = config_get_int_value(metadata_config, "BLOCKS");
	file_system_metadata->magic_number = strdup(config_get_string_value(metadata_config, "MAGIC_NUMBER"));

    config_destroy(metadata_config);

    log_info(logger, "Metadata AFIP inicializada correctamente");

    log_info(logger, "Block size: %d", file_system_metadata->block_size);
    log_info(logger, "Blocks: %d", file_system_metadata->blocks);
    log_info(logger, "Magic number: %s", file_system_metadata->magic_number);

    return 0;
}

int iniciar_bitmap()
{
    log_info(logger, "Inicializando bitmap");

    int bloques = file_system_metadata->blocks; 
    size_t size = bloques / 8;

    bit_array = malloc(size);

    char* bitmapPath = get_metadata_bitmap_path();

    FILE* bitmapFile = fopen(bitmapPath, "r");

    if (bitmapFile != NULL)
    {
        log_info(logger, "Leyendo bitmap");

        fread(bit_array, size, 1, bitmapFile);
        fclose(bitmapFile);

        log_info(logger, "Bitmap leído correctamente.");
    }    

    bitmap = bitarray_create_with_mode(bit_array, size, LSB_FIRST);

    if (bitmapFile == NULL)
    {
        log_info(logger, "No se encontró archivo %s, instanciando bitmap vacío en memoria.", bitmapPath);
    
        for (int i = 0; i < file_system_metadata->blocks; i++)
        {
            bitarray_clean_bit(bitmap, i); 
        }

        log_info(logger, "Bitmap limpiado correctamente");
    }

    pthread_mutex_unlock(&mutex_bitmap);

    log_info(logger, "Bitmap inicializado ok");

    free(bitmapPath);

    return 0;
}

int guardar_bitmap() //Sólo al finalizar file_system
{
    char* bitmapPath = get_metadata_bitmap_path();

    log_info(logger, "Guardando bitmap en %s", bitmapPath);
    
    int bloques = file_system_metadata->blocks; 
    size_t size = bloques / 8;

    FILE* bitmapFile = fopen(bitmapPath, "w+");

    log_info(logger, "Archivo abierto");

    fwrite(bit_array, size, 1, bitmapFile);

    fclose(bitmapFile);

    log_info(logger, "Bitmap guardado");

    free(bitmapPath);

    return 0;
}

int pedir_bloque()
{
    pthread_mutex_lock(&mutex_bitmap);

    for (int i = 0; i < file_system_metadata->blocks; i++)
    {
        if (!bitarray_test_bit(bitmap, i))
        {
            bitarray_set_bit(bitmap, i);
            
            guardar_bitmap();

            pthread_mutex_unlock(&mutex_bitmap);

            return i;
        }
    }

    log_error(logger, "Bitmap lleno");

    return -1;
}

void liberar_bloque(int bloque)
{
    pthread_mutex_lock(&mutex_bitmap);
    
    bitarray_clean_bit(bitmap, bloque);

    guardar_bitmap();

    pthread_mutex_unlock(&mutex_bitmap);
}
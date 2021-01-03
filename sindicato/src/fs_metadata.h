#ifndef SRC_FS_METADATA_H
#define SRC_FS_METADATA_H

//Utiles de Sindicato
#include "utils.h"

#include <commons/bitarray.h>

int FS_BLOCK_SIZE;
int FS_BLOCKS;
char* FS_MAGIC_NUMBER;
char* FS_METADATA_PATH;
char* FS_METADATA_FILE_NAME;
char* FS_METADATA_BITMAP_NAME;
char* FS_FILES_PATH;
char* FS_RECETAS_PATH;
char* FS_RESTAURANTES_PATH;
char* FS_BLOCKS_PATH;

int crear_archivo_metadata(void);
int iniciar_metadata_config(void);
int iniciar_bitmap(void);

char* get_metadata_file_path(void);
char* get_metadata_bitmap_path(void);
char* get_metadata_directory_path(void);

typedef struct {
	int block_size;
	int blocks;
	char* magic_number; 
} t_metadata_config;

t_metadata_config* file_system_metadata;

pthread_mutex_t mutex_bitmap;

t_bitarray* bitmap;
void* bit_array;


#endif

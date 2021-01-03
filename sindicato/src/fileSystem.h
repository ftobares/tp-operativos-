#ifndef SRC_FILESYSTEM_H
#define SRC_FILESYSTEM_H

#include "fs_metadata.h"
#include "utils.h"

#include <dirent.h>
#include <semaphore.h>

typedef struct {
	int size;
	uint32_t initial_block; 
} t_file_info;

int runFileSystem(void);
int inicializar(void);
int inicializar_directorios(void);

t_restaurante* get_restaurante(char* nombre);

int write_file_content(char* file_content, uint32_t initial_block);
char* read_file(char* info_path);
int read_file_content(char* file_content, t_file_info* file_info);

t_file_info* get_file_info(char* info_path);

char* get_file_metadata_text(int size, int initial_block);
char* get_receta_text(t_receta* receta);
char* get_pedido_text(t_pedido* pedido);
char* get_restaurante_text(t_restaurante* restaurante);

char* get_block_full_string(int block);

char* get_files_path(void);

char* get_blocks_directory_path(void);
char* get_block_path(int block);

char* get_recetas_directory_path(void);
char* get_receta_path(char* plato);

char* get_restaurantes_directory_path(void);
char* get_restaurante_directory_path(char* restaurante);
char* get_restaurante_info_path(char* restaurante);
char* get_restaurante_pedido_path(char* restaurante, uint32_t id_pedido);

#endif
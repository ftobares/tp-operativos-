#ifndef MODELS_H_
#define MODELS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <commons/collections/list.h>

typedef struct{
	int x;
	int y;
} t_posicion;

typedef struct
{
	int id;
	char* nombre;
	int precio;
} t_plato;

typedef struct
{
    int id;
    char* afinidad;
} t_cocinero;

typedef struct
{
    int id;
    char* nombre;
    t_posicion* posicion;
    t_list* cocineros;
    t_list* platos;
    int cantidadHornos;
} t_restaurante;

typedef struct
{
	char* nombre;
	int tiempo;
} t_paso;

typedef struct
{
	char* plato;
	t_list* pasos;
} t_receta;

typedef enum
{
	PEDIDO_PENDIENTE,
	PEDIDO_CONFIRMADO,
	PEDIDO_TERMINADO
} estado_pedido;

typedef struct
{
	uint32_t id;
	char* nombre_restaurante;
	t_list* platos;
	estado_pedido estado;
	uint32_t precio;
} t_pedido;

typedef struct
{
	char* nombre_plato;
	uint32_t cantidad_pedida;
	uint32_t cantidad_lista;
} t_pedido_plato;


//GENERAL
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/* POR LAS DUDAS LO DEJAMOS COMENTADO
typedef struct{
	int _bool_long;
	int _bool;				//	0 --> Generacion exitosa //	1 --> Error
}tBool;

typedef struct{
	int _id_long;
	int _id;
}tId;
*/
//%%%%	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//															PEDIDO

//		LISTA DE PLATOS COMO LISTA	//

typedef struct{
	int id_long;
	void* id;
	int nombre_restaurante_long;
	char* nombre_restaurante;
	int lista_platos_long;
	t_list* lista_platos;
	int estado_precio_long;
	int estado;			//Estado = 1 => Listo // Estado = 2 => Sin terminar
	int precio;
} Pedido;

typedef struct
{
	int sizeofinfo; // = sizeof(info);
	Pedido info;
} paq_Pedido;

/*typedef struct{
	int lista_long;
	l_Plato lista;
}paquete_l_Plato;

typedef struct{
	int plato_info_long;
	p_Plato plato_info;
	int nodo_siguiente_long;
	l_Plato nodo_siguiente;
}l_Plato;

typedef struct{
	int plato_long;
	struct Plato plato;
}p_Plato;*/

typedef struct{
	int comida_long;		//Receta
	char* comida;
	int total_listo_long;
	int cant_total;
	int cant_listo;
} Plato;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//																RESTAURANTE

typedef struct{
	int info_long;			//Header
	t_restaurante info;			//Payload
}paq_Restaurante;

//		Usando platos como un array
/*
typedef struct{
	int id_long;
	tId id;
	int nombre_long
	p_nombre_restaurante nombre;
	int posicion_long;
	tPosicion posicion;
	int lista_platos_long;
	paquete_l_Plato lista_platos; 		//Las recetas que prepara el restaurante
	int precios_long:
	tLista precios;						//Lista de precios
///	int cocineros_hornos_long;		//TODO revisar estos sizeof
	int cant_cocineros;
	int cant_hornos;
}Restaurante;

typedef struct{
	int nombre_restaurante_long;
	char* nombre_restaurante;
}p_nombre_restaurante;*/

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//RECETA

typedef struct
{
	int pasos_long;
	t_list* pasos;
	int tiempo_long;
	t_list* tiempo;
}Receta;

typedef struct
{
	int info_long;
	Receta info;
}paq_Receta;

/*
typedef struct{
	int info_long;
	void* info;
	int siguiente_long;
	lista siguiente;
}lista;

typedef struct{
	int header;
	lista buffer;
}tLista;
*/
#endif

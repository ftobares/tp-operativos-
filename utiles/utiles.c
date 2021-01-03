#include "utiles.h"

// Lee una instrucción por la terminal/consola
char* leerlinea()
{	
	char* leido = string_new();

	leido = readline(">");
	string_trim(&leido);

	// Validacion para que no metan una cadena vacía o solo espacios
	while(string_equals_ignore_case(leido,"")){
		free(leido);
		//fflush(stdin);
		leido = readline(">");
	//	gets(leido);
		string_trim(&leido);
	}
	
	printf("\n");
  add_history(leido);
	return leido;
}

char** leerInstruccion()
{
	char* linea = leerlinea(); 
	char** split = string_split(linea," ");

	free(linea);

	return split;
}

/*@NAME: parseLista
 *@DESC: RECIBE UN CHAR* [e1,e2,...,eN] CON LOS ELEMENTOS 
 			DEVUELVE UN t_list* de char* */
t_list* parseLista(char* listaString)
{
	t_list* lista = list_create();

	if (strlen(listaString) == 2) return lista; //En este caso sería [] lista vacía

	int comas = 0;

	//Cuento apariciones de ','
	for (int i = 0; i < strlen(listaString); i++)
		if (listaString[i] == ',')
			comas++;

	// Quitamos [ ] del string inicial
	char* listaLimpia = string_substring(listaString, 1, strlen(listaString)-2);

	// Hacemos un split por separando por comas
	char** split;

	split = string_split(listaLimpia, ",");


	for (int i = 0; i <= comas; i++) list_add(lista, split[i]);
		
	free(split);
	free(listaLimpia);

	return lista;
}

/*@NAME: lista_to_text
 *@DESC: RECIBE UN t_list DE char* Y DEVUELVE UN char* CON FORMATO [e1,e2,...,eN]*/
char* lista_to_text(t_list* lista)
{
	char* text = malloc(sizeof(char) * 1);

	strcpy(text, "[");

	for (int i = 0; i < list_size(lista); i++)
	{
		string_append(&text, list_get(lista,i));
		
		if (i + 1 < list_size(lista))
			string_append(&text, ",");
	}

	string_append(&text,"]");

	return text;
}

t_posicion* parsePosicion(char* stringLista)
{
	t_list* lista = parseLista(stringLista);

	t_posicion* posicion = malloc(sizeof(t_posicion));

	posicion->x = atoi(list_get(lista, 0));
	posicion->y = atoi(list_get(lista, 1));

	list_destroy_and_destroy_elements(lista, &free);

	return posicion;
}

// Parsea una lista de posiciones en formato [X|Y,X|Y,X|Y]
t_list* parseListaPosiciones(char* posiciones){

	// Elimino primer y ultimo caracter (corchetes)
	char* cadenaLimpia =  string_substring(posiciones,1,strlen(posiciones)-2);
	int cantidadRepartidores = 0;

	//Cuento apariciones de ','
	for(int i = 0; i < strlen(cadenaLimpia); i++){
		if (cadenaLimpia[i] == ',')
			cantidadRepartidores++;
	}
	cantidadRepartidores++;

	char** split = string_split(cadenaLimpia, ",");


	t_list* lista = list_create();

	for (int i = 0; i < cantidadRepartidores; i++){
		char** pipe_split = string_split(split[i],"|");
		t_posicion* posicion_repartidor = malloc(sizeof(t_posicion));
		posicion_repartidor->x = atoi(pipe_split[0]);
		posicion_repartidor->y = atoi(pipe_split[1]);
		list_add(lista,posicion_repartidor);
		free(pipe_split[1]);
		free(pipe_split[0]);
		free(pipe_split);
	}

	// Libero la memoria usada en el primer split
	for(int j=cantidadRepartidores; j>0; j--){
		free(split[j-1]);
	}
	free(cadenaLimpia);
	free(split);
	return lista;
}

// Dado un string con el comando devuelve su representación en enum.
enum API obtenerComando(char* comando){

	if(string_equals_ignore_case(comando,"CONSULTAR_RESTAURANTES"))
		return CONSULTAR_RESTAURANTES;
	else if(string_equals_ignore_case(comando,"SELECCIONAR_RESTAURANTE"))
		return SELECCIONAR_RESTAURANTE;
	else if(string_equals_ignore_case(comando,"OBTENER_RESTAURANTE"))
		return OBTENER_RESTAURANTE;
	else if(string_equals_ignore_case(comando,"CONSULTAR_PLATOS"))
		return CONSULTAR_PLATOS;
	else if(string_equals_ignore_case(comando,"CREAR_PEDIDO"))
		return CREAR_PEDIDO;
	else if(string_equals_ignore_case(comando,"GUARDAR_PEDIDO"))
		return GUARDAR_PEDIDO;
	else if(string_equals_ignore_case(comando,"ANIADIR_PLATO"))
		return ANIADIR_PLATO;
	else if(string_equals_ignore_case(comando,"GUARDAR_PLATO"))
		return GUARDAR_PLATO;
	else if(string_equals_ignore_case(comando,"CONFIRMAR_PEDIDO"))
		return CONFIRMAR_PEDIDO;
	else if(string_equals_ignore_case(comando,"PLATO_LISTO"))
		return PLATO_LISTO;
	else if(string_equals_ignore_case(comando,"CONSULTAR_PEDIDO"))
		return CONSULTAR_PEDIDO;
	else if(string_equals_ignore_case(comando,"OBTENER_PEDIDO"))
		return OBTENER_PEDIDO;
	else if(string_equals_ignore_case(comando,"FINALIZAR_PEDIDO"))
		return FINALIZAR_PEDIDO;
	else if(string_equals_ignore_case(comando,"TERMINAR_PEDIDO"))
		return TERMINAR_PEDIDO;
	else if(string_equals_ignore_case(comando,"OBTENER_RECETA"))
		return OBTENER_RECETA;
	else if(string_equals_ignore_case(comando,"EXIT"))
		return EXIT;
	else
		return ERROR;
}

// Recibe un string y valida si puede ser casteado a int. 
// En caso de que si, devuelve true y asigna el valor numero a result
// En caso de que no, devuelve false
bool try_string_to_int(char* str, int* result) {
	int str_length = string_length(str);
	bool is_number = true;

	int i = 0;
	while(is_number && i< str_length){
		if (isdigit(str[i]) == 0) {
			is_number = false;
		}
		i++;
	}

	if (is_number) {
		*result = atoi(str);
	}
	return is_number;
}

char* append(char* comando, char* argumento){
	char* string = string_new();
	string_append(&string, comando);
	string_append(&string, " ");
	string_append(&string, argumento);
	string_append(&string,"");
}

// Convierte un número entero que representa un puerto a una cadena
char* intPortToString(int port){
	char* string = malloc(5);
	sprintf(string,"%d",port);
	return string;
}

char* int_to_string(int number)
{
	char* big_string = malloc(32);
	sprintf(big_string,"%d",number);

	char* string = malloc(strlen(big_string) + 1);
	strcpy(string, big_string);

	free(big_string);

	return string;
}

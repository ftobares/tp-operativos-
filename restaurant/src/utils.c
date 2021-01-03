#include "utils.h"

void useCaseDefault();
void useCaseCatedra();
void useCaseError();

t_obtener_restaurante_respuesta_s* cargarRestauranteUseCase(char* nombreRestaurante);
t_obtener_restaurante_respuesta_s* cargarRestauranteDefault(char* nombreRestaurante);

t_list* crearRecetaAsadoJugoso();
t_list* crearRecetaAsadoSeco();
t_list* crearRecetaChoripan();
t_list* crearRecetaTortillaDePapa();
void freeMemPasoReceta(t_paso* paso);

void leerConfiguracion() {
	config = cargar_configuracion(CONFIG_PATH, RESTAURANTE);
}

void empaquetarNombreRestaurante(t_nombre_restaurante_s* paqueteAEnviar) {
	paqueteAEnviar->nombre_restaurante = config->nombre_restaurante;
	paqueteAEnviar->nombre_restaurante_size = strlen(
			paqueteAEnviar->nombre_restaurante) + 1;
}

void armarHeader(t_header* header, t_tipo_mensaje tipoMensaje, int32_t size) {
	header->msj_type = tipoMensaje;
	header->size = size;
}

void liberarConfig(t_restaurante_config* config) {
	log_info(logger, "Libero Config");

	free(config->algoritmo_planificacion);
	free(config->archivo_log);
	free(config->ip_app);
	free(config->ip_sindicato);
	free(config->nombre_restaurante);
	free(config->puerto_app);
	free(config->puerto_escucha);
	free(config->puerto_sindicato);
	free(config);
}

int calcularTamanioRestaurant(t_obtener_restaurante_respuesta_s* paqueteAEnviar) {

	int size = 0;

	for (int i = 0; i < paqueteAEnviar->cantCocineros; i++) {
		t_cocinero_s* cocinero = (t_cocinero_s*) list_get(
				paqueteAEnviar->cocineros, i);
		size += sizeof(uint32_t) * 2 + cocinero->afinidad_size;
	}

	for (int j = 0; j < paqueteAEnviar->cantPlatos; j++) {
		t_plato_s* plato = (t_plato_s*) list_get(paqueteAEnviar->platos, j);
		size += sizeof(uint32_t) * 3 + plato->nombre_size;
	}

	size += sizeof(uint32_t) * 7 + paqueteAEnviar->nombre_size;

	return size;

}

void _liberarCocinero(t_cocinero_s* cocinero) {
	free(cocinero->afinidad);
	free(cocinero);
}

void _liberarPlato(t_plato_s* plato) {
	free(plato->nombre);
	free(plato);
}

void liberarRestaurante(t_obtener_restaurante_respuesta_s* restaurante) {

	log_info(logger, "Libero Restaurante");

	list_destroy_and_destroy_elements(restaurante->cocineros, _liberarCocinero);
	list_destroy_and_destroy_elements(restaurante->platos, _liberarPlato);
//	free(restaurante->nombre);  --- Invalid free() / delete / delete[] / realloc()
	free(restaurante->posicion);
	free(restaurante);
}

uint32_t obtenerIdPedido(){
	pthread_mutex_lock(&mutexCantidadPedidos);
	uint32_t pedidoId =  __sync_fetch_and_add(&CANTIDAD_PEDIDOS, 1);
	pthread_mutex_unlock(&mutexCantidadPedidos);

	return pedidoId;
}

void setCantidadPedidos(uint32_t cantidadPedidos){
	CANTIDAD_PEDIDOS = cantidadPedidos;
}

/*********************** PARA UNIT TEST ***********************/

void testPlanificador(char* useCase) {
	if (string_equals_ignore_case(useCase, "default")) {
		useCaseDefault();
	} else if (string_equals_ignore_case(useCase, "catedra")) {
		useCaseCatedra();
	} else if (string_equals_ignore_case(useCase, "error")) {
		useCaseError();
	}
}

t_obtener_restaurante_respuesta_s* getMockRestaurante(char* nombreRestaurante) {
	if (string_equals_ignore_case(nombreRestaurante, "SNOOP")) {
		return cargarRestauranteDefault(nombreRestaurante);
	} else if (string_equals_ignore_case(nombreRestaurante, "ElParrillon")) {
		return cargarRestauranteUseCase(nombreRestaurante);
	}
}

t_obtener_restaurante_respuesta_s* cargarRestauranteUseCase(char* nombreRestaurante) {
	// Mock de int
	int cantidadDeHornos = 1;
	int cantidadPedidos = 5;

	/***************************************************
	 Lista de Platos
	 ***************************************************/
	t_list* listaPlatos = list_create();
	int listaPlatosSize = 0;

	// Defino un plato
	t_plato_s* plato1 = malloc(sizeof(t_plato_s));
	plato1->id = 1;
	plato1->precio = 180;
	char* asadoJugoso = string_new();
	string_append(&asadoJugoso, "AsadoJugoso");
	plato1->nombre = asadoJugoso;
	plato1->nombre_size = strlen(plato1->nombre) + 1;
	listaPlatosSize += sizeof(uint32_t) * 3 + plato1->nombre_size;

	list_add(listaPlatos, plato1);

	// Defino un segundo plato
	t_plato_s* plato2 = malloc(sizeof(t_plato_s));
	plato2->id = 2;
	plato2->precio = 200;
	char* asadoSeco = string_new();
	string_append(&asadoSeco, "AsadoSeco");
	plato2->nombre = asadoSeco;
	plato2->nombre_size = strlen(plato2->nombre) + 1;

	listaPlatosSize += sizeof(uint32_t) * 3 + plato2->nombre_size;

	list_add(listaPlatos, plato2);

	// Defino un segundo plato
	t_plato_s* plato3 = malloc(sizeof(t_plato_s));
	plato3->id = 3;
	plato3->precio = 100;
	char* choripan = string_new();
	string_append(&choripan, "Choripan");
	plato3->nombre = choripan;
	plato3->nombre_size = strlen(plato3->nombre) + 1;

	listaPlatosSize += sizeof(uint32_t) * 3 + plato3->nombre_size;

	list_add(listaPlatos, plato3);

	// Defino un segundo plato
	t_plato_s* plato4 = malloc(sizeof(t_plato_s));
	plato4->id = 4;
	plato4->precio = 150;
	char* tortilla = string_new();
	string_append(&tortilla, "TortillaDePapa");
	plato4->nombre = tortilla;
	plato4->nombre_size = strlen(plato4->nombre) + 1;

	listaPlatosSize += sizeof(uint32_t) * 3 + plato4->nombre_size;

	list_add(listaPlatos, plato4);

	/***************************************************
	 Lista de Cocineros
	 ***************************************************/
	t_list* listaCocineros = list_create();
	int listaCocinerosSize = 0;

	t_cocinero_s* cocinero1 = malloc(sizeof(t_cocinero_s));
	cocinero1->id = 1;
	char* afinidadAsadoJugoso = string_new();
	string_append(&afinidadAsadoJugoso, "AsadoJugoso");
	cocinero1->afinidad = afinidadAsadoJugoso;
	cocinero1->afinidad_size = strlen(afinidadAsadoJugoso) + 1;

	listaCocinerosSize += sizeof(uint32_t) * 2 + cocinero1->afinidad_size;

	list_add(listaCocineros, cocinero1);

	t_cocinero_s* cocinero2 = malloc(sizeof(t_cocinero_s));
	cocinero2->id = 2;
	char* sinAfinidad = string_new();
	string_append(&sinAfinidad, "SIN_AFINIDAD");
	cocinero2->afinidad = sinAfinidad;
	cocinero2->afinidad_size = strlen(sinAfinidad) + 1;

	listaCocinerosSize += sizeof(uint32_t) * 2 + cocinero2->afinidad_size;

	list_add(listaCocineros, cocinero2);

	/***************************************************
	 Posicion
	 ***************************************************/
	t_posicion_s* posicion = malloc(sizeof(t_posicion_s));
	posicion->x = 5;
	posicion->y = 5;
	//	int posicionSize = sizeof(uint32_t) * 2;

	/***************************************************
	 Armo respuesta final
	 ***************************************************/

	t_obtener_restaurante_respuesta_s* respuesta = malloc(
			sizeof(t_obtener_restaurante_respuesta_s));
	respuesta->cantidadHornos = cantidadDeHornos;
	respuesta->cantCocineros = list_size(listaCocineros);
	respuesta->cocineros = listaCocineros;
	respuesta->nombre = nombreRestaurante;
	respuesta->nombre_size = strlen(nombreRestaurante) + 1;
	respuesta->cantPlatos = list_size(listaPlatos);
	respuesta->platos = listaPlatos;
	respuesta->posicion = posicion;
	respuesta->id = 1;

	log_info(logger, "Datos de restaurante recibidos: ");

	log_info(logger, "ID: %i", respuesta->id);
	log_info(logger, "Hornos: %i", respuesta->cantidadHornos);
	log_info(logger, "Cocineros: %i", respuesta->cantCocineros);
	log_info(logger, "Platos: %i", respuesta->cantPlatos);
	log_info(logger, "Nombre: %s", respuesta->nombre);
	log_info(logger, "Nombre Size: %i", respuesta->nombre_size);

	for (int i = 0; i < respuesta->cantCocineros; i++) {
		log_info(logger, "Cocinero %i", i);
		t_cocinero_s* cocinero = (t_cocinero_s*) list_get(respuesta->cocineros,
				i);
		log_info(logger, "ID: %i", cocinero->id);
		log_info(logger, "Afinidad Size: %i", cocinero->afinidad_size);
		log_info(logger, "Afinidad: %s", cocinero->afinidad);
	}

	for (int j = 0; j < respuesta->cantPlatos; j++) {
		log_info(logger, "Plato %i", j);
		t_plato_s* plato = (t_plato_s*) list_get(respuesta->platos, j);
		log_info(logger, "ID: %i", plato->id);
		log_info(logger, "Nombre Size: %i", plato->nombre_size);
		log_info(logger, "Nombre: %s", plato->nombre);
		log_info(logger, "Precio: %i", plato->precio);
	}

	return respuesta;
}

t_obtener_restaurante_respuesta_s* cargarRestauranteDefault(char* nombreRestaurante) {

	// Mock de int
	int cantidadDeHornos = 2;
	int cantidadPedidos = 3;

	/***************************************************
	 Lista de Platos
	 ***************************************************/
	t_list* listaPlatos = list_create();
	int listaPlatosSize = 0;

	// Defino un plato
	t_plato_s* plato1 = malloc(sizeof(t_plato_s));
	plato1->id = 1;
	plato1->precio = 100;
	char* milanesa = string_new();
	string_append(&milanesa, "Milanesa");
	plato1->nombre = milanesa;
	plato1->nombre_size = strlen(plato1->nombre) + 1;
	listaPlatosSize += sizeof(uint32_t) * 3 + plato1->nombre_size;

	list_add(listaPlatos, plato1);

	// Defino un segundo plato
	t_plato_s* plato2 = malloc(sizeof(t_plato_s));
	plato2->id = 2;
	plato2->precio = 250;
	char* empanadas = string_new();
	string_append(&empanadas, "Empanadas");
	plato2->nombre = empanadas;
	plato2->nombre_size = strlen(plato2->nombre) + 1;

	listaPlatosSize += sizeof(uint32_t) * 3 + plato2->nombre_size;

	list_add(listaPlatos, plato2);

	/***************************************************
	 Lista de Cocineros
	 ***************************************************/
	t_list* listaCocineros = list_create();
	int listaCocinerosSize = 0;

	t_cocinero_s* cocinero1 = malloc(sizeof(t_cocinero_s));
	cocinero1->id = 1;
	char* afinidadMila = string_new();
	string_append(&afinidadMila, "Milanesa");
	cocinero1->afinidad = afinidadMila;
	cocinero1->afinidad_size = strlen(afinidadMila) + 1;

	listaCocinerosSize += sizeof(uint32_t) * 2 + cocinero1->afinidad_size;

	list_add(listaCocineros, cocinero1);

	t_cocinero_s* cocinero2 = malloc(sizeof(t_cocinero_s));
	cocinero2->id = 2;
	char* sinAfinidad = string_new();
	string_append(&sinAfinidad, "SIN_AFINIDAD");
	cocinero2->afinidad = sinAfinidad;
	cocinero2->afinidad_size = strlen(sinAfinidad) + 1;

	listaCocinerosSize += sizeof(uint32_t) * 2 + cocinero2->afinidad_size;

	list_add(listaCocineros, cocinero2);

	/***************************************************
	 Posicion
	 ***************************************************/
	t_posicion_s* posicion = malloc(sizeof(t_posicion_s));
	posicion->x = 4;
	posicion->y = 10;
	//	int posicionSize = sizeof(uint32_t) * 2;

	/***************************************************
	 Armo respuesta final
	 ***************************************************/

	t_obtener_restaurante_respuesta_s* respuesta = malloc(
			sizeof(t_obtener_restaurante_respuesta_s));
	respuesta->cantidadHornos = cantidadDeHornos;
	respuesta->cantCocineros = list_size(listaCocineros);
	respuesta->cocineros = listaCocineros;
	respuesta->nombre = nombreRestaurante;
	respuesta->nombre_size = strlen(nombreRestaurante) + 1;
	respuesta->cantPlatos = list_size(listaPlatos);
	respuesta->platos = listaPlatos;
	respuesta->posicion = posicion;
	respuesta->id = 1;

	log_info(logger, "Datos de restaurante enviados: ");

	log_info(logger, "ID: %i", respuesta->id);
	log_info(logger, "Hornos: %i", respuesta->cantidadHornos);
	log_info(logger, "Cocineros: %i", respuesta->cantCocineros);
	log_info(logger, "Platos: %i", respuesta->cantPlatos);
	log_info(logger, "Nombre: %s", respuesta->nombre);
	log_info(logger, "Nombre Size: %i", respuesta->nombre_size);

	for (int i = 0; i < respuesta->cantCocineros; i++) {
		log_info(logger, "Cocinero %i", i);
		t_cocinero_s* cocinero = (t_cocinero_s*) list_get(respuesta->cocineros,
				i);
		log_info(logger, "ID: %i", cocinero->id);
		log_info(logger, "Afinidad Size: %i", cocinero->afinidad_size);
		log_info(logger, "Afinidad: %s", cocinero->afinidad);
	}

	for (int j = 0; j < respuesta->cantPlatos; j++) {
		log_info(logger, "Plato %i", j);
		t_plato_s* plato = (t_plato_s*) list_get(respuesta->platos, j);
		log_info(logger, "ID: %i", plato->id);
		log_info(logger, "Nombre Size: %i", plato->nombre_size);
		log_info(logger, "Nombre: %s", plato->nombre);
		log_info(logger, "Precio: %i", plato->precio);
	}

	return respuesta;
}

/***************** INICIO USE CASE DEFAULT *****************/
void useCaseDefault() {
	// PEDIDO 1 - PLATO 1 //

	t_list* pasos1 = list_create();

	t_paso* paso1 = malloc(sizeof(t_paso));
	paso1->nombre = strdup("Pelar");
	paso1->tiempo = 2;
	list_add(pasos1, paso1);

	t_paso* paso2 = malloc(sizeof(t_paso));
	paso2->nombre = strdup("Hervir");
	paso2->tiempo = 3;
	list_add(pasos1, paso2);

	t_paso* paso3 = malloc(sizeof(t_paso));
	paso3->nombre = strdup("Hornear");
	paso3->tiempo = 4;
	list_add(pasos1, paso3);

	t_pcb* tortilla = crearPCB(1, 1, "TortillaDePapa", pasos1);

	// PEDIDO 2 - PLATO 1 //

	t_list* pasos2 = list_create();

	t_paso* paso21 = malloc(sizeof(t_paso));
	paso21->nombre = strdup("Limpiar");
	paso21->tiempo = 1;
	list_add(pasos2, paso21);

	t_paso* paso22 = malloc(sizeof(t_paso));
	paso22->nombre = strdup("Condimentar");
	paso22->tiempo = 1;
	list_add(pasos2, paso22);

	t_paso* paso23 = malloc(sizeof(t_paso));
	paso23->nombre = strdup("Hornear");
	paso23->tiempo = 2;
	list_add(pasos2, paso23);

	t_paso* paso24 = malloc(sizeof(t_paso));
	paso24->nombre = strdup("Reposar");
	paso24->tiempo = 3;
	list_add(pasos2, paso24);

	t_pcb* asadoSeco = crearPCB(2, 1, "AsadoSeco", pasos2);

	// PEDIDO 1 - PLATO 2 //

	t_list* pasos3 = list_create();

	t_paso* paso31 = malloc(sizeof(t_paso));
	paso31->nombre = strdup("Condimentar");
	paso31->tiempo = 3;
	list_add(pasos3, paso31);

	t_paso* paso32 = malloc(sizeof(t_paso));
	paso32->nombre = strdup("Reposar");
	paso32->tiempo = 2;
	list_add(pasos3, paso32);

	t_paso* paso33 = malloc(sizeof(t_paso));
	paso33->nombre = strdup("Hornear");
	paso33->tiempo = 4;
	list_add(pasos3, paso33);

	t_pcb* asadoJugoso = crearPCB(1, 2, "AsadoJugoso", pasos3);

	encolarNuevoPlato(tortilla);
	encolarNuevoPlato(asadoSeco);
	encolarNuevoPlato(asadoJugoso);

	free(paso1->nombre);
	free(paso1);
	free(paso2->nombre);
	free(paso2);
	free(paso3->nombre);
	free(paso3);
	list_destroy(pasos1);

	free(paso31->nombre);
	free(paso31);
	free(paso32->nombre);
	free(paso32);
	free(paso33->nombre);
	free(paso33);
	list_destroy(pasos3);

	free(paso21->nombre);
	free(paso21);
	free(paso22->nombre);
	free(paso22);
	free(paso23->nombre);
	free(paso23);
	list_destroy(pasos2);
}

/***************** FIN USE CASE DEFAULT *****************/


/***************** INICIO USE CASE CATEDRA *****************/
t_list* crearRecetaAsadoJugoso(){
	t_list* listaPasos = list_create();

	t_paso* paso1 = malloc(sizeof(t_paso));
	paso1->nombre = strdup("Cortar");
	paso1->tiempo = 4;
	list_add(listaPasos, paso1);

	t_paso* paso2 = malloc(sizeof(t_paso));
	paso2->nombre = strdup("Hornear");
	paso2->tiempo = 5;
	list_add(listaPasos, paso2);

	t_paso* paso3 = malloc(sizeof(t_paso));
	paso3->nombre = strdup("Servir");
	paso3->tiempo = 2;
	list_add(listaPasos, paso3);

	return listaPasos;
}

t_list* crearRecetaAsadoSeco(){
	t_list* listaPasos = list_create();

	t_paso* paso1 = malloc(sizeof(t_paso));
	paso1->nombre = strdup("Cortar");
	paso1->tiempo = 4;
	list_add(listaPasos, paso1);

	t_paso* paso2 = malloc(sizeof(t_paso));
	paso2->nombre = strdup("Hornear");
	paso2->tiempo = 10;
	list_add(listaPasos, paso2);

	t_paso* paso3 = malloc(sizeof(t_paso));
	paso3->nombre = strdup("Servir");
	paso3->tiempo = 2;
	list_add(listaPasos, paso3);

	return listaPasos;
}

t_list* crearRecetaChoripan(){
	t_list* listaPasos = list_create();

	t_paso* paso1 = malloc(sizeof(t_paso));
	paso1->nombre = strdup("Cortar");
	paso1->tiempo = 1;
	list_add(listaPasos, paso1);

	t_paso* paso2 = malloc(sizeof(t_paso));
	paso2->nombre = strdup("Hornear");
	paso2->tiempo = 2;
	list_add(listaPasos, paso2);

	t_paso* paso3 = malloc(sizeof(t_paso));
	paso3->nombre = strdup("Servir");
	paso3->tiempo = 1;
	list_add(listaPasos, paso3);

	return listaPasos;
}

t_list* crearRecetaTortillaDePapa(){
	t_list* listaPasos = list_create();

	t_paso* paso1 = malloc(sizeof(t_paso));
	paso1->nombre = strdup("Cortar");
	paso1->tiempo = 2;
	list_add(listaPasos, paso1);

	t_paso* paso2 = malloc(sizeof(t_paso));
	paso2->nombre = strdup("Hervir");
	paso2->tiempo = 2;
	list_add(listaPasos, paso2);

	t_paso* paso3 = malloc(sizeof(t_paso));
	paso3->nombre = strdup("Hornear");
	paso3->tiempo = 3;
	list_add(listaPasos, paso3);

	t_paso* paso4 = malloc(sizeof(t_paso));
	paso4->nombre = strdup("Servir");
	paso4->tiempo = 1;
	list_add(listaPasos, paso4);

	return listaPasos;
}

void freeMemPasoReceta(t_paso* paso){
	free(paso->nombre);
	free(paso);
}

void useCaseCatedra() {

	t_list* asadoJugoso = crearRecetaAsadoJugoso();
	t_list* asadoSeco = crearRecetaAsadoSeco();
	t_list* choripan = crearRecetaChoripan();
	t_list* tortillaDepapa = crearRecetaTortillaDePapa();

	t_pcb* asadoJugoso1 = crearPCB(1, 1, "AsadoJugoso", asadoJugoso);

	t_pcb* chori1 = crearPCB(2, 1, "Choripan", choripan);
	t_pcb* chori2 = crearPCB(2, 2, "Choripan", choripan);
	t_pcb* chori3 = crearPCB(2, 3, "Choripan", choripan);
	t_pcb* chori4 = crearPCB(2, 4, "Choripan", choripan);
	t_pcb* chori5 = crearPCB(2, 5, "Choripan", choripan);

	t_pcb* asadoSeco1 = crearPCB(3, 1, "AsadoSeco", asadoSeco);
	t_pcb* asadoSeco2 = crearPCB(3, 2, "AsadoSeco", asadoSeco);

	t_pcb* tortilla1 = crearPCB(4, 1, "TortillaDePapa", tortillaDepapa);
	t_pcb* tortilla2 = crearPCB(4, 2, "TortillaDePapa", tortillaDepapa);

	encolarNuevoPlato(asadoJugoso1);
	encolarNuevoPlato(chori1);
	encolarNuevoPlato(chori2);
	encolarNuevoPlato(chori3);
	encolarNuevoPlato(chori4);
	encolarNuevoPlato(chori5);
	encolarNuevoPlato(asadoSeco1);
	encolarNuevoPlato(asadoSeco2);
	encolarNuevoPlato(tortilla1);
	encolarNuevoPlato(tortilla2);

	sleep(1);

	list_destroy_and_destroy_elements(asadoJugoso, freeMemPasoReceta);
	list_destroy_and_destroy_elements(asadoSeco, freeMemPasoReceta);
	list_destroy_and_destroy_elements(choripan, freeMemPasoReceta);
	list_destroy_and_destroy_elements(tortillaDepapa, freeMemPasoReceta);
}
/***************** FIN USE CASE CATEDRA *****************/


/***************** INICIO USE CASE ERROR *****************/
void useCaseError() {
	printf("USE CASE ERROR NO IMPLEMENTADO");
}

/***************** FIN USE CASE ERROR *****************/

#include "cliente.h"

int main(int argc, char* argv[]) {

	if(argc < 5){
		printf("Error en los argumentos.");
		return EXIT_FAILURE;
	}
	hostEscucha = argv[2];
	puertoEscucha = argv[4];

	int inicializacion = inicializar();
	
	if(inicializacion == -1){
		// Esto se puede mejorar indicando más códigos que devuelva inicializar() en un switch y con un enum para cada número
		log_error(logger,"No se pudo inicializar el cliente correctamente");
		finalizar();
		return EXIT_FAILURE;
	}

	printf("Bienvenido al modulo CLIENTE. Ingrese un comando para continuar...\n");
	
	
	if(pthread_create(&consola,NULL,manejar_consola,NULL) != 0){
		perror("No se pudo manejar la consola");
	}
	void* res = NULL;
	pthread_join(consola,&res);		//reever q no muera copn dettach state.
	pthread_join(servidor,NULL);


	return EXIT_SUCCESS;
}

int inicializar(){

	inicializar_listas();
	if(!usuario_hash()){
		log_error(logger,"No se creo el ID del Cliente");
	}
	config = cargar_configuracion("./src/cliente.config", CLIENTE);

	logger = log_create(config->archivo_log,"Cliente",0,LOG_LEVEL_INFO);
	log_info(logger,"Se creo el ID del Cliente: %s.",config->id_cliente);
	
	if(handshake() == -1){
		log_error(logger,"No se pudo conectar al modulo especificado por configuracion.");
		return -1;
	}
	
	if( pthread_create(&servidor,NULL,levantar_servidor,NULL) == -1 ){
		log_error(logger,"Hubo un error al levantar el servidor de escucha actualizaciones.");
		perror("Hubo un error al levantar el servidor de escucha actualizaciones.\n");
		return -1;
	}

	return 1;
}

void inicializar_listas(){
	hilos = list_create();
}

void liberar_config(){
	free(config->archivo_log);
	free(config->ip);
}

void* liberar_hilos(){
	list_destroy_and_destroy_elements(hilos, pthread_detach);
}

void finalizar(t_socket cliente){
	log_info(logger,"Finaliza el programa.");
	liberar_config();
	close(socket_servidor.socket);
	pthread_cancel(&consola);
	pthread_cancel(servidor);
	close(socket_conectado.socket);
	log_destroy(logger);
	liberar_hilos();
//	exit(1);
}

int usuario_hash(){

	printf("GENERANDO CLAVE UNICA DEL CLIENTE....\n");
	time_t* t = time(NULL);
	struct tm FECHA_HORA = *localtime(&t);
	char* ID_Cliente = string_new();
	//string_append(&ID_Cliente, "HOLAAAAAA");
	string_append(&ID_Cliente, string_itoa(FECHA_HORA.tm_yday));
	string_append(&ID_Cliente, string_itoa(FECHA_HORA.tm_wday));
	string_append(&ID_Cliente, string_itoa(FECHA_HORA.tm_year));
	string_append(&ID_Cliente, string_itoa(FECHA_HORA.tm_mon));
	string_append(&ID_Cliente, string_itoa(FECHA_HORA.tm_mday));
	string_append(&ID_Cliente, string_itoa(FECHA_HORA.tm_hour));
	string_append(&ID_Cliente, string_itoa(FECHA_HORA.tm_min));
	string_append(&ID_Cliente, string_itoa(FECHA_HORA.tm_sec));
	
	srand ( getpid() );
	string_append(&ID_Cliente, string_itoa(rand() % 99));	//le agrego 2 int como mucho al cliente

	t_config* cli_cfg = config_create("./src/cliente.config");
	config_set_value(cli_cfg,"ID_CLIENTE",ID_Cliente);
	config_save(cli_cfg);
	config_destroy(cli_cfg);
	free(ID_Cliente);
	//free(t);
	//free(&FECHA_HORA);
	return 1;
}



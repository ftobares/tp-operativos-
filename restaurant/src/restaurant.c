#include "restaurant.h"

// Manejo de Signal & Monitoreo
static sigset_t signalMask;
int signalCaught;
pthread_t createThreadSignal();
void signalHandler();
pthread_t createThreadAtomicRecvManager();
pthread_t createThreadAtomicErrorManager();

// Test mode
#define UNIT_TEST "UNIT_TEST"
#define IP_PARAM "IP"
int testMode = 0;
char* ip_restaurante;

// Threads Globales
pthread_t signalThread;
pthread_t errorThread;

// Inicializacion
bool inicializar();
bool conectarseConApp(t_obtener_restaurante_respuesta_s* datosRestaurante);
bool consultarDatosRestauranteASindicato(t_nombre_restaurante_s* paqueteAEnviar, t_header* header);
pthread_t createServerThread();

// Core Logic
pthread_t createServerThread();

// Finalizacion
void finalizarPrograma();
void atomicSignalRecibido();
void atomicLoggingError();

// Utils
void banner();

// Test
void testPlanificador();

int main(int argc, char* argv[]) {

	banner();

	if(!(argc < 2)){
		if(string_equals_ignore_case(argv[1],UNIT_TEST)){
			printf("########## TEST MODE ON ########## \n");
			testMode = 1;
		}else if(string_equals_ignore_case(argv[1],IP_PARAM)){
			ip_restaurante = string_new();
			string_append(&ip_restaurante, argv[2]);
		}
	}else{
		perror("ERROR debe ingresar la ip del restaurante como parametro. Ejemplo: ./restaurante ip 127.0.0.1 \n");
		return EXIT_FAILURE;
	}

	if(!inicializar()){
		finalizarPrograma();
		return EXIT_FAILURE;
	}

	log_info(logger, "Restaurant Inicializado OK");

	// Configuro Signal
	signalThread = createThreadSignal();
	if(signalThread == -1){
		return EXIT_FAILURE;
	}

	// Creo Hilos de Monitoreo
	pthread_t recibidoThread = createThreadAtomicRecvManager();
	if(recibidoThread == -1){
		pthread_cancel(signalThread);
		return EXIT_FAILURE;
	}

	errorThread = createThreadAtomicErrorManager();
	if(errorThread == -1){
		pthread_cancel(signalThread);
		pthread_cancel(recibidoThread);
		return EXIT_FAILURE;
	}

	// Inicializo planificador
	startupPlanificador();

	// Inicializo administrador de mensajes
	pthread_t hiloServer = createServerThread();
	if(hiloServer == -1){
		pthread_cancel(recibidoThread);
		finalizarPrograma();
		return EXIT_FAILURE;
	}

	/***** PARA TESTEO *****/
	if(testMode == 1) testPlanificador(argv[2]);
	/***** PARA TESTEO *****/

	ciclosCPU();

	finalizarPrograma();

	return EXIT_SUCCESS;
}

bool conectarseConApp(t_obtener_restaurante_respuesta_s* datosRestaurante){

	t_restaurante_handshake* paqueteAEnviar = malloc(sizeof(t_restaurante_handshake));
	paqueteAEnviar->nombre_restaurante = datosRestaurante->nombre;
	paqueteAEnviar->nombre_restaurante_size = datosRestaurante->nombre_size;
	paqueteAEnviar->posicion_x = datosRestaurante->posicion->x;
	paqueteAEnviar->posicion_y = datosRestaurante->posicion->y;

	paqueteAEnviar->ip = ip_restaurante;
	paqueteAEnviar->ip_size = strlen(paqueteAEnviar->ip) + 1;

	paqueteAEnviar->puerto = config->puerto_escucha;
	paqueteAEnviar->puerto_size = strlen(paqueteAEnviar->puerto) + 1;

	t_header* header = malloc(sizeof(t_header));

	armarHeader(header, T_DATOS_RESTAURANTE, (sizeof(uint32_t) * 5 + paqueteAEnviar->nombre_restaurante_size + paqueteAEnviar->ip_size + paqueteAEnviar->puerto_size));

	socketApp = crear_socket_de_conexion(config->ip_app, config->puerto_app);

	log_info(logger, "Conectandose con App...");
	if(!conectar_socket(socketApp)){
		return false;
	}

	t_buffer buffer = serializar_mensaje(paqueteAEnviar, header->msj_type, header->size, socketApp.socket);

	log_info(logger, "Enviando datos del Restaurante...");
	if(!enviar_mensaje(&buffer)){
		return false;
	}

	t_buffer respuestaApp = recibir_mensaje(socketApp.socket);
	t_ping_s* resultado = deserializar_mensaje(&respuestaApp);
	if(!resultado->result){
		return false;
	}
	log_info(logger, "Conectado a la App satisfactoriamente");

	return true;
}

bool consultarDatosRestauranteASindicato(t_nombre_restaurante_s* paqueteAEnviar, t_header* header){

	socketSindicato = crear_socket_de_conexion(config->ip_sindicato, config->puerto_sindicato);

	log_info(logger, "Conectandose con sindicato...");
	if(conectar_socket(socketSindicato)){
		t_buffer buffer = serializar_mensaje(paqueteAEnviar, header->msj_type, header->size, socketSindicato.socket);

		log_info(logger, "Consultando datos del restaurante...");
		if(enviar_mensaje(&buffer)){

			log_info(logger, "Esperando respuesta...");
			t_buffer bufferFromServer = recibir_mensaje(socketSindicato.socket);

			if(bufferFromServer.msj_type == T_OBTENER_RESTAURANTE_RESPUESTA){
				log_info(logger, "Datos obtenidos. Continua inicializacion de restaurante");
				restaurante = (t_obtener_restaurante_respuesta_s*) deserializar_mensaje(&bufferFromServer);

				if(restaurante->cantCocineros < 1 || string_equals_ignore_case(restaurante->nombre, "error")){
					log_error(logger, "Error obteniendo metadata de restaurante en Sindicato");
					return false;
				}

//				mockLogging(restaurante);

				setCantidadPedidos(restaurante->cantidadPedidos);

				return true;
			}

			log_error(logger, "Tipo de mensaje recibido erroneo. Cerrando proceso...");
		}
	}

	return false;
}

bool inicializar(){

	// Se levanta archivo de configuracion
	config = cargar_configuracion(CONFIG_PATH, RESTAURANTE);
	logger = log_create(config->archivo_log, "Restaurante", 1, LOG_LEVEL_INFO);

	sem_init(&signalRecibido, 1, 0);
	sem_init(&signalError, 1, 0);

	restaurante = NULL;

	/***** PARA TESTEO *****/
	if(testMode == 1){
		restaurante = (t_obtener_restaurante_respuesta_s*) getMockRestaurante("ElParrillon");
		return true;
	}
	/***** PARA TESTEO *****/

	t_nombre_restaurante_s* paqueteAEnviar = malloc(sizeof(t_nombre_restaurante_s));
	empaquetarNombreRestaurante(paqueteAEnviar);

	t_header* header = malloc(sizeof(t_header));
	armarHeader(header, T_OBTENER_RESTAURANTE, (sizeof(uint32_t) + paqueteAEnviar->nombre_restaurante_size));

	//Contacto sindicato para obtener informacion basica del restaurant
	if(consultarDatosRestauranteASindicato(paqueteAEnviar, header)){

		// Aviso existencia del restaurante a la App
		if(conectarseConApp(restaurante)) return true;

	}

	return false;
}

void finalizarServicios(){
	// Bajar Planificador
	finalizarPlanificador();

	// Bajar Server
	finalizarServer();

	sleep(30);
}

void finalizarPrograma(){
	log_info(logger, "Finalizando Restaurante...");

	if(restaurante != NULL) liberarRestaurante(restaurante);
	liberarConfig(config);

	free(ip_restaurante);

	freeaddrinfo(socketSindicato.socket_info);
	freeaddrinfo(socketApp.socket_info);
	log_destroy(logger);

	pthread_cancel(signalThread);
	pthread_cancel(errorThread);

	sem_destroy(&signalError);
	sem_destroy(&signalRecibido);

	sleep(5);
}

pthread_t createThreadSignal() {
	log_info(logger, "Iniciando Signal Handler");
	// Configuro Signal
	sigemptyset(&signalMask);
	sigaddset(&signalMask, SIGINT);
	sigaddset(&signalMask, SIGTERM);
	sigaddset(&signalMask, SIGQUIT);
	if (pthread_sigmask(SIG_BLOCK, &signalMask, NULL) != 0) {
		log_error(logger, "Error creando mask de señales.");
		return -1;
	}

	pthread_t signalThread;
	log_info(logger, "Creando Handler de Señales");
	if (pthread_create(&signalThread, NULL, signalHandler, NULL) != 0) {
		log_error(logger, "Error creando hilo para capturar señales.");
		return -1;
	};

	return signalThread;
}

void atomicSignalRecibido(){
	sem_wait(&signalRecibido);

	switch(signalCaught){
	case SIGINT:
		log_info(logger, "Signal recibida SIGINT: %i", signalCaught);
		finalizarServicios();
		break;
	case SIGTERM: //TESTEAR
		log_info(logger, "Signal recibida SIGTERM: %i", signalCaught);
		finalizarServicios();
		break;
	case SIGQUIT:
		log_info(logger, "Signal recibida SIGQUIT: %i", signalCaught);
		finalizarServicios();
		break;
	default:
		printf("Signal capturada. signal=%i",signalCaught);
	}
}

void atomicLoggingError(){
	sem_wait(&signalError);
	log_error(logger, "Error recibiendo signal");
}

void signalHandler(){

	int rc;
	rc = sigwait(&signalMask, &signalCaught);
	if(rc != 0){
		sem_post(&signalError);
	}

	sem_post(&signalRecibido);

}

pthread_t createServerThread() {
	// Inicializo administrador de mensajes
	pthread_t hiloServer;
	pthread_attr_t attrServer;
	pthread_attr_init(&attrServer);
	pthread_attr_setdetachstate(&attrServer, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&hiloServer, &attrServer, startupServer, NULL) != 0) {
		log_error(logger, "Error creando el hilo de server");
	}
	return hiloServer;
}

pthread_t createThreadAtomicRecvManager() {
	log_info(logger, "Iniciando hilo receptor-seguro de señales.");
	pthread_t recibidoThread;
	pthread_attr_t attrRecibido;
	pthread_attr_init(&attrRecibido);
	pthread_attr_setdetachstate(&attrRecibido, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&recibidoThread, &attrRecibido, atomicSignalRecibido, NULL) != 0) {
		log_error(logger, "Error creando el hilo de recibidoThread");
		return -1;
	}
	return recibidoThread;
}

pthread_t createThreadAtomicErrorManager() {
	log_info(logger, "Inicializar hilo manejo de error fallo en captura de señales.");
	pthread_t errorThread;
	pthread_attr_t attrError;
	pthread_attr_init(&attrError);
	pthread_attr_setdetachstate(&attrError, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&errorThread, &attrError, atomicLoggingError, NULL) != 0) {
		log_error(logger, "Error creando el hilo de recibidoThread");
		return -1;
	}
	return errorThread;
}

void banner(){
	char* banner1 = string_new();
	char* banner2 = string_new();
	char* banner3 = string_new();
	char* banner4 = string_new();
	char* banner5 = string_new();
	char* banner6 = string_new();

	string_append(&banner1, " _____           _                              _       ");
	string_append(&banner2, "|  __ \\         | |                            | |      ");
	string_append(&banner3, "| |__) |___  ___| |_ __ _ _   _ _ __ __ _ _ __ | |_ ___ ");
	string_append(&banner4, "|  _  // _ \\/ __| __/ _` | | | | '__/ _` | '_ \\| __/ _ \\");
	string_append(&banner5, "| | \\ \\  __/\\__ \\ || (_| | |_| | | | (_| | | | | ||  __/");
	string_append(&banner6, "|_|  \\_\\___||___/\\__\\__,_|\\__,_|_|  \\__,_|_| |_|\\__\\___|");

	puts(banner1);
	puts(banner2);
	puts(banner3);
	puts(banner4);
	puts(banner5);
	puts(banner6);

	free(banner1);
	free(banner2);
	free(banner3);
	free(banner4);
	free(banner5);
	free(banner6);
}

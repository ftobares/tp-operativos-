#include "planificador.h"

volatile sig_atomic_t isExit = 1;

t_obtener_restaurante_respuesta_s* getMockRestaurante(char* nombreRestaurante);

//Startup
void setearVariablesGlobales();
bool inicializarQueues();
bool iniciarSemaforos();
bool crearHilosDePlanificacion();
pthread_t* crearHilosDePlanificacionConAfinidad(const pthread_attr_t* attr, int afinidad);
pthread_t* crearHilosDeHornos(const pthread_attr_t* attr, int horno);

// Reloj CPU
//bool iniciarCiclosCocineros();
void ciclosCPU();
void sumarCicloCPU(t_pcb* plato);

// Planificacion Largo Plazo
void planificarPlatosNew();
void planificarPlatosExit();

// Planificacion Corto Plazo
void planificarColasConAfinidad(int afinidad);
void planificarColasSinAfinidad();
void planificarEjecucion();
void planificarPlatosBlock();
void planificarHornos();
void cocinarPlato(int horno);
void resetQuantumCocineroYEstadoDeBloqueo(t_pcb* plato, char* proximoPaso);
void encolarPlatoReady(t_pcb* plato);
void incrementQuantumRecetaYEncolarExec(t_pcb* plato);
void encolarPlatoBlock(t_pcb* plato, t_paso* proximoPaso);
void encolarPlatoEnHorno(t_pcb* plato, t_paso* proximoPaso);
void encolarPlatoExit(t_pcb* plato);
void ejecutarPlato(t_pcb* plato);

// Algoritmos
void planificacionRoundRobin(t_pcb* plato);
void planificacionFIFO(t_pcb* plato);

// Seek and Destroy
void liberarRecursosPlanificador();
void freeMemHilos();
void freeMemHilo(pthread_t* thread);
void freeMemQueues();
void freeMemQueuesPorAfinidad(t_queue_afinidad* qAfinidad);
void freeMemListas();
void freeMemSemaforos();
void freeMemHorno(t_horno_io* horno);
void freeMemCocinero(t_cocinero_cpu* cocinero);
void freeMemPasosReceta(t_paso* paso);
void vaciarQueues();
void vaciarHornos();
void cerrarSemaforos();

// Utils
bool isCocineroLibre(t_cocinero_cpu* cocinero, char* afinidad);
bool isCocineroLibreSinAfinidad(t_cocinero_cpu* cocinero);
bool containsSinAfinidad(t_cocinero_cpu* cocinero);
void imprimirPasos(t_paso* paso);
t_list* listDeepCopyPasos(t_list* listPasos);
void setCocineroComoLibre(t_cocinero_cpu* cocinero);
void mockLogging(t_obtener_restaurante_respuesta_s* restauranteLocal);

/**************************** FUNCIONES PUBLICAS ****************************/
int startupPlanificador(){

	log_debug(logger, "Iniciando startupPlanificador");

	if(string_equals_ignore_case(config->algoritmo_planificacion, "RR")){
		log_info(logger, "PLANIFICACION POR ROUND ROBIN. QUANTUM=%i", config->quantum);
	}else{
		log_info(logger, "PLANIFICACION POR FIFO");
	}

	setearVariablesGlobales();

	if(!inicializarQueues()){
		liberarRecursosPlanificador();
		return EXIT_FAILURE;
	}
	if(!iniciarSemaforos()){
		liberarRecursosPlanificador();
		return EXIT_FAILURE;
	}
	if(!crearHilosDePlanificacion()){
		liberarRecursosPlanificador();
		return EXIT_FAILURE;
	}

	log_debug(logger, "Finalizando startupPlanificador");

	return EXIT_SUCCESS;
}

t_pcb* crearPCB(uint32_t id_pedido, uint32_t id_plato, char* plato, t_list* pasosReceta){

	log_info(logger, "Creando PCB || Pedido %i || Plato %i - %s", id_pedido, id_plato, plato);

	t_pcb* platoPCB = malloc(sizeof(t_pcb));

	platoPCB->id_pedido = id_pedido;
	platoPCB->id_plato = id_plato;

	t_receta* receta = malloc(sizeof(t_receta));
	receta->plato = strdup(plato);
	receta->pasos = listDeepCopyPasos(pasosReceta);
	platoPCB->receta = receta;

	platoPCB->cocinero = NULL;
	platoPCB->tipoBloqueo = NULL;
	platoPCB->ciclosBloqueado = 0;
	platoPCB->quatumProcesados = 0;
	platoPCB->pasoActualEnReceta = 0;
	platoPCB->ciclosProcesadosEnPasoDeReceta = 0;

//	log_info(logger, "Pedido %i:", id_pedido);
//	log_info(logger, "Plato %i creado:", id_plato);
//	log_info(logger, "Nombre Plato: %s", receta->plato);
//	list_iterate(receta->pasos, imprimirPasos);

	return platoPCB;
}

void encolarNuevoPlato(t_pcb* pcb){

	log_debug(logger, "Inicio encolarNuevoPlato");
	log_debug(logger, "Encolar Pedido %i || Plato %i - %s", pcb->id_pedido, pcb->id_plato, pcb->receta->plato);

	pthread_mutex_lock(&mutexQueueNew);
	queue_push(queueNew, pcb);
	pthread_mutex_unlock(&mutexQueueNew);

	sem_post(&semPlatosNew);

	log_debug(logger, "Fin encolarNuevoPlato");
}

/**************************** FUNCIONES DE INICIALIZACION ****************************/
void setearVariablesGlobales(){
	RETARDO_CICLO_CPU = config->retardo_ciclo_cpu;
}

bool inicializarQueues(){

	log_debug(logger, "Inicio inicializarQueueYSemaforos");

	// Crear queue basicas
	queueNew = queue_create();
	queueReady = queue_create();
	queueHornos = queue_create();
	queueExit = queue_create();
	queueExec = queue_create();
	queueBlock = queue_create();

	listCocineros = list_create();
	listQueueAfinidades = list_create();
	listHornos = list_create();

	CANTIDAD_AFINIDADES = 0;

	// Crear colas por afinidad
	// Recorro los cocineros para determinar que afinidades tienen
	for(int i = 0; i < restaurante->cantCocineros; i++){
		t_cocinero_s* cocinero = (t_cocinero_s*) list_get(restaurante->cocineros, i);

		if(!string_equals_ignore_case(cocinero->afinidad, SIN_AFINIDAD)){

			bool _listContainsAfinidad(t_queue_afinidad* element){
				return string_equals_ignore_case(element->afinidad, cocinero->afinidad);
			}

			// Validacion para no crear mas de un elemento por afinidad
			if(list_size(listQueueAfinidades) == 0 || !list_any_satisfy(listQueueAfinidades, _listContainsAfinidad)){

				t_queue_afinidad* afinidad = malloc(sizeof(t_queue_afinidad));
				afinidad->semaforo = malloc(sizeof(sem_t));
				if(sem_init(afinidad->semaforo, 0, 0) != 0){
					log_error(logger, "Error iniciando semaforo listQueueAfinidades %i", afinidad);
				}
				afinidad->cocineroLibreEnAfinidad = malloc(sizeof(sem_t));

				afinidad->afinidadQueue = queue_create();

				afinidad->afinidad = strdup(cocinero->afinidad);

				afinidad->mutex = malloc(sizeof(pthread_mutex_t));
				if(pthread_mutex_init(afinidad->mutex,NULL) != 0) return false;

				list_add(listQueueAfinidades, afinidad);
				CANTIDAD_AFINIDADES++;
			}
		}

		// Guardo los Cocineros (CPUs)
		t_cocinero_cpu* cocineroCPU = malloc(sizeof(t_cocinero_cpu));
		cocineroCPU->id = cocinero->id;
		cocineroCPU->afinidad = strdup(cocinero->afinidad);
		cocineroCPU->libre = true;
		list_add(listCocineros, cocineroCPU);
	}

	// Crear colas de entrada/salida, para los hornos
	for(int i = 0; i < restaurante->cantidadHornos; i++){
		t_horno_io* horno = malloc(sizeof(t_horno_io));
		horno->libre = true;
		horno->plato = NULL;
		horno->semaforo = malloc(sizeof(sem_t));
		if(sem_init(horno->semaforo, 0, 0) != 0){
			log_error(logger, "Error iniciando semaforo arraySemHornos %i", i);
		}

		list_add(listHornos, horno);
	}

	log_info(logger, "Cantidad de Afinidades: %i", CANTIDAD_AFINIDADES);
	log_debug(logger, "Fin inicializarQueueYSemaforos");

	return true;
}

bool iniciarSemaforos(){

	log_debug(logger, "Inicio iniciarSemaforos");

	// Semaforos basicos
	if(sem_init(&semPlatosNew,0,0) != 0) return false;
	if(sem_init(&semPlatosReady,0,0) != 0) return false;
	if(sem_init(&semPlatosBlock,0,0) != 0) return false;
	if(sem_init(&semPlatosExec,0,0) != 0) return false;
	if(sem_init(&semPlatosExit,0,0) != 0) return false;

	if(sem_init(&semCPUBurstExec,0,0) != 0) return false;
	if(sem_init(&semCPUBurstBlock,0,0) != 0) return false;
	if(sem_init(&semCPUBurstIO,0,0) != 0) return false;

	for(int i = 0; i < CANTIDAD_AFINIDADES; i++){

		t_queue_afinidad* afinidadElement = (t_queue_afinidad*) list_get(listQueueAfinidades, i);

		// Busco cocineros que cumplan con la condicion de tener la afinidad actual en posicion i
		bool _listContainsAfinidad(void* element){
			t_cocinero_cpu* cocinero = (t_cocinero_cpu*) element;
			return string_equals_ignore_case(cocinero->afinidad, afinidadElement->afinidad);
		}

		int cantCocinerosPorAfinidad = list_count_satisfying(listCocineros, _listContainsAfinidad);

		if(sem_init(afinidadElement->cocineroLibreEnAfinidad, 0, cantCocinerosPorAfinidad) != 0){
			log_error(logger, "Error iniciando semaforo arraySemCocinerosConAfinidad %i", i);
		}
	}

	int cantCocinerosSinAfinidad = list_count_satisfying(listCocineros, containsSinAfinidad);
	if(sem_init(&semCocinerosSinAfinidad, 0, cantCocinerosSinAfinidad) != 0) return false;

	if(sem_init(&semQueueHornos, 0, 0) != 0) return false;

	if(pthread_mutex_init(&mutexQueueNew,NULL) != 0) return false;
	if(pthread_mutex_init(&mutexQueueReady,NULL) != 0) return false;
	if(pthread_mutex_init(&mutexQueueBlock,NULL) != 0) return false;
	if(pthread_mutex_init(&mutexQueueExec,NULL) != 0) return false;
	if(pthread_mutex_init(&mutexQueueExit,NULL) != 0) return false;
	if(pthread_mutex_init(&mutexQueueHornos,NULL) != 0) return false;
	if(pthread_mutex_init(&mutexCocinerosLibres,NULL) != 0) return false;

	log_debug(logger, "Fin iniciarSemaforos");

	return true;
}

bool crearHilosDePlanificacion(){

	log_debug(logger, "Inicio crearHilosDePlanificacion");

	threadsPlanificador = list_create();

	pthread_t* queue_new = malloc(sizeof(pthread_t));
	pthread_t* queue_ready = malloc(sizeof(pthread_t));
	pthread_t* queue_block = malloc(sizeof(pthread_t));
	pthread_t* queue_exec = malloc(sizeof(pthread_t));
	pthread_t* queue_exit = malloc(sizeof(pthread_t));
	pthread_t* queue_hornos = malloc(sizeof(pthread_t));

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	if(pthread_create(queue_new, &attr, planificarPlatosNew, NULL) != 0){
		log_error(logger,"Error creando hilo para planificar queue NEW");
		return false;
	}

	for(int i = 0; i < CANTIDAD_AFINIDADES; i++){
		pthread_t* threadAfinidad = crearHilosDePlanificacionConAfinidad(&attr, i);
		if(threadAfinidad == NULL){
			log_error(logger,"Error creando hilo para planificar queue READY - Cocineros con Afinidad %i", i);
			return false;
		}
		list_add(threadsPlanificador, threadAfinidad);
	}

	if(pthread_create(queue_ready, &attr, planificarColasSinAfinidad, NULL) != 0){
		log_error(logger,"Error creando hilo para planificar queue READY - Cocineros Sin Afinidad");
		return false;
	}

	if(pthread_create(queue_block, &attr, planificarPlatosBlock, NULL) != 0){
		log_error(logger,"Error creando hilo para planificar queue BLOCK");
		return false;
	}

	if(pthread_create(queue_hornos, &attr, planificarHornos, NULL) != 0){
		log_error(logger,"Error creando hilo para planificar queue HORNOS");
		return false;
	}

	for(int i = 0; i < restaurante->cantidadHornos; i++){
		pthread_t* threadHorno = crearHilosDeHornos(&attr, i);
		if(threadHorno == NULL){
			log_error(logger,"Error creando hilo para gestionar cada HORNO, horno=%i", i);
			return false;
		}
		list_add(threadsPlanificador, threadHorno);
	}

	if(pthread_create(queue_exec, &attr, planificarEjecucion, NULL) !=0){
		log_error(logger,"Error creando hilo para planificar queue EXEC");
		return false;
	}

	if(pthread_create(queue_exit, &attr, planificarPlatosExit, NULL) !=0){
		log_error(logger,"Error creando hilo para planificar queue EXIT");
		return false;
	}

	list_add(threadsPlanificador, queue_new);
	list_add(threadsPlanificador, queue_ready);
	list_add(threadsPlanificador, queue_block);
	list_add(threadsPlanificador, queue_exec);
	list_add(threadsPlanificador, queue_exit);

	log_debug(logger, "Fin crearHilosDePlanificacion");

	return true;
}

pthread_t* crearHilosDePlanificacionConAfinidad(const pthread_attr_t* attr, int afinidad) {
	pthread_t* thAfinidad = malloc(sizeof(pthread_t));
	if(pthread_create(thAfinidad, &*attr, planificarColasConAfinidad, afinidad) != 0){
		return NULL;
	}
	return thAfinidad;
}

pthread_t* crearHilosDeHornos(const pthread_attr_t* attr, int horno) {
	pthread_t* thHorno = malloc(sizeof(pthread_t));
	if(pthread_create(thHorno, &*attr, cocinarPlato, horno) != 0){
		return NULL;
	}
	return thHorno;
}

/**************************** FUNCIONES DE LOGICA CORE ****************************/

void ciclosCPU(){

	log_debug(logger, "Inicio ciclosCPU");

	while(isExit){

		sleep(RETARDO_CICLO_CPU);

		log_info(logger, " ### || ### Nuevo ciclo de CPU ### || ###");

		pthread_mutex_lock(&mutexQueueNew);
		pthread_mutex_lock(&mutexQueueExec);
		pthread_mutex_lock(&mutexQueueReady);
		pthread_mutex_lock(&mutexQueueBlock);
		pthread_mutex_lock(&mutexQueueHornos);
		pthread_mutex_lock(&mutexQueueExit);
		pthread_mutex_lock(&mutexCocinerosLibres);
		for(int i=0; i < CANTIDAD_AFINIDADES; i++) {
			t_queue_afinidad* qAfinidad = (t_queue_afinidad*) list_get(listQueueAfinidades, i);
			pthread_mutex_lock(qAfinidad->mutex);
		}

		if(queue_size(queueExec) > 0) sem_post(&semCPUBurstExec);
		if(queue_size(queueBlock) > 0) sem_post(&semCPUBurstBlock);

		sem_post(&semCPUBurstIO);

		pthread_mutex_unlock(&mutexQueueNew);
		pthread_mutex_unlock(&mutexQueueExec);
		pthread_mutex_unlock(&mutexQueueReady);
		pthread_mutex_unlock(&mutexQueueBlock);
		pthread_mutex_unlock(&mutexQueueHornos);
		pthread_mutex_unlock(&mutexQueueExit);
		pthread_mutex_unlock(&mutexCocinerosLibres);
		for(int i=0; i < CANTIDAD_AFINIDADES; i++) {
			t_queue_afinidad* qAfinidad = (t_queue_afinidad*) list_get(listQueueAfinidades, i);
			pthread_mutex_unlock(qAfinidad->mutex);
		}
	}

	liberarRecursosPlanificador();

	log_debug(logger, "Fin ciclosCPU");
}

void planificarPlatosNew(){
	
	log_debug(logger, "Inicio planificarPlatosNew");

	while(isExit){

		sem_wait(&semPlatosNew);

		pthread_mutex_lock(&mutexQueueNew);
		int qSize = queue_size(queueNew);
		pthread_mutex_unlock(&mutexQueueNew);

		if(qSize > 0){
			log_info(logger, "Nuevo plato a planificar en cola NEW");

			pthread_mutex_lock(&mutexQueueNew);
			t_pcb* platoPCB = queue_pop(queueNew);
			pthread_mutex_unlock(&mutexQueueNew);

			bool _queueContainsAfinidad(t_queue_afinidad* element){
				return string_equals_ignore_case(element->afinidad, platoPCB->receta->plato);
			}

			t_queue_afinidad* queueAfinidad = (t_queue_afinidad*) list_find(listQueueAfinidades, _queueContainsAfinidad);
			if(queueAfinidad != NULL){

				log_info(logger, "Pedido %i || Plato %i - %s || Se envia a cola ready de afinidad", platoPCB->id_pedido, platoPCB->id_plato, platoPCB->receta->plato);

				pthread_mutex_lock(queueAfinidad->mutex);
				queue_push(queueAfinidad->afinidadQueue, platoPCB);
				pthread_mutex_unlock(queueAfinidad->mutex);

				sem_post(queueAfinidad->semaforo);

			}else{

				log_info(logger, "Pedido %i || Plato %i - %s || se envia a cola de ready generica", platoPCB->id_pedido, platoPCB->id_plato, platoPCB->receta->plato);

				pthread_mutex_lock(&mutexQueueReady);
				queue_push(queueReady, platoPCB); //Sin Afinidad
				pthread_mutex_unlock(&mutexQueueReady);

				sem_post(&semPlatosReady);

			}
		}
	}
	
	log_debug(logger, "Fin planificarPlatosNew");

}

void planificarColasConAfinidad(int afinidad){
	
	t_queue_afinidad* qAfinidad = (t_queue_afinidad*) list_get(listQueueAfinidades, afinidad);
	sem_t* afinidadSem = (sem_t*) qAfinidad->semaforo;
	pthread_mutex_t* mutex = (pthread_mutex_t*) qAfinidad->mutex;
	sem_t* cocineroLibreSem = (pthread_mutex_t*) qAfinidad->cocineroLibreEnAfinidad;

	log_debug(logger, "Inicio planificarColasConAfinidad, afinidad: %s", qAfinidad->afinidad);

	bool _isCocineroLibreConAfinidad(t_cocinero_cpu* cocinero){
		return isCocineroLibre(cocinero, qAfinidad->afinidad);
	}

	while(isExit){

		//Espero a que haya cocineros libres para esta afindad
		sem_wait(cocineroLibreSem);

		//Espero a que haya platos listos en la cola de afindad
		sem_wait(afinidadSem);

		pthread_mutex_lock(mutex);
		int qSize = queue_size(qAfinidad->afinidadQueue);
		pthread_mutex_unlock(mutex);

		if(qSize > 0){

			pthread_mutex_lock(&mutexCocinerosLibres);
			t_cocinero_cpu* cocinero = (t_cocinero_cpu*) list_find(listCocineros, _isCocineroLibreConAfinidad);
			cocinero->libre = false;
			pthread_mutex_unlock(&mutexCocinerosLibres);

			pthread_mutex_lock(mutex);
			t_pcb* platoPCB = queue_pop(qAfinidad->afinidadQueue);
			pthread_mutex_unlock(mutex);

			// Asigno cocinero al plato
			platoPCB->cocinero = cocinero;

			log_info(logger, "Pedido %i || Plato %i con Afinidad %s || Asignado a Cocinero %i",
					platoPCB->id_pedido, platoPCB->id_plato, qAfinidad->afinidad, cocinero->id);

			pthread_mutex_lock(&mutexQueueExec);
			queue_push(queueExec, platoPCB);
			pthread_mutex_unlock(&mutexQueueExec);

			sem_post(&semPlatosExec);

		}
	}
	
	log_debug(logger, "Fin planificarColasConAfinidad");

}

void planificarColasSinAfinidad(){
	
	log_debug(logger, "Inicio planificarColasSinAfinidad");

	while(isExit){

		sem_wait(&semCocinerosSinAfinidad);

		sem_wait(&semPlatosReady);

		pthread_mutex_lock(&mutexQueueReady);
		int qSize = queue_size(queueReady);
		pthread_mutex_unlock(&mutexQueueReady);

		if(qSize > 0){

			pthread_mutex_lock(&mutexCocinerosLibres);
			t_cocinero_cpu* cocinero = (t_cocinero_cpu*) list_find(listCocineros, isCocineroLibreSinAfinidad);
			pthread_mutex_unlock(&mutexCocinerosLibres);

			pthread_mutex_lock(&mutexQueueReady);
			t_pcb* platoPCB = queue_pop(queueReady);
			pthread_mutex_unlock(&mutexQueueReady);

			// Asigno cocinero al plato
			cocinero->libre = false;
			platoPCB->cocinero = cocinero;

			log_info(logger, "Pedido %i || Plato %i SIN AFINIDAD || Asignado a Cocinero %i",
							platoPCB->id_pedido, platoPCB->id_plato, cocinero->id);

			pthread_mutex_lock(&mutexQueueExec);
			queue_push(queueExec, platoPCB);
			pthread_mutex_unlock(&mutexQueueExec);

			sem_post(&semPlatosExec);

		}

	}
	
	log_debug(logger, "Fin planificarColasSinAfinidad");
}

void planificarEjecucion(){
	
	log_debug(logger, "Inicio planificarEjecucion");

	while(isExit){

		// Espero que haya platos para ejecutar
		sem_wait(&semPlatosExec);

		// Espera de ciclos para ejecutar
		sem_wait(&semCPUBurstExec);

		pthread_mutex_lock(&mutexQueueExec);
		int qSize = queue_size(queueExec);
		pthread_mutex_unlock(&mutexQueueExec);

		if(qSize > 0){

			for(int i = 0; i < qSize; i++){

				pthread_mutex_lock(&mutexQueueExec);
				t_pcb* plato = (t_pcb*) queue_pop(queueExec);
				pthread_mutex_unlock(&mutexQueueExec);

				if(string_equals_ignore_case(config->algoritmo_planificacion, "RR")){
					planificacionRoundRobin(plato);
				}else{
					planificacionFIFO(plato);
				}

			}

		}
	}

	log_debug(logger, "Fin planificarEjecucion");
}

void planificarPlatosBlock(){

	log_debug(logger, "Inicio planificarPlatosBlock");

	while(isExit){

		// Espero que haya platos para ejecutar
		sem_wait(&semPlatosBlock);

		// Espera de ciclos para ejecutar
		sem_wait(&semCPUBurstBlock); // Analizar orden con el de arriba

		pthread_mutex_lock(&mutexQueueBlock);
		int qSize = queue_size(queueBlock);
		pthread_mutex_unlock(&mutexQueueBlock);

		if(qSize > 0){

			for(int i = 0; i < qSize; i++){

				pthread_mutex_lock(&mutexQueueBlock);
				t_pcb* plato = (t_pcb*) queue_pop(queueBlock);
				pthread_mutex_unlock(&mutexQueueBlock);

				t_paso* pasoActual = (t_paso*) list_get(plato->receta->pasos, plato->pasoActualEnReceta);

				if(plato->ciclosBloqueado < pasoActual->tiempo){
					log_info(logger, "Pedido %i || Plato %i - %s  || bloqueado, paso actual: %s", plato->id_pedido, plato->id_plato, plato->receta->plato, pasoActual->nombre);

					plato->ciclosBloqueado++;

					pthread_mutex_lock(&mutexQueueBlock);
					queue_push(queueBlock, plato); //Sigue en la lista de bloqueado
					pthread_mutex_unlock(&mutexQueueBlock);

					sem_post(&semPlatosBlock);

				}else{
					plato->pasoActualEnReceta++;
					plato->ciclosBloqueado = 0;

					t_paso* proximoPaso = (t_paso*) list_get(plato->receta->pasos, plato->pasoActualEnReceta);

					log_info(logger, "Pedido %i || Plato %i - %s || Fin de bloqueo por %s", plato->id_pedido, plato->id_plato, plato->receta->plato, pasoActual->nombre);

					if(proximoPaso != NULL && string_equals_ignore_case(proximoPaso->nombre, HORNEAR)){
						encolarPlatoEnHorno(plato, proximoPaso);
					}else{
						encolarPlatoReady(plato);
					}
				}
			}

		}
	}

	log_debug(logger, "Fin planificarPlatosBlock");

}

void planificarHornos(){

	log_debug(logger, "Inicio planificarHornos");

	bool isHornoLibre(t_horno_io* horno){
		return horno->libre;
	}

	while(isExit){
		sem_wait(&semQueueHornos);

		t_horno_io* hornoLibre = (t_horno_io*) list_find(listHornos, isHornoLibre);
		if(hornoLibre != NULL){

			pthread_mutex_lock(&mutexQueueHornos);
			t_pcb* plato = queue_pop(queueHornos);
			pthread_mutex_unlock(&mutexQueueHornos);

			hornoLibre->plato = plato;
			hornoLibre->libre = false;

			log_info(logger, "Horno libre || Asigno Pedido %i || Plato %i - %s", plato->id_pedido, plato->id_plato, plato->receta->plato);

			sem_post(hornoLibre->semaforo);

		}else{
			sleep(1);
			sem_post(&semQueueHornos);  //Espera activa... FEO
		}

	}

	log_debug(logger, "Fin planificarHornos");

}

void cocinarPlato(int horno){

	log_debug(logger, "Inicio cocinarPlato %i", horno);

	t_horno_io* hornoIO = (t_horno_io*) list_get(listHornos, horno);

	while(isExit){

		// Espera de ciclos para ejecutar
		sem_wait(&semCPUBurstIO);

		if(!hornoIO->libre){

			// Espera que el horno tenga la orden de cocinar
			sem_wait(hornoIO->semaforo);

			t_paso* pasoActual = (t_paso*) list_get(hornoIO->plato->receta->pasos, hornoIO->plato->pasoActualEnReceta);

			if(hornoIO->plato->ciclosBloqueado < pasoActual->tiempo){

				log_info(logger, "Horno %i cocinando || Pedido %i || Plato %i - %s", horno, hornoIO->plato->id_pedido, hornoIO->plato->id_plato, hornoIO->plato->receta->plato);

				hornoIO->plato->ciclosBloqueado++;

				sem_post(hornoIO->semaforo);

			}else{
				hornoIO->plato->pasoActualEnReceta++;
				hornoIO->plato->ciclosBloqueado = 0;
				hornoIO->plato->ciclosProcesadosEnPasoDeReceta = 0;

				log_info(logger, "Horno %i || Pedido %i || Plato %i - %s, se termino de cocinar", horno, hornoIO->plato->id_pedido, hornoIO->plato->id_plato, hornoIO->plato->receta->plato);

				encolarPlatoReady(hornoIO->plato);

				hornoIO->plato = NULL;
				hornoIO->libre = true;
			}
		}
	}

	log_debug(logger, "Fin cocinarPlato %i", horno);
}

void planificarPlatosExit(){

	while(isExit){
		sem_wait(&semPlatosExit);

		pthread_mutex_lock(&mutexQueueExit);
		int qSize = queue_size(queueExit);
		pthread_mutex_unlock(&mutexQueueExit);

		if(qSize > 0){

			pthread_mutex_lock(&mutexQueueExit);
			t_pcb* plato = (t_pcb*) queue_pop(queueExit);
			pthread_mutex_unlock(&mutexQueueExit);

			log_info(logger, "Pedido %i || Plato %i - %s ||  Ha finalizado.", plato->id_pedido, plato->id_plato, plato->receta->plato);

			platoListo(restaurante->nombre, plato->id_pedido, plato->receta->plato);

			destruirPCB(plato);

		}

	}

}

void ejecutarPlato(t_pcb* plato){
	t_paso* pasoActual = (t_paso*) list_get(plato->receta->pasos, plato->pasoActualEnReceta);

	if (pasoActual == NULL) {

		log_info(logger, "Pedido %i || Plato %i - %s || No tiene mas pasos se envia a EXIT", plato->id_pedido, plato->id_plato, plato->receta->plato);
		setCocineroComoLibre(plato->cocinero);
		encolarPlatoExit(plato);

	} else if (plato->ciclosProcesadosEnPasoDeReceta < pasoActual->tiempo) {

		log_info(logger, "Preparando paso: %s || Ejecutados [%i, %i] || Pedido %i || Plato %i - %s"  , pasoActual->nombre, plato->ciclosProcesadosEnPasoDeReceta, pasoActual->tiempo, plato->id_pedido, plato->id_plato, plato->receta->plato);
		incrementQuantumRecetaYEncolarExec(plato);

	} else {

		log_info(logger, "Terminado paso %s || Pedido %i || Plato %i - %s", pasoActual->nombre, plato->id_pedido, plato->id_plato, plato->receta->plato);

		plato->pasoActualEnReceta++;
		plato->ciclosProcesadosEnPasoDeReceta = 0;
		t_paso* proximoPaso = (t_paso*) list_get(plato->receta->pasos, plato->pasoActualEnReceta);

		if (proximoPaso == NULL) {

			log_info(logger, "Pedido %i || Plato %i - %s || No tiene mas pasos, se envia a EXIT", plato->id_pedido, plato->id_plato, plato->receta->plato);
			setCocineroComoLibre(plato->cocinero);
			encolarPlatoExit(plato);

		} else if (string_equals_ignore_case(proximoPaso->nombre, REPOSAR)) {

			log_info(logger, "Pedido %i || Plato %i - %s || Se envia a bloqueo para %s", plato->id_pedido, plato->id_plato, plato->receta->plato, REPOSAR);
			setCocineroComoLibre(plato->cocinero);
			encolarPlatoBlock(plato, proximoPaso);

		} else if (string_equals_ignore_case(proximoPaso->nombre, HORNEAR)) {

			log_info(logger, "Pedido %i || Plato %i - %s || Se envia a bloqueo para %s", plato->id_pedido, plato->id_plato, plato->receta->plato, HORNEAR);
			setCocineroComoLibre(plato->cocinero);
			encolarPlatoEnHorno(plato, proximoPaso);

		} else {

			log_info(logger, "Preparando paso: %s || Pedido %i || Plato %i - %s", proximoPaso->nombre, plato->id_pedido, plato->id_plato, plato->receta->plato);
			incrementQuantumRecetaYEncolarExec(plato);
		}
	}
}

void planificacionRoundRobin(t_pcb* plato) {

	log_info(logger, "Pedido %i || Plato %i - %s || Quantums [%i,%i]", plato->id_pedido, plato->id_plato, plato->receta->plato, plato->quatumProcesados, config->quantum);

	if (plato->quatumProcesados < config->quantum) {
		ejecutarPlato(plato);
	} else {

		log_info(logger, "Todos los quantums consumidos para || Pedido %i || Plato %i - %s", plato->id_pedido, plato->id_plato, plato->receta->plato);

		t_cocinero_cpu* cocinero = plato->cocinero;
		t_paso* pasoActual = (t_paso*) list_get(plato->receta->pasos, plato->pasoActualEnReceta);

		if (plato->ciclosProcesadosEnPasoDeReceta == pasoActual->tiempo) {

			plato->pasoActualEnReceta++;
			plato->ciclosProcesadosEnPasoDeReceta = 0;
			t_paso* proximoPaso = (t_paso*) list_get(plato->receta->pasos, plato->pasoActualEnReceta);

			if (proximoPaso == NULL) {

				log_info(logger, "Pedido %i || Plato %i - %s || No tiene mas pasos, se envia a EXIT", plato->id_pedido, plato->id_plato, plato->receta->plato);
				encolarPlatoExit(plato);

			} else if (string_equals_ignore_case(proximoPaso->nombre, REPOSAR)) {

				encolarPlatoBlock(plato, proximoPaso);

			} else if (string_equals_ignore_case(proximoPaso->nombre, HORNEAR)) {

				encolarPlatoEnHorno(plato, proximoPaso);

			} else {
				encolarPlatoReady(plato);
			}
		} else {
			encolarPlatoReady(plato);
		}
		setCocineroComoLibre(cocinero);
	}
}

void planificacionFIFO(t_pcb* plato){

	ejecutarPlato(plato);

}

void resetQuantumCocineroYEstadoDeBloqueo(t_pcb* plato, char* proximoPaso){
	plato->tipoBloqueo = strdup(proximoPaso);
	plato->quatumProcesados = 0;
	plato->cocinero = NULL;
}

void encolarPlatoReady(t_pcb* plato) {

	char* empty = string_new("");
	resetQuantumCocineroYEstadoDeBloqueo(plato, empty);
	free(empty);

	bool _containsAfinidad(t_queue_afinidad* element){
		return string_equals_ignore_case(element->afinidad, plato->receta->plato);
	}

	t_queue_afinidad* qAfinidad = (t_queue_afinidad*) list_find(listQueueAfinidades, _containsAfinidad);

	if(qAfinidad != NULL){

		pthread_mutex_lock(qAfinidad->mutex);
		queue_push(qAfinidad->afinidadQueue, plato);
		pthread_mutex_unlock(qAfinidad->mutex);

		sem_post(qAfinidad->semaforo);

	}else{

		pthread_mutex_lock(&mutexQueueReady);
		queue_push(queueReady, plato); //Sin Afinidad
		pthread_mutex_unlock(&mutexQueueReady);

		sem_post(&semPlatosReady);

	}

}

void incrementQuantumRecetaYEncolarExec(t_pcb* plato) {

	plato->ciclosProcesadosEnPasoDeReceta++;
	plato->quatumProcesados++;

	pthread_mutex_lock(&mutexQueueExec);
	queue_push(queueExec, plato);
	pthread_mutex_unlock(&mutexQueueExec);

	sem_post(&semPlatosExec);

}

void encolarPlatoBlock(t_pcb* plato, t_paso* proximoPaso) {

	resetQuantumCocineroYEstadoDeBloqueo(plato, proximoPaso->nombre);

	pthread_mutex_lock(&mutexQueueBlock);
	queue_push(queueBlock, plato);
	pthread_mutex_unlock(&mutexQueueBlock);

	sem_post(&semPlatosBlock);

}

void encolarPlatoEnHorno(t_pcb* plato, t_paso* proximoPaso) {

	resetQuantumCocineroYEstadoDeBloqueo(plato, proximoPaso->nombre);

	pthread_mutex_lock(&mutexQueueHornos);
	queue_push(queueHornos, plato); //Paso a bloqueado
	pthread_mutex_unlock(&mutexQueueHornos);

	sem_post(&semQueueHornos);

}

void encolarPlatoExit(t_pcb* plato) {

	plato->cocinero = NULL;

	pthread_mutex_lock(&mutexQueueExit);
	queue_push(queueExit, plato);
	pthread_mutex_unlock(&mutexQueueExit);

	sem_post(&semPlatosExit);

}

void setCocineroComoLibre(t_cocinero_cpu* cocinero) {

	cocinero->libre = true;

	if (string_equals_ignore_case(cocinero->afinidad, SIN_AFINIDAD)) {
		sem_post(&semCocinerosSinAfinidad);
	} else {
		for (int j = 0; j < CANTIDAD_AFINIDADES; j++) {
			t_queue_afinidad* qAfinidad = (t_queue_afinidad*) list_get(listQueueAfinidades, j);
			if (string_equals_ignore_case(qAfinidad->afinidad, cocinero->afinidad)) {
				sem_post(qAfinidad->cocineroLibreEnAfinidad);
			}
		}
	}
}

/**************************** FUNCIONES UTILS ****************************/

bool isCocineroLibre(t_cocinero_cpu* cocinero, char* afinidad){
	return cocinero->libre && string_equals_ignore_case(cocinero->afinidad, afinidad);
}

bool isCocineroLibreSinAfinidad(t_cocinero_cpu* cocinero){
	return isCocineroLibre(cocinero, SIN_AFINIDAD);
}

bool containsSinAfinidad(t_cocinero_cpu* cocinero){
	return string_equals_ignore_case(cocinero->afinidad, SIN_AFINIDAD);
}

void sumarCicloCPU(t_pcb* plato){
	plato->quatumProcesados++;
}

void imprimirPasos(t_paso* paso){
	log_debug(logger, "Paso: %s", paso->nombre);
	log_debug(logger, "Tiempo del paso: %i", paso->tiempo);
}

t_list* listDeepCopyPasos(t_list* listPasos){
	t_list* listAux = list_create();
	int size = list_size(listPasos);
	for(int i=0; i < size; i++){
		t_paso* pasoAux = malloc(sizeof(t_paso));
		t_paso* paso = (t_paso*) list_get(listPasos, i);
		pasoAux->tiempo = paso->tiempo;
		pasoAux->nombre = strdup(paso->nombre);
		list_add(listAux, pasoAux);
	}
	return listAux;
}

void mockLogging(t_obtener_restaurante_respuesta_s* restauranteLocal) {
	log_info(logger, "Datos de restaurante enviados: ");
	log_info(logger, "ID: %i", restauranteLocal->id);
	log_info(logger, "Hornos: %i",
			restauranteLocal->cantidadHornos);
	log_info(logger, "Cocineros: %i",
			restauranteLocal->cantCocineros);
	log_info(logger, "Platos: %i", restauranteLocal->cantPlatos);
	log_info(logger, "Nombre: %s", restauranteLocal->nombre);
	log_info(logger, "Nombre Size: %i",
			restauranteLocal->nombre_size);
	for (int i = 0; i < restauranteLocal->cantCocineros; i++) {
		log_info(logger, "Cocinero %i", i);
		t_cocinero_s* cocinero = (t_cocinero_s*) list_get(
				restauranteLocal->cocineros, i);
		log_info(logger, "ID: %i", cocinero->id);
		log_info(logger, "Afinidad Size: %i", cocinero->afinidad_size);
		log_info(logger, "Afinidad: %s", cocinero->afinidad);
	}
	for (int j = 0; j < restauranteLocal->cantPlatos; j++) {
		log_info(logger, "Plato %i", j);
		t_plato_s* plato = (t_plato_s*) list_get(restauranteLocal->platos, j);
		log_info(logger, "ID: %i", plato->id);
		log_info(logger, "Nombre Size: %i", plato->nombre_size);
		log_info(logger, "Nombre: %s", plato->nombre);
		log_info(logger, "Precio: %i", plato->precio);
	}
}

/**************************** FUNCIONES DE FINALIZACION ****************************/

void finalizarPlanificador(){
	log_info(logger, "Finalizando Planificador...");
	log_info(logger, "Set isExit en Planificador. [preValue: 1, postValue: 0]");
	isExit = 0;
}

void cerrarSemaforos(){
	sem_close(&semPlatosNew);
	sem_close(&semPlatosReady);
	sem_close(&semPlatosBlock);
	sem_close(&semPlatosExec);
	sem_close(&semPlatosExit);
	sem_close(&semCocinerosSinAfinidad);
	sem_close(&semQueueHornos);
	sem_close(&semCPUBurstExec);
	sem_close(&semCPUBurstBlock);
	sem_close(&semCPUBurstIO);
}

void liberarRecursosPlanificador(){

	freeMemHilos();

	// Vacio todas las colas de estados
	log_info(logger, "Vaciando Queues de Estado en Planificador...");
	vaciarQueues();

	// Libero los hornos
	log_info(logger, "Vaciando Hornos en Planificador...");
	list_iterate(listHornos, vaciarHornos);

	// Libero semaforos bloqueados
	log_info(logger, "Desbloqueando semaforos de Planficador...");
	cerrarSemaforos();

	log_info(logger, "Liberando recursos de Planficador...");
	freeMemSemaforos();
	freeMemQueues();
	freeMemListas();
}

void vaciarQueueAfinidad(t_queue_afinidad*  str_queue){
	queue_clean(str_queue->afinidadQueue);
	sem_close(str_queue->semaforo);
	sem_close(str_queue->cocineroLibreEnAfinidad);
}

void vaciarQueues(){
	queue_clean(queueNew);
	queue_clean(queueReady);
	queue_clean(queueExit);
	queue_clean(queueHornos);
	queue_clean(queueExec);
	queue_clean(queueBlock);
	list_iterate(listQueueAfinidades, vaciarQueueAfinidad);
}

void vaciarHornos(t_horno_io* horno){
	if(!horno->libre){
		destruirPCB(horno->plato);
		horno->libre = true;
	}
}

void freeMemPasosReceta(t_paso* paso){
	free(paso->nombre);
	free(paso);
}

void destruirPCB(t_pcb* plato){
	free(plato->receta->plato);
	list_destroy_and_destroy_elements(plato->receta->pasos, freeMemPasosReceta);
	free(plato->receta);
	free(plato->tipoBloqueo);
	free(plato);
}

void freeMemQueuesPorAfinidad(t_queue_afinidad* qAfinidad){
	free(qAfinidad->afinidad);
	sem_destroy(qAfinidad->semaforo);
	sem_destroy(qAfinidad->cocineroLibreEnAfinidad);
	free(qAfinidad->semaforo);
	free(qAfinidad->cocineroLibreEnAfinidad);
	queue_destroy(qAfinidad->afinidadQueue);
	free(qAfinidad);
}

void freeMemQueues(){
	queue_destroy(queueNew);
	queue_destroy(queueReady);
	queue_destroy(queueExit);
	queue_destroy(queueHornos);
	queue_destroy(queueExec);
	queue_destroy(queueBlock);

	list_destroy_and_destroy_elements(listQueueAfinidades, freeMemQueuesPorAfinidad);
}

void freeMemCocinero(t_cocinero_cpu* cocinero){
	free(cocinero->afinidad);
	free(cocinero);
}

void freeMemHorno(t_horno_io* horno){
	sem_destroy(horno->semaforo);
	free(horno->semaforo);
	if(horno->plato != NULL){
		destruirPCB(horno->plato);
	}
	free(horno);
}

void freeMemListas(){
	list_destroy_and_destroy_elements(listCocineros, freeMemCocinero);
	list_destroy_and_destroy_elements(listHornos, freeMemHorno);
}

void freeMemSemaforos(){
	sem_destroy(&semPlatosNew);
	sem_destroy(&semPlatosReady);
	sem_destroy(&semPlatosBlock);
	sem_destroy(&semPlatosExec);
	sem_destroy(&semPlatosExit);
	sem_destroy(&semCocinerosSinAfinidad);
	sem_destroy(&semQueueHornos);
	sem_destroy(&semCPUBurstExec);
	sem_destroy(&semCPUBurstBlock);
	sem_destroy(&semCPUBurstIO);

	pthread_mutex_destroy(&mutexQueueNew);
	pthread_mutex_destroy(&mutexQueueReady);
	pthread_mutex_destroy(&mutexQueueBlock);
	pthread_mutex_destroy(&mutexQueueHornos);
	pthread_mutex_destroy(&mutexQueueExec);
	pthread_mutex_destroy(&mutexQueueExit);
	pthread_mutex_destroy(&mutexCocinerosLibres);
}

void freeMemHilo(pthread_t* thread){
	pthread_cancel(*thread);
}

void freeMemHilos(){
	list_destroy_and_destroy_elements(threadsPlanificador, freeMemHilo);
}

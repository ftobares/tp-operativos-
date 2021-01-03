#include "planificador.h"
#include <commons/collections/list.h>
#include "app.h"
#include <unistd.h>

#define OCUPADO 0
#define LIBRE 1

// Hilo que está escuchando la llegada de nuevos pedidos
void* planificarPedidosNuevos(void *data){

	t_list* repartidoresQueEstanLibres;
	t_repartidor* repartidorMasCercano;

	while (1) {
		// Se espera a que llegue algún pedido
		sem_wait(&pedidosNuevos);

		// Se desencola un pedido
		pthread_mutex_lock(&mutexColaNew);
		t_PCB* pedido = list_get(colaNew,0);
		list_remove(colaNew,0);
		pthread_mutex_unlock(&mutexColaNew);

		log_debug(logger,"PEDIDO NUEVO: Se creó un nuevo pedido con ID: %d",pedido->id_pedido);
		// Se espera a que haya un repartidor libre
		sem_wait(&repartidoresLibres);

		// Se obtiene el más cercano al restaurante del pedido
		repartidoresQueEstanLibres = obtenerRepartidoresLibres();
		repartidorMasCercano = obtenerRepartidorMasCercano(repartidoresQueEstanLibres,pedido);
		list_destroy(repartidoresQueEstanLibres);

		// Asignamos el repartidor al pedido y lo marcamos como ocupado
		pedido->repartidor_asignado = repartidorMasCercano;
		repartidorMasCercano->libre = OCUPADO;
		repartidorMasCercano->pedidoAsignado = pedido;

		log_trace(logger,"ASIGNACION DE PEDIDO (NEW -> READY): Pedido con ID: %d asignado al Repartidor %d ubicado en %d|%d,"
				" del Restaurante %s ubicado en %d|%d para el Cliente ubicado en %d|%d",
				pedido->id_pedido, pedido->repartidor_asignado->id,
				pedido->repartidor_asignado->posicion->x,pedido->repartidor_asignado->posicion->y, pedido->restaurante,
				pedido->posicion_restaurante->x, pedido->posicion_restaurante->y,
				pedido->posicion_cliente->x, pedido->posicion_cliente->y);

		// Agregamos al pedido a listo
		pthread_mutex_lock(&mutexColaReady);
		list_add(colaReady, pedido);
		pthread_mutex_unlock(&mutexColaReady);

		sem_post(&pedidosListos);
	}
}

// Encolamos un pedido nuevo que fue confirmado en la App
t_PCB* planificarPedidoNuevo(t_pedido_en_creacion* pedidoEnCreacion, uint32_t socketCliente){

	pthread_mutex_lock(&mutexClientesConectados);
	t_cliente_app* cliente = obtenerClienteSegunSocket(socketCliente);
	pthread_mutex_unlock(&mutexClientesConectados);

	t_PCB* nuevoPedido = malloc(sizeof(t_PCB));
	nuevoPedido->id_pedido = pedidoEnCreacion->id_pedido;
	nuevoPedido->restaurante = strdup(pedidoEnCreacion->restaurante->nombre);
	nuevoPedido->id_cliente = strdup(cliente->idCliente);

	t_posicion* posicionCliente = malloc(sizeof(t_posicion));
	posicionCliente->x = cliente->posicion->x;
	posicionCliente->y = cliente->posicion->y;
	nuevoPedido->posicion_cliente = posicionCliente;
	t_posicion* posicionRestaurante = malloc(sizeof(t_posicion));
	posicionRestaurante->x = pedidoEnCreacion->restaurante->posicion->x;
	posicionRestaurante->y = pedidoEnCreacion->restaurante->posicion->y;
	nuevoPedido->posicion_restaurante = posicionRestaurante;

	// Datos de la planificación
	nuevoPedido->repartidor_asignado = NULL;
	nuevoPedido->tiempo_espera = 0;
	nuevoPedido->estimacion = ESTIMACION_INICIAL;
	nuevoPedido->ultimaRafaga = 0;
	nuevoPedido->motivoBloqueo = SIN_BLOQUEO;
	nuevoPedido->listo = false;

	pthread_mutex_lock(&mutexPedidosEnCreacion);
	eliminarPedidoEnCreacionSegunIdYRestaurante(pedidoEnCreacion->id_pedido, pedidoEnCreacion->restaurante->nombre);
	pthread_mutex_unlock(&mutexPedidosEnCreacion);

	pthread_mutex_lock(&mutexColaNew);
	list_add(colaNew,nuevoPedido);
	sem_post(&pedidosNuevos);
	pthread_mutex_unlock(&mutexColaNew);

	return nuevoPedido;
}

// Hilo que está escuchando la llegada de pedidos terminados
void* planificarPedidosTerminados(void *data){

	while(1){
		// Chequea que haya algun pedido en la cola de EXIT
		sem_wait(&pedidosTerminados);

		// Obtenemos un pedido
		pthread_mutex_lock(&mutexColaExit);
		t_PCB* pedido = list_get(colaExit,0);
		list_remove(colaExit,0);
		pthread_mutex_unlock(&mutexColaExit);

		// Avisamos a Comanda y a Cliente
		finalizarPedido(pedido);

		log_debug(logger,"EXIT: Finalizo el pedido con ID: %d del Restaurante %s", pedido->id_pedido, pedido->restaurante);

		// Se asigna repartidor como libre
		pedido->repartidor_asignado->libre = LIBRE;
		sem_post(&repartidoresLibres);

		// Se libera la memoria ocupada por el PCB
		free(pedido->id_cliente);
		free(pedido->restaurante);
		free(pedido->posicion_cliente);
		free(pedido->posicion_restaurante);
		free(pedido);
	}
}

// Hilo que está escuchando la llega de pedidos a cola de Ready
void* planificarPedidosListos(void *data){

	t_PCB* pedido;
	while(1){
		// Espera que el grado de exec sea menor al grado de multiprogramación (haya un core libre)
		sem_wait(&coresLibres);

		// En ese momento analiza la cola de ready (espera que haya algun elemento)
		sem_wait(&pedidosListos);

		pthread_mutex_lock(&mutexColaReady);
		if(string_equals_ignore_case(ALGORITMO_DE_PLANIFICACION,"SJF-SD") || string_equals_ignore_case(ALGORITMO_DE_PLANIFICACION,"HRRN"))
			estimarRafagas();
		pedido = obtenerProximoAEjecutar();
		pedido->ultimaRafaga = 0;
		pthread_mutex_unlock(&mutexColaReady);

		log_debug(logger,"READY: Pedido seleccionado para ejecutar - ID: %d del Restaurante: %s",pedido->id_pedido, pedido->restaurante);
		// Guardar el PCB en la lista de exec.
		pthread_mutex_lock(&mutexColaExec);
		list_add(listaExec,pedido);
		pthread_mutex_unlock(&mutexColaExec);

		// Al repartidor asignado al pedido indicarle que puede moverse
		sem_post(pedido->repartidor_asignado->enMovimiento);
	}
}

// Hilo que está escuchando la llegada de pedidos a cola de block
void* planificarPedidosBloqueados(void* data){

	t_PCB* pedido;
	pthread_t descansar;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	// Si se bloqueo porque esta descansando, descansa y vuelve a ready
	while(1){

		sem_wait(&pedidosBloqueados);

		pthread_mutex_lock(&mutexColaBlock);
		pedido = list_get(colaBlock,(colaBlock->elements_count)-1);
		pthread_mutex_unlock(&mutexColaBlock);

		log_debug(logger,"BLOCK: Pedido con ID: %d del Restaurante %s", pedido->id_pedido, pedido->restaurante);
		pthread_create(&descansar,&attr,repartidorBloqueado,(void*) pedido);
	}
}

void* relojPlanificador(void* data){

	void sumarCicloEspera(t_PCB* pedido){
		pedido->tiempo_espera += 1;
		log_info(logger, "READY: El pedido %d está esperando hace %d ciclos.",pedido->id_pedido,pedido->tiempo_espera);
	}
	void habilitarMovimientoRepartidor(t_PCB* pedido){
		t_repartidor* repartidor = pedido->repartidor_asignado;
		sem_post(repartidor->enMovimiento);
	}
	void descansarUnCicloRepartidor(t_PCB* pedido){
		t_repartidor* repartidor = pedido->repartidor_asignado;
		if(pedido->motivoBloqueo == REPARTIDOR_CANSADO){
			sem_post(pedido->repartidor_asignado->enDescanso);
		}
	}
	while(1){
		// Genera un retardo de CPU
		sleep(RETARDO_CICLO_CPU);

		log_info(logger,"======= NUEVO CICLO DE CPU =====");

		pthread_mutex_lock(&mutexColaNew);
		pthread_mutex_lock(&mutexColaExit);
		pthread_mutex_lock(&mutexColaBlock);
		pthread_mutex_lock(&mutexColaReady);
		pthread_mutex_lock(&mutexColaExec);

		// Para los pedidos en ready le suma un ciclo de espera
		list_iterate(colaReady,(void*) sumarCicloEspera);

		// Para los repartidores bloqueados descansando se les descuenta un ciclo
		list_iterate(colaBlock,(void*) descansarUnCicloRepartidor);

		// Para los repartidores que estan en exec se les habilita un movimiento
		list_iterate(listaExec,(void*) habilitarMovimientoRepartidor);

		pthread_mutex_unlock(&mutexColaNew);
		pthread_mutex_unlock(&mutexColaExit);
		pthread_mutex_unlock(&mutexColaBlock);
		pthread_mutex_unlock(&mutexColaReady);
		pthread_mutex_unlock(&mutexColaExec);
	}
}

void inicializarColas(){

	pthread_mutex_lock(&mutexColaNew);
	pthread_mutex_lock(&mutexColaExit);
	pthread_mutex_lock(&mutexColaBlock);
	pthread_mutex_lock(&mutexColaReady);
	pthread_mutex_lock(&mutexColaExec);

	colaNew = list_create();
	colaReady = list_create();
	listaExec = list_create();
	colaBlock = list_create();
	colaExit = list_create();

	pthread_mutex_unlock(&mutexColaNew);
	pthread_mutex_unlock(&mutexColaExit);
	pthread_mutex_unlock(&mutexColaBlock);
	pthread_mutex_unlock(&mutexColaReady);
	pthread_mutex_unlock(&mutexColaExec);
}

void liberarColas(){
	void pcb_destroy(t_PCB* pedido){
		free(pedido->id_cliente);
		free(pedido->restaurante);
		free(pedido->posicion_cliente);
		free(pedido->posicion_restaurante);
		free(pedido);
	}

	pthread_mutex_lock(&mutexColaNew);
	pthread_mutex_lock(&mutexColaExit);
	pthread_mutex_lock(&mutexColaBlock);
	pthread_mutex_lock(&mutexColaReady);
	pthread_mutex_lock(&mutexColaExec);

	if(colaNew != NULL) list_destroy_and_destroy_elements(colaNew,(void*)pcb_destroy);
	if(colaReady != NULL) list_destroy_and_destroy_elements(colaReady,(void*)pcb_destroy);
	if(listaExec != NULL) list_destroy_and_destroy_elements(listaExec,(void*)pcb_destroy);
	if(colaBlock != NULL) list_destroy_and_destroy_elements(colaBlock,(void*)pcb_destroy);
	if(colaExit != NULL) list_destroy_and_destroy_elements(colaExit,(void*)pcb_destroy);

	pthread_mutex_unlock(&mutexColaNew);
	pthread_mutex_unlock(&mutexColaExit);
	pthread_mutex_unlock(&mutexColaBlock);
	pthread_mutex_unlock(&mutexColaReady);
	pthread_mutex_unlock(&mutexColaExec);
}

void liberarRepartidores(){

	void repartidor_destroy(t_repartidor* repartidor){
		sem_destroy(repartidor->enMovimiento);
		sem_destroy(repartidor->enDescanso);
		sem_destroy(repartidor->enEspera);
		free(repartidor->enMovimiento);
		free(repartidor->enDescanso);
		free(repartidor->enEspera);
		free(repartidor->posicion);
		free(repartidor);
	}
	if(repartidores != NULL) list_destroy_and_destroy_elements(repartidores,(void*)repartidor_destroy);
}

void liberarRestauranteDefault(){
	void plato_destroy(char* plato){
		free(plato);
	}
	if(restoDefault != NULL){
		free(restoDefault->nombre);
		free(restoDefault->posicion);
		list_destroy_and_destroy_elements(restoDefault->platos,(void*) plato_destroy);
		free(restoDefault->socketRestaurante);
		free(restoDefault);
	}
}

void liberarHilos(){
	void thread_destroy(pthread_t* thread){
		pthread_cancel(*thread);
		free(thread);
		sleep(1);
	}
	list_destroy_and_destroy_elements(threads, (void*)thread_destroy);
	sleep(1);
}

void liberarSemaforosPlanificador(){
	sem_destroy(&pedidosNuevos);
	sem_destroy(&pedidosListos);
	sem_destroy(&pedidosTerminados);
	sem_destroy(&pedidosBloqueados);
	sem_destroy(&repartidoresLibres);
	sem_destroy(&coresLibres);
	pthread_mutex_destroy(&mutexColaNew);
	pthread_mutex_destroy(&mutexColaReady);
	pthread_mutex_destroy(&mutexColaBlock);
	pthread_mutex_destroy(&mutexColaExec);
	pthread_mutex_destroy(&mutexColaExit);
}

void* repartidorBloqueado(void *data){

	t_PCB* pedido = (t_PCB* ) data;
	t_bloqueo motivoBloqueo = pedido->motivoBloqueo;

	bool tieneIdYRestaurante(t_PCB* pedidoLista){
		return pedidoLista->id_pedido == pedido->id_pedido && string_equals_ignore_case(pedidoLista->restaurante,pedido->restaurante);
	}

	if(motivoBloqueo == REPARTIDOR_CANSADO)
	{
		int tiempoDescanso = pedido->repartidor_asignado->tiempo_descanso;
		for(int i=0; i<tiempoDescanso;i++){
			log_info(logger,"REPARTIDOR %d: Le quedan por descansar %d ciclos de CPU", pedido->repartidor_asignado->id, tiempoDescanso-i);
			sem_wait(pedido->repartidor_asignado->enDescanso);
		}
		pedido->motivoBloqueo = SIN_BLOQUEO;

		pthread_mutex_lock(&mutexColaBlock);
		list_remove_by_condition(colaBlock,(void*)tieneIdYRestaurante);
		pthread_mutex_unlock(&mutexColaBlock);

		pthread_mutex_lock(&mutexColaReady);
		list_add(colaReady,pedido);
		pthread_mutex_unlock(&mutexColaReady);

		log_trace(logger, "REPARTIDOR %d: BLOCK -> READY. Motivo: el repartidor terminó su tiempo de descanso.", pedido->repartidor_asignado->id);
	}
	if(motivoBloqueo == PEDIDO_SIN_TERMINAR){
		// Espera a que el pedido esté listo
		sem_wait(pedido->repartidor_asignado->enEspera);

		pedido->motivoBloqueo = SIN_BLOQUEO;
		pthread_mutex_lock(&mutexColaBlock);
		list_remove_by_condition(colaBlock,(void*)tieneIdYRestaurante);
		pthread_mutex_unlock(&mutexColaBlock);

		pthread_mutex_lock(&mutexColaReady);
		list_add(colaReady,pedido);
		pthread_mutex_unlock(&mutexColaReady);
		log_trace(logger, "REPARTIDOR %d: BLOCK -> READY, Motivo: el restaurante terminó el pedido, y el repartidor ya puede llevarselo al cliente.", pedido->repartidor_asignado->id);
	}
	pedido->tiempo_espera = 0;
	sem_post(&pedidosListos);
	pthread_exit(NULL);
}

void* entregarPedidos(void* data){
	t_repartidor* repartidor = (t_repartidor *) data;

	while(1){
		// Espera a que le asignen el pedido
		sem_wait(repartidor->enMovimiento);

		// Estado EXEC
		t_PCB* pedido = (t_PCB*) repartidor->pedidoAsignado;

		log_trace(logger, "REPARTIDOR %d: READY -> EXEC. Motivo: el planificador le ha asignado un procesador disponible.", repartidor->id);
		log_info(logger, "REPARTIDOR %d: Se encuentra en posicion %d|%d. Se moverá hacia el restaurante ubicado en %d|%d",
				repartidor->id,repartidor->posicion->x, repartidor->posicion->y,
				pedido->posicion_restaurante->x, pedido->posicion_restaurante->y);

		// Comienza el viaje hacia el restaurante
		recorrerCamino(repartidor,pedido->posicion_restaurante);

		log_debug(logger, "REPARTIDOR %d: Llegó al restaurante a retirar el pedido.",repartidor->id);

		// Si es restaurante default, el pedido está listo. Si no, espera a que lo esté.
		if(!esRestauranteDefault(pedido->restaurante)){
			if(!estaListo(pedido)){
				pedido->motivoBloqueo = PEDIDO_SIN_TERMINAR;
				bool tieneIdYRestaurante(t_PCB* pedidoLista){
					return pedidoLista->id_pedido == pedido->id_pedido && string_equals_ignore_case(pedidoLista->restaurante,pedido->restaurante);
				}
				pthread_mutex_lock(&mutexColaExec);
				list_remove_by_condition(listaExec,(void*) tieneIdYRestaurante);
				pthread_mutex_unlock(&mutexColaExec);

				pthread_mutex_lock(&mutexColaBlock);
				list_add(colaBlock,pedido);
				pthread_mutex_unlock(&mutexColaBlock);

				log_trace(logger, "REPARTIDOR %d: EXEC -> BLOCK, Motivo: el repartidor debe esperar a que el Restaurante termine de preparar el pedido.", repartidor->id);
			 	sem_post(&pedidosBloqueados);
				sem_post(&coresLibres);
			}
		}
		// Debe esperar a que termine el retardo (si está en exec) o que lo planifiquen (si está en ready).
		sem_wait(repartidor->enMovimiento);

		// A esta sección de código llega cuando el pedido ya está listo y se puede entregar al cliente.
		log_info(logger, "REPARTIDOR %d: Se encuentra en posicion %d|%d. Se moverá hacia el cliente ubicado en %d|%d",
						repartidor->id,repartidor->posicion->x, repartidor->posicion->y,
						pedido->posicion_cliente->x, pedido->posicion_cliente->y);


		recorrerCamino(repartidor,pedido->posicion_cliente);
		log_debug(logger, "REPARTIDOR %d: Llegó a la posición del cliente. Se entregará el pedido...",repartidor->id);

		bool tieneIdYRestaurante(t_PCB* pedidoLista){
			return pedidoLista->id_pedido == pedido->id_pedido && string_equals_ignore_case(pedidoLista->restaurante,pedido->restaurante);
		}
		pthread_mutex_lock(&mutexColaExec);
		list_remove_by_condition(listaExec,(void*) tieneIdYRestaurante);
		pthread_mutex_unlock(&mutexColaExec);

		repartidor->pedidoAsignado = NULL;

		pthread_mutex_lock(&mutexColaExit);
		list_add(colaExit,pedido);
		pthread_mutex_unlock(&mutexColaExit);

		log_trace(logger, "ENTREGA DE PEDIDO: El repartidor %d entregó el pedido %d del Restaurante %s al cliente %s.",repartidor->id, pedido->id_pedido, pedido->restaurante, pedido->id_cliente);
		log_trace(logger, "REPARTIDOR %d: EXEC -> EXIT. Motivo: Entregó un pedido.",repartidor->id);

		sem_post(&pedidosTerminados);
		sem_post(&coresLibres);
	}
}

void recorrerCamino(t_repartidor* repartidor, t_posicion* destino){

	t_PCB* pedido = repartidor->pedidoAsignado;
	int movimientosDisponibles = repartidor->frecuencia_descanso;

	// Mientras no haya llegado a destino
	while(!llegoADestino(repartidor, destino)){

		// Ejecuta todos sus movimientos (no hay desalojo en ningún algoritmo)
		while(movimientosDisponibles > 0){
			log_info(logger, "REPARTIDOR %d: Al repartidor le quedan %d movimientos antes de descansar.",repartidor->id, movimientosDisponibles);
			moverseHacia(repartidor,destino);
			movimientosDisponibles--;
			pedido->ultimaRafaga = pedido->ultimaRafaga + 1;
			log_info(logger, "=== PEDIDO %d - Lleva rafaga de %d ciclos", pedido->id_pedido, pedido->ultimaRafaga);
			if(llegoADestino(repartidor, destino))
				break;
			sem_wait(repartidor->enMovimiento);
		}

		// Si todavía no llegó al destino...
		if(!llegoADestino(repartidor, destino)){
			// Se asigna el tiempo de descanso
			pedido->motivoBloqueo = REPARTIDOR_CANSADO;

			bool tieneIdYRestaurante(t_PCB* pedidoLista){
				return pedidoLista->id_pedido == pedido->id_pedido && string_equals_ignore_case(pedidoLista->restaurante,pedido->restaurante);
			}
			pthread_mutex_lock(&mutexColaExec);
			list_remove_by_condition(listaExec,(void*) tieneIdYRestaurante);
			pthread_mutex_unlock(&mutexColaExec);

			pthread_mutex_lock(&mutexColaBlock);
			list_add(colaBlock,pedido);
			pthread_mutex_unlock(&mutexColaBlock);

			log_trace(logger, "REPARTIDOR %d: EXEC -> BLOCK. Motivo: el repartidor está cansado y debe descansar %d ciclos.", repartidor->id, repartidor->tiempo_descanso);
			sem_post(&pedidosBloqueados);
			sem_post(&coresLibres);

			// Descansa y reinicia su cantidad de movimientos disponibles
			sem_wait(repartidor->enMovimiento);
			movimientosDisponibles = repartidor->frecuencia_descanso;
		}
	}
}

int crearRepartidores(){

	void cadena_destroy(char* cadena){
		free(cadena);
	}
	void posicion_destroy(t_posicion* posicion){
		free(posicion);
	}

	t_list* posiciones = parseListaPosiciones(config->repartidores);
	t_list* frecuencias = parseLista(config->frecuencia_descanso);
	t_list* tiempos = parseLista(config->tiempo_descanso);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	int cantidadRepartidores = list_size(posiciones);
	repartidores = list_create();

	for(int i=0; i<cantidadRepartidores; i++){
		t_repartidor* repartidor = malloc(sizeof(t_repartidor));

		t_posicion* posicion = list_get(posiciones,i);

		repartidor->posicion = malloc(sizeof(t_posicion));
		repartidor->posicion->x = posicion->x;
		repartidor->posicion->y = posicion->y;
		repartidor->frecuencia_descanso = atoi(list_get(frecuencias,i));
		repartidor->tiempo_descanso = atoi(list_get(tiempos,i));
		repartidor->libre = 1;
		repartidor->id = (i+1);
		repartidor->pedidoAsignado = NULL;

		// Se crea un semáforo por repartidor para asignar sus movimientos
		repartidor->enMovimiento = malloc(sizeof(sem_t));
		sem_t enMovimiento;
		if(sem_init(&enMovimiento,1,0) != 0) return -1;
		*(repartidor->enMovimiento) = enMovimiento;

		// Se crea un semáforo por repartidor para asignar sus descanso
		repartidor->enDescanso = malloc(sizeof(sem_t));
		sem_t enDescanso;
		if(sem_init(&enDescanso,1,0) != 0) return -1;
		*(repartidor->enDescanso) = enDescanso;

		// Se crea un semáforo por repartidor para manejar la espera ante un pedido que no está listo
		repartidor->enEspera = malloc(sizeof(sem_t));
		sem_t enEspera;
		if(sem_init(&enEspera,1,0) != 0) return -1;
		*(repartidor->enEspera) = enEspera;

		// Se abre un hilo por repartidor
		pthread_t hilo;

		if(pthread_create(&hilo,&attr,entregarPedidos,(void*) repartidor) != 0){
			log_error(logger,"Hubo un error al crear un hilo de repartidor.");
			list_destroy_and_destroy_elements(posiciones,(void*)posicion_destroy);
			list_destroy_and_destroy_elements(frecuencias,(void*)cadena_destroy);
			list_destroy_and_destroy_elements(tiempos,(void*)cadena_destroy);
			return -1;
		}

		pthread_t* tr = malloc(sizeof(pthread_t));
		*tr = hilo;
		pthread_mutex_lock(&mutexListaThreads);
		list_add(threads,tr);
		pthread_mutex_unlock(&mutexListaThreads);

		list_add(repartidores,repartidor);
	}

	list_destroy_and_destroy_elements(posiciones,(void*)posicion_destroy);
	list_destroy_and_destroy_elements(frecuencias,(void*)cadena_destroy);
	list_destroy_and_destroy_elements(tiempos,(void*)cadena_destroy);
	return 0;
}

void crearRestauranteDefault(){

	restoDefault = malloc(sizeof(t_restaurante_app));
	// Nombre
	char* nombre = string_new();
	string_append(&nombre,"DEFAULT");
	restoDefault->nombre = nombre;

	// Platos
	restoDefault->platos = parseLista(config->platos_default);

	// Posicion
	t_posicion* posicion = malloc(sizeof(t_posicion));
	posicion->x = config->posicion_x_default;
	posicion->y = config->posicion_y_default;
	restoDefault->posicion = posicion;

	// Socket
	restoDefault->socketRestaurante = NULL;
}

void asignarValoresPlanificacion(){

	RETARDO_CICLO_CPU = config->retardo_cpu;
	GRADO_DE_MULTIPROCESAMIENTO = config->multiprocesamiento;
	ALGORITMO_DE_PLANIFICACION = strdup(config->algoritmo_planificacion);
	ALPHA = config->alpha;
	ESTIMACION_INICIAL = config->estimacion_inicial;
}

int abrirHilosPlanificador(){

	pthread_t cola_new, cola_exit, cola_ready, cola_block;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	// Seteo que no se espere a los hilos
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	if(pthread_create(&cola_new,&attr,planificarPedidosNuevos,config) != 0){
		log_error(logger,"Hubo un error al crear el hilo de planificador de pedidos nuevos");
		return -1;
	}
	if(pthread_create(&cola_ready,&attr,planificarPedidosListos,config) != 0){
		log_error(logger,"Hubo un error al crear el hilo de planificador de pedidos listos.");
		return -1;
	}
	if(pthread_create(&cola_block,&attr,planificarPedidosBloqueados,config) != 0){
		log_error(logger,"Hubo un error al crear el hilo de planificador de pedidos bloqueados.");
		return -1;
	}
	if(pthread_create(&cola_exit,&attr,planificarPedidosTerminados,config) !=0){
		log_error(logger,"Hubo un error al crear el hilo de planificador de pedidos finalizados.");
		return -1;
	}

	// Guardamos los hilos para poder liberarlos al final
	pthread_t hilos[4] = {cola_new, cola_exit, cola_block, cola_ready};
	pthread_mutex_lock(&mutexListaThreads);
	for(int i=0; i<4; i++){
		pthread_t* t = malloc(sizeof(pthread_t));
		*t = hilos[i];
		list_add(threads,t);
	}
	pthread_mutex_unlock(&mutexListaThreads);
	return 0;
}

int iniciarReloj(){
	pthread_t reloj;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	// Seteo que no se espere a los hilos
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	if(pthread_create(&reloj,&attr,relojPlanificador,config) !=0){
		log_error(logger,"Hubo un error al crear el hilo del reloj del planificador.");
		return -1;
	}
	pthread_t* t = malloc(sizeof(pthread_t));
	*t = reloj;

	pthread_mutex_lock(&mutexListaThreads);
	list_add(threads,t);
	pthread_mutex_unlock(&mutexListaThreads);

	return 0;
}

int iniciarPlanificador(){
	inicializarColas();
	log_info(logger, "INIT 3/3: Colas de planificación inicializadas con éxito.");
	if(crearRepartidores() != 0) return -1;
	log_info(logger, "INIT 3/3: Repartidores obtenidos con éxito.");
	crearRestauranteDefault();
	log_info(logger, "INIT 3/3: Restaurante Default cargado con éxito.");
	asignarValoresPlanificacion();
	log_info(logger, "INIT 3/3: Parámetros de configuración cargados con éxito.");
	if(inicializarSemaforosPlanificador() != 0) return -1;
	log_info(logger, "INIT 3/3: Sincronización creada con éxito.");
	if(abrirHilosPlanificador() != 0) return -1;
	if(iniciarReloj() != 0) return -1;
	log_info(logger, "INIT 3/3: Planificador esperando por la llegada de PCBs.");
	return 0;
}

int inicializarSemaforosPlanificador(){
	if(sem_init(&pedidosNuevos,1,0) != 0) return -1;
	if(sem_init(&pedidosListos,1,0) != 0) return -1;
	if(sem_init(&pedidosTerminados,1,0) != 0) return -1;
	if(sem_init(&pedidosBloqueados,1,0) != 0) return -1;
	if(sem_init(&repartidoresLibres,1,repartidores->elements_count) != 0) return -1;
	if(sem_init(&coresLibres,1,GRADO_DE_MULTIPROCESAMIENTO) != 0) return -1;
	if(pthread_mutex_init(&mutexColaNew,NULL) != 0) return -1;
	if(pthread_mutex_init(&mutexColaReady,NULL) != 0) return -1;
	if(pthread_mutex_init(&mutexColaBlock,NULL) != 0) return -1;
	if(pthread_mutex_init(&mutexColaExec,NULL) != 0) return -1;
	if(pthread_mutex_init(&mutexColaExit,NULL) != 0) return -1;
	return 0;
}

t_PCB* obtenerProximoAEjecutar(){
	t_PCB* pedido;

	if(string_equals_ignore_case(ALGORITMO_DE_PLANIFICACION, "FIFO")){
		pedido = obtenerProximoFIFO();
	}
	else if(string_equals_ignore_case(ALGORITMO_DE_PLANIFICACION, "HRRN")){
		pedido = obtenerProximoHRRN();
	}
	else{
		pedido = obtenerProximoSJF();
	}

	return pedido;
}

// FIFO
t_PCB* obtenerProximoFIFO(){
	t_PCB* pedido = list_get(colaReady,0);
	list_remove(colaReady,0);
	return pedido;
}

// HRRN
t_PCB* obtenerProximoHRRN(){
	t_PCB* pedido = obtenerPedidoRRMasAlto();
	eliminarPedidoSegunIdYRestaurante(colaReady,pedido->id_pedido, pedido->restaurante);
	return pedido;

}

double calculoHRRN(t_PCB* pcb){
	log_info(logger, "HRRN: Pedido %d - Estimacion de %f ciclos - Tiempo de espera de %d ciclos",pcb->id_pedido,pcb->estimacion,pcb->tiempo_espera);
	return (pcb->estimacion + pcb->tiempo_espera)/ pcb->estimacion;
}

t_PCB* obtenerPedidoRRMasAlto(){
	t_PCB* masAlto = list_get(colaReady,0);

	double rrMasAlto = calculoHRRN(masAlto);
	for(int i=1; i<list_size(colaReady); i++){
		t_PCB* pedido = list_get(colaReady,i);
		double rrActual = calculoHRRN(pedido);
		if(rrActual > rrMasAlto){
			rrMasAlto = rrActual;
			masAlto = pedido;
		}
	}
	log_debug(logger, "HRRN: El pedido %d es el de RATIO mas grande con %f.", masAlto->id_pedido, rrMasAlto);
	return masAlto;
}

// SJF
t_PCB* obtenerProximoSJF(){
	t_PCB* pedido = obtenerPedidoConEstimacionSJFmasCorta();
	eliminarPedidoSegunIdYRestaurante(colaReady,pedido->id_pedido,pedido->restaurante);
	log_debug(logger, "SJF: El pedido %d es el más corto con una estimacion de %f ciclos.", pedido->id_pedido, pedido->estimacion);
	return pedido;
}

t_PCB* obtenerPedidoConEstimacionSJFmasCorta(){
	// Pongo al primer como el mas cercano, despues itero sobre el resto buscando una distancia menor
	t_PCB* masCorto = list_get(colaReady,0);
	double rafagaMasCorta = masCorto->estimacion;
	for(int i=1; i<list_size(colaReady); i++){
		t_PCB* pedido = list_get(colaReady,i);
		double rafaga = pedido->estimacion;
		if(rafaga < rafagaMasCorta){
			rafagaMasCorta = rafaga;
			masCorto = pedido;
		}
	}
	return masCorto;
}

// Estimador
double estimarProximaRafaga(double estimadoAnterior, double realAnterior){
	return estimadoAnterior * ALPHA + realAnterior * (1 - ALPHA);
}

void estimarRafagas(){
	void actualizarEstimacion(t_PCB* pedido){
		log_info(logger, "Estimador: Pedido %d - Ultima estimacion: %f ciclos. Rafaga anterior: %d ciclos.", pedido->id_pedido, pedido->estimacion, pedido->ultimaRafaga);
		if(pedido->ultimaRafaga == 0) // Aún no se ejecutaron
			pedido->estimacion = ESTIMACION_INICIAL;
		else
			pedido->estimacion = estimarProximaRafaga(pedido->estimacion,pedido->ultimaRafaga);
		log_info(logger, "Estimador: Pedido %d se estima rafaga de %f ciclos.", pedido->id_pedido, pedido->estimacion);
	}
	log_info(logger, "Estimador: Comienza estimacion de rafagas para los pedidos en READY.");
	list_iterate(colaReady,(void*) actualizarEstimacion);
}

// Utilidades
void moverseHacia(t_repartidor* repartidor, t_posicion* posicion){
	bool seMovio = false;

	// Movimiento en X
	if(repartidor->posicion->x != posicion->x){
		if(posicion->x < repartidor->posicion->x)
			repartidor->posicion->x -= 1;
		else
			repartidor->posicion->x += 1;
		seMovio = true;
	}

	// Movimiento en Y
	if(!seMovio && repartidor->posicion->y != posicion->y){
		if(posicion->y < repartidor->posicion->y)
			repartidor->posicion->y -= 1;
		else
			repartidor->posicion->y += 1;
	}
	log_trace(logger, "REPARTIDOR %d: Se movió a la posicion %d|%d", repartidor->id, repartidor->posicion->x, repartidor->posicion->y);
}

bool estaListo(t_PCB* pedido){
	return pedido->listo;
}

bool llegoADestino(t_repartidor* repartidor, t_posicion* destino){
	return calcularDistancia(repartidor->posicion,destino) == 0;
}

bool esRestauranteDefault(char* restaurante){
	return string_equals_ignore_case(restaurante,restoDefault->nombre);
}

bool esElMismoCliente(char* idClientePedido, char* idClienteEncontrado){
	return string_equals_ignore_case(idClientePedido, idClienteEncontrado);
}

double calcularDistancia(t_posicion* inicio, t_posicion* fin){

	int x = fin->x - inicio->x;
	int y = fin->y - inicio->y;
	return sqrt(pow(x,2) + pow(y,2));
}

t_repartidor* obtenerRepartidorMasCercano(t_list* repartidoresLibres, t_PCB* pedido){

	// Pongo al primer como el mas cercano, despues itero sobre el resto buscando una distancia menor
	t_repartidor* masCercano = list_get(repartidoresLibres,0);
	double distanciaMasCercana = calcularDistancia(masCercano->posicion, pedido->posicion_restaurante);
	for(int i=1; i<list_size(repartidoresLibres); i++){
		t_repartidor* repartidor = list_get(repartidoresLibres,i);
		double distancia = calcularDistancia(repartidor->posicion,pedido->posicion_restaurante);
		if(distancia < distanciaMasCercana){
			distanciaMasCercana = distancia;
			masCercano = repartidor;
		}
	}
	return masCercano;
}

t_list* obtenerRepartidoresLibres(){
	bool estaLibre(t_repartidor* repartidor){
		return repartidor->libre == 1;
	}
	return list_filter(repartidores,(void*)estaLibre);
}

void eliminarPedidoSegunIdYRestaurante(t_list* lista, uint32_t idPedido, char* nombreRestaurante){
	bool tieneId(t_PCB* pedido){
		return pedido->id_pedido == idPedido && string_equals_ignore_case(pedido->restaurante,nombreRestaurante);
	}
	list_remove_by_condition(lista,(void*)tieneId);
}

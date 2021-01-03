#include "memoria.h"

t_frame* leerFrame(void* memoria, uint32_t posicion) {
    t_frame* frame = (t_frame*)(memoria + (posicion * sizeof(t_frame)));
    return (t_frame*)(memoria + (posicion * sizeof(t_frame)));  
}
t_frame* leerFrameDeSwap(uint32_t nroFrame) { 
    return leerFrame(swap, nroFrame); 
}
t_frame* leerFrameDeMP(uint32_t nroFrame) { 
    // nroFrame es sobre SWAP
    // busco el indice en MP en base al nroFrame
    int posicion = -1;
    int cantidadFrames = list_size(framesMp) - 1 ;
    bool encontro = false;
    t_info_frame* infoFrame;
    while (!encontro && cantidadFrames > posicion) {
        posicion += 1; 
        infoFrame = list_get(framesMp, posicion);
        encontro = infoFrame->nroFrame == nroFrame;
    }

    if (!encontro) return NULL;

    if (algoritmo == 0) {
        // LRU
        // llevo el frame al final de la lista porque lo lei
        int filtro(int* i) { return *i == posicion; };
        int* pLru = list_remove_by_condition(lru, filtro);
        list_add(lru, pLru);
    } else {
        // CLOCK_MEJ
        // marco el bit de uso
        infoFrame->uso = 1;
    }
    
    return leerFrame(memoriaPrincipal, posicion); 
}

t_frame* leerFrameDeMpByIndice(uint32_t indice) {
    return leerFrame(memoriaPrincipal, indice);
}

t_frame* leerFrameDeMPoSwap(uint32_t nroFrame) {
    t_frame* frame = leerFrameDeMP(nroFrame);
    if (frame == NULL) {
        log_info(logger, "Fallo de pagina. Se busca en SWAP...");
        // traigo de swap
        frame = leerFrameDeSwap(nroFrame);

        // y escribo en MP
        if (escribirFrameEnMp(frame, nroFrame) == EXIT_FAILURE) return NULL;
        frame = leerFrameDeMP(nroFrame);
    } 
    return frame;
}

t_info_frame* obtenerInfoFrameSwap(uint32_t nroFrame) {
    return obtenerInfoFrame(framesSwap, nroFrame); 
}
t_info_frame* obtenerInfoFrameMp(uint32_t nroFrame) { 
    return obtenerInfoFrame(framesMp, nroFrame); 
}
t_info_frame* obtenerInfoFrame(t_list* frames, uint32_t nroFrame) {
    int filtro(t_info_frame* infoFrame) {
        return infoFrame->nroFrame == nroFrame;
    }
    return (t_info_frame*)list_find(frames, (void*)filtro);
}

int finalizarManejoDeMemoria() {
    
    void liberarLru(int* indice) { free(indice); }
    void liberarPagina(t_pagina* pagina) { free(pagina); }
    void liberarSegmento(t_segmento* segmento) {
        list_destroy_and_destroy_elements(segmento->tablaPaginas, liberarPagina);
        free(segmento);
    }
    void liberarInfoFrame(t_info_frame* infoFrame) { free(infoFrame); }
    void liberarTablaSegmento(t_tabla_segmentos* tablaSegmentos) { 
        list_destroy_and_destroy_elements(tablaSegmentos->segmentos, liberarSegmento); 
        //free(tablaSegmentos->nombreRestaurante); 
        free(tablaSegmentos);
    }
    
    free(memoriaPrincipal); 
    close(swapfd);

    
    list_destroy_and_destroy_elements(tablasDeSegmentos, liberarTablaSegmento);
    list_destroy_and_destroy_elements(framesMp, liberarInfoFrame);
    list_destroy_and_destroy_elements(framesSwap, liberarInfoFrame);
    list_destroy_and_destroy_elements(lru, liberarLru);
    return EXIT_SUCCESS;
}

int inicializarManejoDeMemoria(uint32_t tamanioMp, uint32_t tamanioSwap, char* pAlgoritmo, t_log* pLogger) {

    logger = pLogger;

    if (string_equals_ignore_case(pAlgoritmo, "LRU")) {
        algoritmo = 0;
    } else if(string_equals_ignore_case(pAlgoritmo, "CLOCK_MEJ")) {
        algoritmo = 1;
        
    } else {
        perror("Algoritmo de reemplazo no reconocido. Debe ser 'LRU' o 'CLOCK_MEJ'");
        return EXIT_FAILURE;
    }

    punteroClock = 0;
    
    memoriaPrincipal = malloc(tamanioMp);
    if (memoriaPrincipal == NULL) return EXIT_FAILURE;

    log_info(logger, "Memoria principal creada correctamente (%p)", memoriaPrincipal);
    // creo listas de estructuras auxiliares
    tablasDeSegmentos = list_create();
    framesMp = list_create();
    framesSwap = list_create();
    lru= list_create();

    TAMANIO_FRAME = sizeof(t_frame);
	MAXIMA_CANTIDAD_FRAME_MP = tamanioMp / TAMANIO_FRAME;
    MAXIMA_CANTIDAD_FRAME_SWAP = tamanioSwap / TAMANIO_FRAME;

    swapfd = open("swap", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);	//open for R&W and user has read and write permission //,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR

	if(swapfd == -1) return EXIT_FAILURE;
	
    ftruncate(swapfd, tamanioSwap);

    struct stat st = swaplong(swapfd);
    swap = mmap(NULL,st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED,swapfd,0);	//st.st_size
    
    log_info(logger, "SWAP creado correctamente (%p)", swap);

    if(swap == MAP_FAILED) perror("MMAP");

    return EXIT_SUCCESS;
}

int escribirFrame(void* memoria, t_frame* frame, uint32_t posicion) {
    // creo el frame   

    log_info(logger, "Se escribe frame en posición %d", posicion);
    if (memoria == memoriaPrincipal) {
        log_info(logger, "Se escribe frame en memoria principal. Desplazamiento %x. Ubicación final: %p", posicion * sizeof(t_frame), memoriaPrincipal + (posicion * sizeof(t_frame)));        
    } else if (memoria == swap){
        log_info(logger, "Se escribe frame en SWAP. Desplazamiento %x. Ubicación final: %p", posicion * sizeof(t_frame), swap + (posicion * sizeof(t_frame)));
    }

    log_info(logger, "Datos del frame | Comida: %s | Cantidad %d | Cantidad listo %d", frame->comida, frame->cantidad, frame->cantidadLista);
    t_frame* frameEscribir = (t_frame*)(memoria + (posicion * sizeof(t_frame)));

    frameEscribir->cantidad = frame->cantidad;
    frameEscribir->cantidadLista = frame->cantidadLista;    
    strncpy(frameEscribir->comida, frame->comida, 24);    
    return EXIT_SUCCESS;
}

int obtenerVictimaReemplazo() {
    // IMPORTANTE: devuelve INDICE de frame en MP que tiene que ser reemplazado
    // NO nroFrame (que es indice en SWAP)

    int victima = -1;
    if (algoritmo == 0) {
        // LRU
        // la victima va a ser el primero de la lista
        // la lista siempre va a tener algo porque valide previamente
        int* pVictima = list_remove(lru, 0);
        victima = *pVictima;
        free(pVictima);
    } else {
        // CLOCK
        int i = 0;
        void paso1() {
            i = 0;
            while (victima == -1 && i < list_size(framesMp)) {
                t_info_frame* infoFrame = list_get(framesMp, punteroClock);
                if (!infoFrame->uso && !infoFrame->modificado) {
                    victima = punteroClock;
                } else {
                    avanzarPunteroClock();
                }
                i +=1;
            }
        }

        void paso2() {
            i = 0;
            while (victima == -1 && i < list_size(framesMp)) {
                t_info_frame* infoFrame = list_get(framesMp, punteroClock);
                if (!infoFrame->uso) {
                    victima = punteroClock;
                } else {
                    infoFrame->uso = 0;
                    avanzarPunteroClock();
                }
                i +=1;
            }
        }

        while (victima == -1) {
            paso1();
            if (victima == -1) {
                paso2();
            }
        }        
    }

    t_frame* frame = leerFrameDeMpByIndice(victima);
    log_info(logger, "Victima seleccionada '%s', cantidad: %d, cantidad lista: %d", frame->comida, frame->cantidad, frame->cantidadLista);
    return victima;
}

void avanzarPunteroClock() {
    if ((punteroClock + 1) == MAXIMA_CANTIDAD_FRAME_MP) {
        // vuelvo al inicio de la lista
        punteroClock = 0;
    } else {
        punteroClock += 1;
    }
}

int agregarInfoFrame(t_list* listaFrames, uint32_t nroFrame) {
    t_info_frame* infoFrame = malloc(sizeof(t_info_frame));

    infoFrame->nroFrame = nroFrame;
    if (algoritmo == 1) {
        infoFrame->uso = 1;
        infoFrame->modificado = 0;
    }

    list_add(listaFrames, infoFrame);
}

void llevarFrameASwap(uint32_t nroFrameMp) {
    // traigo el nro de frame (SWAP) en base al nro de frame en MP
    t_info_frame* infoFrame = list_get(framesMp, nroFrameMp);
    uint32_t nroFrame = infoFrame->nroFrame;
    
    // leo frames y actualizo valores
    t_frame* frameSwap = leerFrameDeSwap(nroFrame);
    t_frame* frameMp = leerFrameDeMpByIndice(nroFrameMp);
    strcpy(frameSwap->comida,frameMp->comida);
    frameSwap->cantidad = frameMp->cantidad;
    frameSwap->cantidadLista = frameMp->cantidadLista;
}

int escribirFrameEnMp(t_frame* frame, uint32_t nroFrame) {
    int ubicacionGuardar;
    
    if (MAXIMA_CANTIDAD_FRAME_MP == list_size(framesMp)) {

        t_info_frame* infoFrame = NULL;
        for(int i = 0; i < list_size(framesMp); i++) { 
            t_info_frame* infoFrameI = list_get(framesMp, i);
            if (infoFrameI->nroFrame == UINT32_MAX) {
                infoFrame = infoFrameI;
                ubicacionGuardar = i;
            } 
        }
        
        if (infoFrame == NULL) {
            // memoria esta llena, hay que buscar victima
            log_info(logger, "Memoria principal se encuentra llena. Se inicia el proceso de reemplazo...");
            ubicacionGuardar = obtenerVictimaReemplazo();

            // chequeo si el frame a reemplazar fue modificado
            infoFrame = list_get(framesMp, ubicacionGuardar);
            if (infoFrame->modificado == 1) {
                llevarFrameASwap(ubicacionGuardar);
            }
        }

        // actualizo datos por el nuevo nro de frame que pongo en MP
        infoFrame->nroFrame = nroFrame;
        infoFrame->uso = 1;
        infoFrame->modificado = 0;

    } else {
        // hay lugar todavia, guardo en el proximo lugar disponible
        ubicacionGuardar = list_size(framesMp);
        agregarInfoFrame(framesMp, nroFrame);
    }

    if (algoritmo == 0) {
        // LRU
        int* pLru = malloc(sizeof(int));
        *pLru = ubicacionGuardar;
        list_add(lru, pLru);
    } else {
        // CLOCK_MEJ
        avanzarPunteroClock();
    }

    return escribirFrame(memoriaPrincipal, frame, ubicacionGuardar);
}

int escribirFrameEnSwap(t_frame* frame, uint32_t nroFrame) {

    if (MAXIMA_CANTIDAD_FRAME_SWAP == list_size(framesSwap)) { return EXIT_FAILURE;}
    int resultEscribirFrame = escribirFrame(swap, frame, nroFrame);
    if (resultEscribirFrame == 0) {
        agregarInfoFrame(framesSwap, nroFrame);
    }
    return resultEscribirFrame;
}

int buscarNroFrameDisponible() {
    uint32_t nroFrame = -1;
    uint32_t i = 0;
    while (nroFrame == -1 && i != MAXIMA_CANTIDAD_FRAME_SWAP) {
        t_info_frame* infoFrame = obtenerInfoFrameSwap(i);
        if (infoFrame == NULL) {
            // no existe frame con nro {i}
            nroFrame = i;
        } else {
            i +=1;
        }
    }
    return nroFrame;
}

t_tabla_segmentos* obtenerTablaDeSegmentos(char* nombreRestaurante){
    int filtro(t_tabla_segmentos* tablaSegmentos) {
        return string_equals_ignore_case(tablaSegmentos->nombreRestaurante, nombreRestaurante);
    }
    
    // busco la tabla de segmentos para el restaurante
    t_tabla_segmentos* tablaSegmentos = list_find(tablasDeSegmentos, (void*)filtro);
    return tablaSegmentos;
}

t_tabla_segmentos* obtenerCrearTablaSegmentos(char* nombreRestaurante) {
    // busco la tabla de segmentos para el restaurante
    t_tabla_segmentos* tablaSegmentos = obtenerTablaDeSegmentos(nombreRestaurante);

    // si no existe la creo
    if (tablaSegmentos == NULL) {
        tablaSegmentos = malloc(sizeof(t_tabla_segmentos));
        if (tablaSegmentos == NULL) {
            return NULL;
        }
        
        tablaSegmentos->nombreRestaurante = nombreRestaurante;
        tablaSegmentos->segmentos = list_create();
        list_add(tablasDeSegmentos, tablaSegmentos);
    }
    return tablaSegmentos;
}

t_segmento* obtenerSegmento(t_tabla_segmentos* tablaSegmentos, uint32_t idPedido) {    
    int filtro(t_segmento* segmento) { return segmento->idPedido == idPedido; }

    // busco si ya existe el segmento para el idPedido
    t_segmento* segmento = list_find(tablaSegmentos->segmentos, (void*)filtro);

    return segmento;
}

t_segmento* obtenerCrearSegmento(t_tabla_segmentos* tablaSegmentos, uint32_t idPedido) {
    t_segmento* segmento = obtenerSegmento(tablaSegmentos, idPedido);

    if(segmento == NULL) {
        // no existe segmento, lo creo
        segmento = malloc(sizeof(t_segmento));
        if (segmento == NULL) return NULL;
        segmento->idPedido = idPedido;
        segmento->estadoPedido = PEDIDO_PENDIENTE;
        // creo la tabla de paginas
        segmento->tablaPaginas = list_create();
        list_add(tablaSegmentos->segmentos, segmento);
    }
    
    return segmento;
}

t_segmento* buscarSegmento(char* nombreRestaurante, uint32_t idPedido, t_tabla_segmentos* tablaSegmentos) {
    int filtro(t_segmento* segmento) { return segmento->idPedido == idPedido; }

    // busco si ya existe el segmento para el idPedido
    t_segmento* segmento = list_find(tablaSegmentos->segmentos, (void*)filtro);

    if(segmento == NULL) {
        // no existe segmento, aviso
        return NULL;
    }
    
    return segmento;
}

int crearPagina(t_segmento* segmento, char nombrePlato[24], int cantidad){
    t_pagina* pagina = malloc(sizeof(t_pagina));
    
    if (pagina == NULL) return EXIT_FAILURE;

    int nroFrame = buscarNroFrameDisponible();

    // no hay mas espacio disponible
    if (nroFrame == -1 ) return EXIT_FAILURE;
    
    pagina->nroFrame = nroFrame;
    list_add(segmento->tablaPaginas, pagina);

    t_frame* frame = malloc(sizeof(t_frame));
    frame->cantidad = cantidad;
    frame->cantidadLista = 0;
    strncpy(frame->comida, nombrePlato, 24);

    // escribo frame en swap
    int result = escribirFrameEnSwap(frame, nroFrame);
    if (result != 0) {  
        free(frame);
        return result; 
    }

    result = escribirFrameEnMp(frame, nroFrame);
    free(frame);
    return result;
}


t_pagina* obtenerPagina(t_segmento* segmento, char nombrePlato[24]) {    
    // la tabla de paginas esta vacia, evito recorrer
    if (list_size(segmento->tablaPaginas) == 0) return NULL;

    t_pagina* pagina = NULL;

    // busco si existe el plato
    int paginasSize = list_size(segmento->tablaPaginas);
    int i = 0;

    // recorro los frames y me fijo si existe el plato
    while (pagina == NULL && i < paginasSize) {
        pagina = list_get(segmento->tablaPaginas,i);
        t_frame* frame = leerFrameDeSwap(pagina->nroFrame);
        if (!string_equals_ignore_case(frame->comida, nombrePlato)) {
            pagina = NULL;
            i += 1;
        }   
    }
    
    // si no existe, es null
    return pagina;
}

int crearPaginaSiNoExiste(t_segmento* segmento, char nombrePlato[24], int cantidad) {
    // busco si existe la pagina
    t_pagina* pagina = obtenerPagina(segmento, nombrePlato);
    if (pagina == NULL) {
        return crearPagina(segmento, nombrePlato,cantidad);
    } else {
        // ya existe la pagina
        t_frame* frame = leerFrameDeMPoSwap(pagina->nroFrame);
        frame->cantidad += cantidad;
        t_info_frame* infoFrame = obtenerInfoFrameMp(pagina->nroFrame);
        infoFrame->modificado = 1;
        return EXIT_SUCCESS;
    }
}

struct stat swaplong(int fileDescriptor){
    struct stat st;
    if(fstat(swapfd,&st) == -1){
        perror("Error al utilizar fstat");
    }
    return st;
}


void liberarPaginasDeSegmento(t_segmento* segmento) {
    int i = 0;
    void liberarFrame(uint32_t nroFrame) {

		int limpiarFrame(t_info_frame* infoFrame) {
            log_info(logger, "Se elimina frame de memoria principal. Posicion frame: %p", memoriaPrincipal + (i * sizeof(t_frame)) );
			if (infoFrame->nroFrame == nroFrame) {
                infoFrame->nroFrame = UINT32_MAX;
            }
		}

        int buscarFrame(t_info_frame* infoFrame) {
            log_info(logger, "Se elimina frame de SWAP principal. Posicion frame: %p", swap + (nroFrame * sizeof(t_frame)) );
			return infoFrame->nroFrame == nroFrame;
		}

		list_iterate(framesMp, &limpiarFrame);
		list_remove_and_destroy_by_condition(framesSwap, buscarFrame, &free);
	}

    void liberarPagina(t_pagina* pagina) {
		liberarFrame(pagina->nroFrame);
		free(pagina); 
	}

    list_destroy_and_destroy_elements(segmento->tablaPaginas, liberarPagina);
}
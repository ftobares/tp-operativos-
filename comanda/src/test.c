char* resto = "bodegon";

char comida[24] = "milanesa";
char comida2[24] = "ravioles";
char comida3[24] = "pollo";
char comida4[24] = "pescado";
char comida5[24] = "ensalada";
char comida6[24] = "tequeños";

int idPedido = 0;
int idPedido1 = 1;
int idPedido2 = 2;

int cantidad = 10;
void abrirMemorias() {
    printf("SWAP:\n");
    int i  = 0;
    for(i = 0; i<list_size(framesSwap); i++) {
        t_frame* frame = leerFrameDeSwap(i);
        printf("COMIDA: %s | CANTIDAD: %d | CANTIDAD LISTA: %d \n", frame->comida, frame->cantidad, frame->cantidadLista);
    }
    printf("________________\n");
    printf("MP:\n");
    for(i = 0; i<list_size(framesMp); i++) {
        t_frame* frame = leerFrameDeMpByIndice(i);
        printf("COMIDA: %s | CANTIDAD: %d | CANTIDAD LISTA: %d \n", frame->comida, frame->cantidad, frame->cantidadLista);
    }
    printf("=====================\n");
}
void mostrarDatosClock() {
    for(int i = 0;i<list_size(framesMp); i++){
        t_info_frame* infoFrame = list_get(framesMp, i);
        t_frame* frame = leerFrameDeSwap(infoFrame->nroFrame);
        printf("COMIDA: %s, USO: %d, MODIFICADO: %d\n",frame->comida, infoFrame->uso, infoFrame->modificado);
    }
    printf("=====================\n");
}

void checkTest(bool result) {
    if (result) printf("TEST OK\n");
    else printf("TEST FALLÓ\n");
}

bool test1() {
    if (guardarPedido(resto, idPedido) != 0) printf("ERROR al guardar pedido\n");
    if (guardarPedido(resto, idPedido1) != 0) printf("ERROR al guardar pedido\n");

    t_tabla_segmentos* ts = obtenerTablaDeSegmentos(resto);
    if (ts == NULL){
        printf("ERROR al crear/obtener segmento");
        return false;
    }
    if (list_size(ts->segmentos) == 2){
        return true;
    } else {
        return false;
    }
}

bool test2() {
    algoritmo = 0;
    test1(); 
    guardarPlato(resto, idPedido, comida, 10);
    guardarPlato(resto, idPedido, comida2, 20);
    guardarPlato(resto, idPedido, comida3, 20);
    guardarPlato(resto, idPedido, comida4, 20);    
    if (list_size(framesMp) != list_size(framesSwap)) {
        return false;
    }

    int victima = obtenerVictimaReemplazo();
    t_frame* frame = leerFrameDeMpByIndice(victima);

    if (!string_equals_ignore_case(frame->comida, comida)) {
        return false;
    }
    
    // leo comida2 para marcarla como leida
    leerFrameDeMP(1);

    victima = obtenerVictimaReemplazo();
    frame= leerFrameDeMpByIndice(victima);

    // victima deberia ser comida3
    if (!string_equals_ignore_case(frame->comida, comida3)) {
        return false;
    }
    return true;    
}

bool test3() {

    algoritmo = 1;
    test1();

    guardarPlato(resto, idPedido, comida, 10);
    guardarPlato(resto, idPedido, comida2, 20);
    guardarPlato(resto, idPedido, comida3, 20);
    guardarPlato(resto, idPedido, comida4, 20);

    t_frame* frame = leerFrameDeMpByIndice(punteroClock);

    if (!string_equals_ignore_case(frame->comida, comida)) {
        return false;
    }

    
    int victima = obtenerVictimaReemplazo(); // bits de uso pasan a 0
    avanzarPunteroClock();

    // pone bit de uso en 1
    leerFrameDeMP(1);

    // consulta el frame nro 1 asi que el bit de uso pasa a ser 1
    t_info_frame* infoFrame = list_get(framesMp, 1);
    if (infoFrame->uso != 1) {
        return false;
    }

    // el puntero esta en comida2 ya que avanzo por leer comida1
    frame = leerFrameDeMpByIndice(punteroClock);
    if (!string_equals_ignore_case(frame->comida, comida2)) {
        return false;
    }

    // victima va a ser la 3 ya que la 2 tiene el bit de uso activos
    victima = obtenerVictimaReemplazo();
    avanzarPunteroClock();
    frame = leerFrameDeMpByIndice(victima);
    if (!string_equals_ignore_case(frame->comida, comida3)) {
        return false;
    }

    return true;
}

bool test4() {
    test1();
    algoritmo = 0;
    guardarPlato(resto, idPedido, comida, 10);
    guardarPlato(resto, idPedido, comida2, 20);
    guardarPlato(resto, idPedido, comida3, 20);
    guardarPlato(resto, idPedido, comida4, 20);
    
    int victima = 0; // por ser LRU va a ser 0
    int nuevoValor = 2708;
    // marco el flag modificado de la futura victima
    t_info_frame* infoFrame = list_get(framesMp, victima);
    infoFrame->modificado = 1;

    // cambio el valor para poder comprobar
    t_frame* frame = leerFrameDeMpByIndice(victima);

    frame->cantidad = nuevoValor;

    int nuevoNroFrame = 5;
    // escribo nuevo frame para forzar reemplazo
    frame = malloc(sizeof(t_frame));
    frame->cantidad= 101;
    frame->cantidadLista=202;
    escribirFrameEnMp(frame, nuevoNroFrame);

    // leo el frame a ver si tiene el nuevo valor
    frame= leerFrameDeSwap(victima);

    if (frame->cantidad != nuevoValor) {
        // el cambio se perdio, algo salio mal
        return false;
    }
    
    // abrirMemorias();
    return true;
}

bool test5() {
    test1();

    guardarPlato(resto, idPedido, comida, 10);
    guardarPlato(resto, idPedido, comida2, 20);
    guardarPlato(resto, idPedido, comida3, 20);
    guardarPlato(resto, idPedido, comida4, 20);

    t_obtener_pedido_s* pedido = obtenerPedido(resto, idPedido);

    t_obtener_pedido_plato_s* plato;
    
    if (list_size(pedido->platos) != 4) {
        return false;
    }

    plato = list_get(pedido->platos, 0);
    if (!string_equals_ignore_case(plato->comida, comida)) return false;

    plato = list_get(pedido->platos, 1);
    if (!string_equals_ignore_case(plato->comida, comida2)) return false;

    plato = list_get(pedido->platos, 2);
    if (!string_equals_ignore_case(plato->comida, comida3)) return false;

    plato = list_get(pedido->platos, 3);
    if (!string_equals_ignore_case(plato->comida, comida4)) return false;
    
    return true;
}

bool test6() {
    test1();

    guardarPlato(resto, idPedido, comida, cantidad);
    
    t_tabla_segmentos* tablaSegmentos = obtenerTablaDeSegmentos(resto);

	if (tablaSegmentos == NULL) {
		return EXIT_FAILURE;
	}
	
	t_segmento* segmento = obtenerSegmento(tablaSegmentos, idPedido);
    if (segmento->estadoPedido != PEDIDO_PENDIENTE) {
        return false;
    }
    
    confirmarPedido(resto, idPedido);
    if (segmento->estadoPedido != PEDIDO_CONFIRMADO) {
        return false;
    }
    return true;
}

bool test7() {

    if (!test6()) return false;

    t_tabla_segmentos* tablaSegmentos = obtenerTablaDeSegmentos(resto);
    t_segmento* segmento = obtenerSegmento(tablaSegmentos, idPedido);

    for(int i = 0;i<cantidad - 1; i++) {

        platoListo(resto, idPedido, comida);
        if (segmento->estadoPedido != PEDIDO_CONFIRMADO) {
            // el pedido se termino antes de lo que deberia
            return false;
        }
    }

    // listo el ultimo
    platoListo(resto, idPedido, comida);
    if (segmento->estadoPedido != PEDIDO_TERMINADO) {
        return false;
    }
    return true;
}

bool test8() {
    
    test1();
    guardarPedido(resto, idPedido2);
    
    guardarPlato(resto, idPedido, comida, 10);
    guardarPlato(resto, idPedido, comida2, 20);
    guardarPlato(resto, idPedido, comida3, 20);

    guardarPlato(resto, idPedido1, comida4, 1111);

    guardarPlato(resto, idPedido1, comida3, 2222);

    guardarPlato(resto, idPedido2, comida, 123123);

    t_tabla_segmentos* ts = obtenerTablaDeSegmentos(resto);

    if (list_size(framesMp) != 4 || list_size(framesSwap) != 4) {
        //return false;
    }

    if (list_size(ts->segmentos) != 2) {
        //return false;
    }

    //abrirMemorias();

    finalizarPedido(resto, idPedido);

    // abrirMemorias();

    t_obtener_pedido_s* pedido;
    
    pedido = obtenerPedido(resto, idPedido1);
    //mostrarRespuestaObtenerPedido(pedido);

    pedido = obtenerPedido(resto, idPedido2);
    //mostrarRespuestaObtenerPedido(pedido);
 
    if (list_size(framesSwap) != 3) {
        return false;
    }

    t_segmento* segmento = list_get(ts->segmentos, 0);
    if (segmento->idPedido != idPedido1) return false;
    
    return true;
}

void mostrarRespuestaObtenerPedido(t_obtener_pedido_s* pedido) {
    
    printf("cantidad platos: %d", pedido->cantidadPlatos);
    printf("\nPlatos: \n");
    for(int i = 0; i<pedido->cantidadPlatos; i++) {
        t_obtener_pedido_plato_s* plato = list_get(pedido->platos, i);
        printf("Nombre: %s; Cantidad: %d; CantidadLista: %d\n", plato->comida, plato->cantidad, plato->cantidadLista);
    }
    printf("=====================\n");
}
bool test9() {
    char* asado = "AsadoConFritas";
    char* bondiolita  = "Bondiolita";
    char* choripan  = "Choripan";
    char* morcipan = "Morcipan";
    int i ;
    t_obtener_pedido_s* pedido ;

    int result  = 0;
    result += guardarPedido(resto, 1);
    result += guardarPedido(resto, 2);
    result += guardarPedido(resto, 3);
    result += guardarPedido(resto, 4);
    result += guardarPedido(resto, 5);
    result += guardarPedido(resto, 6);
    result += guardarPedido(resto, 7);
    if  (result != 0) return false;

    result = 0;
    result += guardarPlato(resto, 1, asado, 1);
    result += guardarPlato(resto, 2, asado, 2);
    result += guardarPlato(resto, 5, bondiolita, 1);
    result += guardarPlato(resto, 3, asado, 1);
    result += guardarPlato(resto, 6, bondiolita, 4);

    if (result != 0) return false;

    pedido = obtenerPedido(resto, 1);
    if (pedido == NULL) return false;
    //mostrarRespuestaObtenerPedido(pedido);

    if (guardarPlato(resto, 4, choripan, 20) != 0) return false;

    pedido = obtenerPedido(resto, 2);
    if (pedido  == NULL) return false;
    //mostrarRespuestaObtenerPedido(pedido);

    result = 0;
    result += guardarPlato(resto, 7, morcipan, 1);
    result += guardarPlato(resto, 1, asado, 1);
    result += guardarPlato(resto, 3, asado, 1);
    
    if(result != 0) return false;

    pedido = obtenerPedido(resto, 3);
    if (pedido  == NULL) return false;
    // mostrarRespuestaObtenerPedido(pedido);
    
    result = 0;
    for(i = 1; i<=7;i++) {
        result += confirmarPedido(resto, i);
    }

    if (result != 0) return false;
    
    result = 0;
    result += platoListo(resto, 1, asado);
    result += platoListo(resto, 2, asado);
    result += platoListo(resto, 3, asado);
    result += platoListo(resto, 1, asado);
    result += platoListo(resto, 2, asado);

    result += platoListo(resto, 5, bondiolita);

    for(i = 0; i<4; i++) {
        result += platoListo(resto, 6, bondiolita);
    }
    
    for(i = 0; i<20; i++) {
        result += platoListo(resto, 4, choripan);
    }
    
    result += platoListo(resto, 7, morcipan);

    if (result != 0) return false;
    
    result = 0;
    result += finalizarPedido(resto, 1);
    result += finalizarPedido(resto, 2);
    result += finalizarPedido(resto, 3);
    result += finalizarPedido(resto, 4);
    result += finalizarPedido(resto, 5);
    result += finalizarPedido(resto, 6);
    result += finalizarPedido(resto, 7);
    
    if (result != 0) return false;


    return true;
}

bool testComanda() {

    printf("\nIngrese opción a testear:\n");
    printf("1. Guardar pedido (crear tabla de segmentos\n");
    printf("2. Algoritmo LRU\n");
    printf("3. Algoritmo CLOCK_MEJ\n");
    printf("4. Modificación en MP\n");
    printf("5. Obtener pedido\n");
    printf("6. Confirmar pedido\n");
    printf("7. Plato listo\n");
    printf("8. Finalizar pedido\n");
    printf("9. Pruebas entrega final\n");

    char* linea = leerlinea();
    int opcionTestear = atoi(linea);
    free(linea);
    
    switch(opcionTestear) {
        case 1: {
            checkTest(test1());
            return true;
        } 
        case 2:{ 
            checkTest(test2());
            return true;
        } 
        case 3:{
            checkTest(test3());
            return true;
        } 
        case 4: {
            checkTest(test4());
            return true; 
        }
        case 5: {
            checkTest(test5());
            return true;
        }
        case 6: {
            checkTest(test6());
            return true;
        }
        case 7: {
            checkTest(test7());
            return true;
        }
        case 8: {
            checkTest(test8());
            return true;
        }
        case 9: {
            checkTest(test9());
            return true;
        }
    }

    return false;
}
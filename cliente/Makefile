## Para correrlo desde Eclipse
 # Agregar en Run Configurations.. -> Enviroment el valor
 # LD_LIBRARY_PATH = "${workspace_loc}/utiles"

## Para correrlo desde terminal, setear la variable 
# export LD_LIBRARY_PATH = ${ruta_donde_este_el_proyecto}/utiles (En esa ruta se genera la lib compartida)

TARGET = cliente
CC = gcc
UTILS_PATH = "../utiles"
LIBS = -lcommons -lutiles -lreadline -lpthread
CFLAGS = -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF

all: $(TARGET)
	
$(TARGET): utiles cliente.o funciones.o consola.o conexiones.o actualizaciones.o
	$(CC) -L$(UTILS_PATH) -o  "cliente"  ./src/cliente.o ./src/funciones.o ./src/consola.o ./src/conexiones.o ./src/actualizaciones.o $(LIBS)
 
cliente.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/cliente.o" -o "src/cliente.o" "src/cliente.c" 

funciones.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/funciones.o" -o "src/funciones.o" "src/funciones.c" 

actualizaciones.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/actualizaciones.o" -o "src/actualizaciones.o" "src/actualizaciones.c" 

consola.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/consola.o" -o "src/consola.o" "src/consola.c" 

conexiones.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/conexiones.o" -o "src/conexiones.o" "src/conexiones.c" 

utiles:
	cd $(UTILS_PATH) && make clean && make all

.PHONY: default all clean

clean:
	-rm -f src/*.o src/*.d
	-rm -f $(TARGET)
	cd ../utiles && rm -f *.o && rm -f libutiles.so
	
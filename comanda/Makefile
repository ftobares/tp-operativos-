## Para correrlo desde Eclipse
 # Agregar en Run Configurations.. -> Enviroment el valor
 # LD_LIBRARY_PATH = "${workspace_loc}/utiles"

## Para correrlo desde terminal, setear la variable 
# export LD_LIBRARY_PATH = ${ruta_donde_este_el_proyecto}/utiles (En esa ruta se genera la lib compartida)

TARGET = comanda
CC = gcc
UTILS_PATH = "../utiles"
LIBS = -lcommons -lutiles -lreadline -lpthread
CFLAGS = -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF

all: $(TARGET)
	
$(TARGET): utiles comanda.o funciones.o memoria.o
	$(CC) -L$(UTILS_PATH) -o  "comanda"  ./src/comanda.o ./src/funciones.o ./src/memoria.o $(LIBS)
 
comanda.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/comanda.o" -o "src/comanda.o" "src/comanda.c" 

funciones.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/funciones.o" -o "src/funciones.o" "src/funciones.c" 

memoria.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/memoria.o" -o "src/memoria.o" "src/memoria.c" 

utiles:
	cd $(UTILS_PATH) && make clean && make all

.PHONY: default all clean

clean:
	-rm -f src/*.o src/*.d
	-rm -f $(TARGET)
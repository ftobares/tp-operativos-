TARGET = app
CC = gcc
UTILS_PATH = "../utiles"
LIBS = -lcommons -lutiles -lreadline -pthread -lm -lpthread -lrt
CFLAGS = -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF

all: $(TARGET)
	
$(TARGET): utiles estructuras.o logic.o planificador.o app.o
	$(CC) -L$(UTILS_PATH) -o  "app" ./src/logic.o ./src/planificador.o ./src/app.o $(LIBS)

app.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/app.o" -o "src/app.o" "src/app.c" 

planificador.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/planificador.o" -o "src/planificador.o" "src/planificador.c" 

logic.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/logic.o" -o "src/logic.o" "src/logic.c" 

estructuras.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/estructuras.o" -o "src/estructuras.o" "src/estructuras.h" 

utiles:
	cd $(UTILS_PATH) && make

.PHONY: default all clean

clean:
	-rm -f src/*.o src/*.d
	-rm -f $(TARGET)
	cd ../utiles && rm -f *.o && rm -f libutiles.so
	

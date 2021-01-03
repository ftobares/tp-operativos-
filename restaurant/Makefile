TARGET = restaurant
CC = gcc
UTILS_PATH = "../utiles"
LIBS = -lcommons -lutiles -lreadline -lpthread
CFLAGS = -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF

all: $(TARGET)
	
$(TARGET): utiles restaurant.o planificador.o server.o logic.o utils.o
	$(CC) -L$(UTILS_PATH) -o "restaurant" ./src/restaurant.o ./src/planificador.o ./src/server.o ./src/logic.o ./src/utils.o $(LIBS)
 
restaurant.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/restaurant.o" -o "src/restaurant.o" "src/restaurant.c" 

planificador.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/planificador.o" -o "src/planificador.o" "src/planificador.c"

server.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/server.o" -o "src/server.o" "src/server.c"

logic.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/logic.o" -o "src/logic.o" "src/logic.c"

utils.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/utils.o" -o "src/utils.o" "src/utils.c"
	
utiles:
	cd $(UTILS_PATH) && make

.PHONY: default all clean

clean:
	-rm -f src/*.o src/*.d
	-rm -f $(TARGET)

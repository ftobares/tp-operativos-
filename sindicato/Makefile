TARGET = sindicato
CC = gcc
UTILS_PATH = "../utiles"
LIBS = -lcommons -lutiles -lreadline -pthread
CFLAGS = -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF

all: $(TARGET)
	
$(TARGET): utiles sindicato.o console.o server.o fileSystem.o fs_metadata.o logic.o utils.o
	$(CC) -L$(UTILS_PATH) -o "sindicato" ./src/sindicato.o ./src/console.o ./src/server.o ./src/fileSystem.o ./src/fs_metadata.o ./src/logic.o ./src/utils.o $(LIBS)

sindicato.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/sindicato.o" -o "src/sindicato.o" "src/sindicato.c"

console.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/console.o" -o "src/console.o" "src/console.c"

server.o:
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/server.o" -o "src/server.o" "src/server.c"

fileSystem.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/fileSystem.o" -o "src/fileSystem.o" "src/fileSystem.c"

fs_metadata.o: 
	$(CC) -I$(UTILS_PATH) $(CFLAGS) "src/fs_metadata.o" -o "src/fs_metadata.o" "src/fs_metadata.c"

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
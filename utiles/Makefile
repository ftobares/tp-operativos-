TARGET = libutiles.so
CC = gcc
LIBS = -lreadline -lcommons
CFLAGS = -c -g -Wall -fPIC
SHARED = -shared -g
OBJECTS = utiles.o utiles_config.o sockets.o serializer.o models.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(SHARED) -o libutiles.so $(OBJECTS) 

utiles.o:
	$(CC) $(CFLAGS) utiles.c $(LIBS)

utiles_config.o:
	$(CC) $(CFLAGS) utiles_config.c $(LIBS)

sockets.o:
	$(CC) $(CFLAGS) sockets.c $(LIBS)

serializer.o:
	$(CC) $(CFLAGS) serializer.c $(LIBS)

models.o:
	$(CC) $(CFLAGS) models.c $(LIBS)

.PHONY: default all clean

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	

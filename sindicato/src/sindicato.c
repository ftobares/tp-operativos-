#include "sindicato.h"

int main(void) 
{
	iniciar_logger();
	iniciar_config();

	pthread_t hiloServer;
	pthread_attr_t attrServer;
	pthread_attr_init(&attrServer);
	pthread_attr_setdetachstate(&attrServer,PTHREAD_CREATE_DETACHED);

	if (pthread_create(&hiloServer, &attrServer, runServer, NULL) != 0) 
	{
		log_error(logger, "Error creando el hilo de server");
	}
	
	if (runFileSystem() != 0)
	{
		log_error(logger, "Error al inicializar file system");
	}

	/*
	pthread_t hiloFileSystem;
	pthread_attr_t attrFileSystem;
	pthread_attr_init(&attrFileSystem);
	pthread_attr_setdetachstate(&attrFileSystem,PTHREAD_CREATE_DETACHED);

	if (pthread_create(&hiloFileSystem, &attrFileSystem, runFileSystem, NULL) != 0) 
	{
		log_error(logger, "Error creando el hilo de File System");
	}*/

	runConsole();

	pthread_cancel(hiloServer);
	sleep(1);
	//pthread_cancel(hiloFileSystem);

	finalizar();

	return 0;
}
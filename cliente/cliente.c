/*
 * cliente.c
 *
 *  Created on: 29/8/2017
 *      Author: utnso
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>



struct addrinfo* getInfoDeServer(char* ip, char* numeroPuerto){

	/*
		Creamos dos estructuras "addrinfo" que vamos a necesitar, una de configuracion y otra donde vamos a guardar la info del server al que
		nos queremos conectar
	*/
	struct addrinfo configuracion;
	struct addrinfo *serverInfo;

	/*
		Inicializamos la configuracion
	*/
	memset(&configuracion, 0, sizeof(configuracion));	// Inicializa todas las variables de la estructura en 0, para evitar errores por contener basura
	configuracion.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	configuracion.ai_socktype = SOCK_STREAM;	// Indica que usamos Socket Stream

	/*
		GetAddrinfo nos obtiene información que necesitamos del servidor
		Le pasamos como parametro una IP, un numero de Puerto, la configuracion que setteamos anteriormente y un struct donde nos va a guardar la info que buscamos
	*/
	int valor_getaddrinfo = getaddrinfo(ip, numeroPuerto, &configuracion, &serverInfo);

	/*
		Si no pudimos obtener info del server, no nos podemos conectar :(
	*/
    if (valor_getaddrinfo) {
    	perror("Error en getAddrInfo(): ");
        exit(1);
    }

    return serverInfo;
}

int crearSocketApuntandoAlServer(struct addrinfo *serverInfo){

	// File Descriptor del socket que vamos a crear, acordate que los FD son enteros, asi que los guardamos en un int :D
	int serverSocket;

	/*
		La funcion socket nos va a crear un socket en base a la información que le pasemos.
		En este caso, le pasamos el struct addrinfo que tiene la info del server
	*/
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	//Veriicamos que hayamos podido crear el socket ..
	if(serverSocket == -1) {
		//No se pudo crear el socket :(
		printf("Error de Socket Servidor");
		exit(1);
	}

	return serverSocket; //Pudimos crear el socket!
}

void conectarAlServer (int serverSocket, struct addrinfo *serverInfo){

	/*
		La funcion connect nos permite realizar la conexion mediante el socket y la info del server que generamos
	*/
	int valor_connect = connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);


	//Si no pudimos conectarnos ....
	if(valor_connect == -1) {
		printf("Error de Conexion");
		exit(1);
	}
}


int getConexionAlServer(char* ip, char* puerto){

	struct addrinfo* serverInfo = getInfoDeServer(ip,puerto);
	int serverSocket = crearSocketApuntandoAlServer(serverInfo);
	conectarAlServer(serverSocket, serverInfo);
	return serverSocket; //es el socket listo que necesitamos para poder comunicarnos con el server.

}


int main(){
	char* ip = "127.0.0.1";
	char* puerto = "2550";

	int fdServer = getConexionAlServer(ip, puerto);
	int cantCaracteres = 256;
	int tamMensaje = sizeof(char)*cantCaracteres;

	printf("Ya me conecte - fdServer %d \n",fdServer);

	char* cadena = malloc(tamMensaje);

	while(1){
		fgets(cadena, tamMensaje, stdin);
		send(fdServer, cadena, tamMensaje,0);
	}

	return EXIT_SUCCESS;
}



/*
 * server.c
 *
 *  Created on: 29/8/2017
 *      Author: utnso
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>



int crearSocketListener()
{
	int listener; //el int que vamos a usar como fd

	/*
	Creamos un socket con la llamada a "socket" con los parametros:
	AF_INET: Usamos ipv4
	SOCK_STREAM: socket de tipo stream
	0: Protocolo que usaremos, por default lo ponemos en 0;
	*/
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error creando el socket listener");
		exit(1);
	}


	return listener; //Retornamos el listener que necesitamos.
}

void socket_setsockopt(int listener)
{	int yes = 1;
	// obviar el mensaje "address already in use" (la dirección ya se está usando)
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))== -1) {
		perror("setsockopt");
		exit(1);
	}
}

void socket_bind(int listener, int port)
{
	//Info que necesitamos para settear el listener

	struct sockaddr_in myaddr;
	myaddr.sin_family = AF_INET; //Protocolo
	myaddr.sin_addr.s_addr = INADDR_ANY; //Indicamos IPv
	myaddr.sin_port = htons(port); //Setteamos el puerto

	memset(&(myaddr.sin_zero), '\0', 8);


	/*
	Bindeamos el listener con las opciones setteadas en el struct sockaddr
	*/
	if (bind(listener, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}

}

void socket_listen(int listener, int cantClientesEnEspera)
{
	/*
	Le indicamos al socket que comience a escuchar conexiones de cliente. Si puede hacerlo, ya tenemos todo listo!
	El segundo parametro es la cantidad de clientes que
	*/
	if (listen(listener, cantClientesEnEspera) == -1) {
		perror("listen");
		exit(1);
	}
}

int startListener(int puerto, int cantClientesEscucha)
{
	int listener;

	// obtener socket a la escucha
	listener = crearSocketListener();

	//setearSocket
	socket_setsockopt(listener);

	// enlazar
	socket_bind(listener, puerto);

	// escuchar
	socket_listen(listener, cantClientesEscucha);

	return listener;
}

int aceptarNuevaConexion(int listener){
	int addrlen;
	struct sockaddr_in remoteaddr; // dirección del cliente
	int newfd;        // descriptor de socket de nueva conexión aceptada
	// gestionar nuevas conexiones
	addrlen = sizeof(remoteaddr);

	if ((newfd = accept(listener,(struct sockaddr *) &remoteaddr, &addrlen)) == -1) {
		perror("accept");
	}

	return newfd;
}


void ejecutarSelect(fd_set* fdAuxClientes, int fdMax ){
	select(fdMax+1, fdAuxClientes, NULL, NULL, NULL);
}

void agregarNuevoCliente(fd_set* fdClientes, int nuevoCliente){
	FD_SET(nuevoCliente, fdClientes);
}

int actualizarMaximo(int fdMax, int nuevoCliente){
	if(nuevoCliente > fdMax)
		fdMax = nuevoCliente;

	printf("El nuevo maximo es: %d \n" , fdMax);

	return fdMax;
}

void desconectarCliente(int fdCliente, fd_set* fdClientes){
	close(fdCliente);
	FD_CLR(fdCliente, fdClientes);
}

int main(){
	int puerto = 2550;
	int cantClientesEscucha = 20;
	int fdListener = startListener(puerto, cantClientesEscucha);

	fd_set fdClientes;
	fd_set fdAuxClientes;

	FD_ZERO(&fdClientes);
	FD_ZERO(&fdAuxClientes);

	int fdMax = fdListener;
	int i=0;
	int nuevoCliente;
	FD_SET(fdListener, &fdClientes);
	char* msg;

	int cantCaracteres = 256;
	int tamMensaje = sizeof(char)*cantCaracteres;


	while(1){
		fdAuxClientes = fdClientes;

		ejecutarSelect(&fdAuxClientes, fdMax);

		/*
		 * A partir de acá se atienden clientes;
		 * Se recorre el for de 0 a fdMax, barriendo a todos los clientes. Podriamos obviar los fd 0,1,2 porque son reservados por el sistema (stdin, stdout y stderror me parece)
		 *
		 */
		for(i=0; i<= fdMax;i++){

			if(FD_ISSET(i, &fdAuxClientes)){ //Si el fd esta dentro del fdAuxClientes, es porque envió un mensaje

				if(i == fdListener){ //Si es el listener, tenemos una nueva conexión.
					nuevoCliente = aceptarNuevaConexion(fdListener);
					agregarNuevoCliente(&fdClientes, nuevoCliente);
					fdMax = actualizarMaximo(fdMax, nuevoCliente);
					printf("Se conecto el cliente %d \n", nuevoCliente);
				}

				else{ //Si no es el listener, es un cliente, tenemos que ver qué mensaje mandó

					msg = malloc(tamMensaje); //Por default reservamos 256 bytes

					if(recv(i,msg,tamMensaje,0) <= 0){ //Si la cantidad recibida son 0 bytes o menos, el cliente se desconectó :(
						desconectarCliente(i, &fdClientes);
						printf("Se desconecto el cliente %d \n", i);
					}
					else{
						//Caso contrario, llegó el mensaje
						printf("El mensaje recibido es %s -- Enviado por Cliente %d \n", msg,i);
					}

					free(msg);


				}
			}
		}
		fflush(stdout);
	}

	return 0;
}

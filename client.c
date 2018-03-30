// section Include
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <pthread.h>
#include <pwd.h>

// les Defines
#define BUFFER 4096 

typedef struct {
	int* socket;
	struct sockaddr* addr;
	char usr_name[50];
} info_socket;

void * ecouter (void * args);
void * ecrire (void * args);

// Début du programme
int main (){

  // crée le socket client
  int socket_fd;
  if(( socket_fd = socket(AF_INET,SOCK_DGRAM,0)) == -1 ){
	perror("socket");
	return EXIT_FAILURE;
  }
  
  info_socket* info = malloc(sizeof(*info));
  
  //création de l'adresse serv
  struct addrinfo serv;
  memset(&serv, 0, sizeof(struct addrinfo));
  serv.ai_family = AF_INET;    /* Allow IPv4 not IPv6 */
  serv.ai_socktype = SOCK_DGRAM; /* Datagram socket (UDP) */

  struct addrinfo *res;
  int r;

  if (( r = getaddrinfo(NULL, "50000",&serv, &res)) != 0) {
	fprintf(stderr,"getaddrinfo():  %s \n", gai_strerror(socket_fd));
	return EXIT_FAILURE;
  }
  if ( res == NULL) {
	fprintf(stderr,"Erreur lors de la constructiond de l'adresse \n");
	return EXIT_FAILURE;
  }
	
	//récupère le nom de l'utilisateur
	struct passwd *pw_user;
	uid_t uid_user;
	char *user_name;
	uid_user = geteuid();
	pw_user = getpwuid(uid_user);
	user_name = pw_user->pw_name;
	
	strcpy(info->usr_name,user_name);
	info->addr = (struct sockaddr*) res->ai_addr;
	info->socket = &socket_fd;
	
	pthread_t ecoute, ecriture;
	if(pthread_create(&ecoute,NULL,ecouter,info) == -1){
			perror("pthread_create");
			return EXIT_FAILURE;
	}
	
	if(pthread_create(&ecriture,NULL,ecrire,info) == -1){
			perror("pthread_create");
			return EXIT_FAILURE;
	}
	while(1);

freeaddrinfo(res);

if(close(socket_fd) == -1 ) {
  perror("close");
  return EXIT_FAILURE;
}

return EXIT_SUCCESS; 
}

//thread d'écoute
void * ecouter (void * args){
	info_socket* info = (info_socket*)args;
	socklen_t res_len = sizeof(info->addr);
	char msg[BUFFER] ;
	while(1){
		if (recvfrom(*info->socket, msg, sizeof(msg) - 1, 0, info->addr, &res_len) == -1) {
				perror("recvfrom\n");
		}
		fprintf(stdout,"recu --:> %s\n",msg);
	}
}
//thread d'écriture
void * ecrire (void * args){
	
	info_socket* info = (info_socket*)args;
	char *user_name = malloc(sizeof(*user_name));
	strcpy(user_name,info->usr_name);
	char msg[BUFFER] ;
	char cont[BUFFER] ; 
	while(1){
		fgets(cont,BUFFER,stdin);
		snprintf(msg, sizeof(msg),"%s: %s \n", user_name,cont);
		fprintf(stdout,"envoie --:> %s\n",msg);
		// si la connection est établie on send le message !
		if (sendto(*info->socket,&msg, strlen(msg)+1,0,info->addr, sizeof(struct sockaddr)) == -1) {
			perror("sendto");
		}
	}
		
}

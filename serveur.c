// section Include
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include "header.h"

// les Defines
#define BUFFER 150 
#define HISTO "historique.log"

// la struct du thread qui lui permet de gerer toute la durée de vie d'un message
typedef struct {
	int *s;
	client* sender;
	pool_client* listclient;
	char msg[256];
	
}handler_client;

//action des threads
void * run (void * args);

/**
*	Le Serveur:
*	1-lorsque il recoit un client crée un thread qui prend en charge le client
*	2-Charge l'historique de la conversation  (max 30 ligne)
	3-ajoute et modifie l'historique de conversation  en fonction des clients et met a jours les autres.
*
*/

int main() {
	

  // construit l'adresse serveur
  int s;
  if ((s = socket(AF_INET,SOCK_DGRAM, 0 )) == -1) {
	perror("socket");
	return EXIT_FAILURE;
  }
  
  struct addrinfo serv;
  memset(&serv, 0, sizeof(serv));
  serv.ai_family = AF_INET;
  serv.ai_socktype = SOCK_DGRAM;
  serv.ai_flags = AI_PASSIVE;

  struct addrinfo *res;
  int r;

  if (( r = getaddrinfo(NULL, "50000",&serv, &res)) != 0) {
        fprintf(stderr,"getaddrinfo():  %s \n", gai_strerror(s));
        return EXIT_FAILURE;
  }
  if ( res == NULL) {
        fprintf(stderr,"Erreur lors de la constructiond de l'adresse \n");
        return EXIT_FAILURE;
  }
  // affecte l'adresse a notre serveur
  if(bind(s, (struct sockaddr *) res->ai_addr, sizeof(struct sockaddr)) == -1) {
	perror("bind");
	return EXIT_FAILURE;
  }

 
  pool_client *listclient = malloc(sizeof(pool_client));
  listclient->nb_client = 0;
  
  char msg[BUFFER];
  msg[BUFFER-1] = 0;
  struct sockaddr_in* addr_from = malloc(sizeof(struct sockaddr_in));
  socklen_t addr_len = sizeof(addr_from);
  while(1){
	  // le serveur attend des client et écoute en boucle
		if (recvfrom(s, msg, sizeof(msg) - 1, 0, (struct sockaddr*) addr_from, &addr_len) == -1) {
			perror("recvfrom\n");
			return EXIT_FAILURE;
		}
		//récupere le contenu du message avant le strtok
		handler_client* action = malloc(sizeof(handler_client));
		strcpy(action->msg, msg);
		
		//gestion des infos reçu
		char* substrings = strtok(msg,":");
		char client_name[50];
		strcpy(client_name,substrings);
		fprintf(stdout,"%s",client_name);
		
		client *sender = malloc(sizeof *sender);
		sender->name = client_name;
		sender->addr = (struct sockaddr*)addr_from;
		
		action->s = &s;
		action->sender = sender;
		action->listclient = listclient;
		
		
		//verifie si le client est déjà connecter si non tente de l'ajouter
		if(verif_client(client_name, listclient) == 0){
			send_brief(&s,sender);
			addclient(client_name,(struct sockaddr*)&addr_from,listclient);
		}
		
		for(int i = 0; i<listclient->nb_client;++i){
			fprintf(stderr,"client: %s",listclient->clients[i]->name);
		}
		
		
		pthread_t thread1;
		/*on recoit un message d'un client on crée un thread pour gérer
		les actions demandé par le client*/
		if(pthread_create(&thread1,NULL,run,(void *)action) == -1){
				perror("pthread_create");
				return EXIT_FAILURE;
		}
			
	}

freeaddrinfo(res);

if (close(s) == -1 ) {
  perror("close");
  return EXIT_FAILURE;
}
}


/**
 * GESTION  THREAD
 * */


void * run (void * args) {
	//gerer des sémaphores pour empecher plusieur thread d'écrire en même temps

  handler_client* action = (handler_client*)args;
  
  int histo = open(HISTO,O_WRONLY | O_APPEND);
  if(histo == -1){
	  perror("open");
	  return 0;
  }
	
	//fprintf(stdout,"Dans le thread \n");	
	char msg[BUFFER];
	msg[BUFFER -1] = 0;
	strcpy(msg,action->msg);
	
	client* sender = action->sender;
	pool_client* listclient = action->listclient;
	int *s = action->s;
	//ecriture dans l'historique
	write(histo,msg,strlen(msg)-1);
	//envoie aux autres clients
	actualise_clients(s,msg,listclient,sender);
	
	return 0;
}


/*
 *Gestion de message client 
 * */
 
 /*
 * actualises les clients leur envoie le dernier message recu par le serveur
 * */
 
void actualise_clients(int* s,char* msg,pool_client* listclient,client* sender){
	
	for(int i=0; i<listclient->nb_client; i++){
		if(strcmp(listclient->clients[i]->name,sender->name) != 0){
			// envoie du message aux autres clients
			fprintf(stderr,"debug");
			//pour le test envoie a l'émetteur également
			if (sendto(*s,msg, strlen(msg)+1,0,listclient->clients[i]->addr, sizeof(struct sockaddr)) == -1) {
				perror("sendto");
			}
		}
		fprintf(stderr,"debug 2");
	}
	
}


#ifndef _HEADER_H
#define _HEADER_H

// section Include
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <pwd.h>

// les Defines
#define BUFFER 150 
#define HISTO "historique.log"
#define MSG_LEN 8192


//struct d'info client
typedef struct {
	struct sockaddr* addr;
	char* name;
} client;

//tableau de client, symbolise le nombre de client possible à un même instant
typedef struct {
	client* clients[10];
	int nb_client;
} pool_client;




//les outils
void send_brief(int*,client*);
void actualise_clients(int *s,char*,pool_client*,client*);
int verif_client(char*,pool_client*);
void addclient(char*,struct sockaddr*,pool_client*);

//verif client
int verif_client(char* name, pool_client* listclient){
	
	if( listclient->nb_client != 0){
		for(int i=0; i<listclient->nb_client;i++){
			if (strcmp(listclient->clients[i]->name,name)==0){
				return 1;
			}
		}
	}
	return 0;
}

//ajout de client
void addclient(char* name,struct sockaddr* info_client,pool_client* listclient){
	
	if(listclient->nb_client < 10){
		//ajout en tant que client connecté
		client* new_client = malloc(sizeof(*new_client));
		
		listclient->nb_client += 1;
		new_client->name = name;
		new_client->addr = info_client;
		listclient->clients[listclient->nb_client-1] = new_client;
	} else {
		fprintf(stderr,"nombre de client maximun atteint!");
	}
	
}

void send_brief(int *socket, client *user) {
    int t[2];
    pipe(t);
    
    switch (fork()) {
    case 0:
        close(t[0]);
        dup2(t[1], STDOUT_FILENO);

        execlp("tail","tail", "-n", "30", HISTO, NULL);
        
    default:
    {
        char log[MSG_LEN];

        close(t[1]);

        read(t[0], log, MSG_LEN);
        log[MSG_LEN - 1] = '\0';

        if (sendto(*socket, log, MSG_LEN, 0, user->addr, sizeof(struct sockaddr)) == -1) {
            perror("sendto");
        }
        break;
    }        
    }    
}

#endif

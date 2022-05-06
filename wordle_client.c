#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <ctype.h> 

#define PORT_INCP 4242
ssize_t exact_read(int fd, void * buf,size_t nbytes);
ssize_t exact_write(int fd, void * buf,size_t nbytes);
void usage(char *nom_prog)
{
	fprintf(stderr, "Usage: %s addr_ipv4\n"
			"client pour INCP (Incrementation Protocol)\n"
			"Exemple: %s 208.97.177.124\n", nom_prog, nom_prog);
}
int main(int argc, char *argv[])
{
	if (argc != 2) {
		usage(argv[0]);
		return 1;
	}
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(2);
	}
	struct sockaddr_in sa = { .sin_family = AF_INET, .sin_port = htons(PORT_INCP) };
	if (inet_pton(AF_INET, argv[1], &sa.sin_addr) != 1) {
		fprintf(stderr, "adresse ipv4 non valable\n");
		exit(1);
	}
	
	if (connect(sock, (struct sockaddr *) &sa, sizeof(struct sockaddr_in)) < 0) {
		perror("connect");
		exit(3);
	}
	uint32_t cont=1;
	while(cont){
		int nbtentatives=0;
		while(1){
			nbtentatives++;
			char propositions[6];
			char response[256];
			printf("Votre proposition: ");
			scanf("%s", propositions);
			propositions[strlen(propositions)]='\0';
			exact_write(sock,&propositions, sizeof(char)*6);
			/* 4.2 Réponse du serveur */
			read(sock, &response, sizeof(char)*256);
			printf("%s\n",response);
			int nbBonMot = 0;
			for(int i=0;i<5;i++){
				if(response[i]==toupper(propositions[i])){
					nbBonMot++;
				}
			}
			if(nbBonMot == 5){
				printf("BRAVO !!! mot trouvé en %d tentatives :) \n",nbtentatives);
				break;
			}
		
		}
		printf("une nouvelle partie ? 1:OUI, 0:NON \n");
		scanf("%u",&cont);
		cont=htonl(cont);
		write(sock, &cont, sizeof(uint32_t));
		cont = ntohl(cont);
	}

	close(sock);
	return 0;
}
ssize_t exact_read(int fd, void * buf,size_t nbytes){
	ssize_t total,n;
	total=0;
	while(total<nbytes){
		n=read(fd,buf+total,nbytes-total);
		if(n==-1){
			return n;
		}
		if(n==0){
			break;
		}
		total+=n;
	}
	return total;
}
ssize_t exact_write(int fd, void * buf,size_t nbytes){
	ssize_t total,n;
	total=0;
	while(total<nbytes){
		n=write(fd,buf+total,nbytes-total);
		if(n==-1){
			return n;
		}
		if(n==0){
			break;
		}
		total+=n;
	}
	return total;
}

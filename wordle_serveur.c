#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <sys/stat.h> 
#include <time.h> 
#define PORT_WRDLP 4242
#include "mots_5_lettres.h"

typedef struct{
	char * prop; // pour y stocker la proposition du joueur
	int sock; // pour la sockette d'échange
	int logfile; // pour le journal de bord
	pthread_mutex_t * mutlog; //pour verrouiller l'écriture sur le journal de bord
	char a_deviner[6]; //pour y stocker le mot à deviner
}wrdlpClient;

/* Met toutes les lettres minuscules présentes dans ch en majuscule (les autres
 * caractères sont inchangés).
 * Précondition : ch contient un caractère nul qui sert de délimiteur de fin */
void chaine_toupper(char *ch);

/* Met dans la chaine prop (tableau d'au moins 6 char) un mot de 5 lettres saisi
 * par le client, si besoin mis en majuscule, et terminé par un '\0'.
 * Redemande la saisie tant que
 * - le mot de l'utilisateur a moins de 5 lettres ou
 * - n'est pas dans la liste de mots.
 * Si le client saisit un mot de plus de 5 lettres, seules les 5 premières
 * sont prises en compte.
 */
void * saisir_prop(void *prop);

/*essaie de lire exactement nbytes octects du descripteur de fichier fd vers le buffer buf.
cette fonction renvoie le nombre d'octects lus */
ssize_t exact_read(int fd, void * buf,size_t nbytes);

/*essaie d'écrire exactement nbytes octects sur le descripteur de fichier fd à partir du buffer buf.
cette fonction renvoie le nombre d'octects écrits */
ssize_t exact_write(int fd, void * buf,size_t nbytes);

/* Pour chaque lettre de prop_joueur[i], stocke dans un tableau wordToSend[i] :
 * * cette lettre en majuscule si elle figure à la même position dans a_deviner
 * * cette lettre en minuscule si elle fait partie de a_deviner mais à une autre
 *   position
 * * le caractère _ sinon
 * Retourne le tableau wordToSend
 * Préconditions : prop_joueur et a_deviner contiennent au moins 5 caractères
 *                 qui sont tous des lettres majuscules */
char * traiter_prop(const char *prop_joueur, const char *a_deviner,int * nb_lettres_trouvees);

int main()
{
	pthread_mutex_t mutlog;
	pthread_mutex_init(&mutlog, NULL);
	int logfile=open("wordle.log",O_CREAT |O_TRUNC |O_WRONLY, 0644);
	if(logfile<0){
		perror("open : ");
		return 1;
	}
	
	srand(time(NULL));
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(2);
	}
	struct sockaddr_in sa = { .sin_family = AF_INET,
				  .sin_port = htons(PORT_WRDLP),
				  .sin_addr.s_addr = htonl(INADDR_ANY) };
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	if (bind(sock, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("bind");
		exit(3);
	}
	if (listen(sock, 10) < 0) {
		perror("listen");
		exit(2);
	}
	pthread_t t;
	while(1){
		struct sockaddr_in s;
		socklen_t  ss = sizeof(s);
		/* 5. Attente passive d'une connection. */
		int sock_echange=accept(sock,(struct sockaddr *)&s, &ss);
		if(sock_echange<0){
			perror("accept : ");
			return 1;
		}
		char addrClient[INET_ADDRSTRLEN];
		if(inet_ntop(AF_INET,&(s.sin_addr),addrClient,INET_ADDRSTRLEN)==NULL){
			perror("inet_ntop");
		}
		else{
			time_t now=time(NULL);
			char dateEtHeure[32];
			char lgmessage[256];
			strftime(dateEtHeure,32,"%F:%T",localtime(&now));
			sprintf(lgmessage, "%s : connecté avec %s\n",dateEtHeure, addrClient);
			pthread_mutex_lock(&mutlog);
			write(logfile,lgmessage,strlen(lgmessage));
			pthread_mutex_unlock(&mutlog);
		}
		wrdlpClient * propClient = malloc(sizeof(wrdlpClient));
		propClient->sock=sock_echange;
		propClient->logfile=logfile;
		propClient->mutlog=&mutlog;
		if((pthread_create(&t,NULL,saisir_prop,propClient))<0){
			perror("pthread_create ");
			return 1;
		}
		pthread_detach(t);
	}
	pthread_mutex_destroy(&mutlog);
	close(sock);
	
	return 0;
}

void chaine_toupper(char *ch)
{
	int i;
	for (i = 0; ch[i] != '\0'; i = i + 1)
		ch[i] = toupper(ch[i]);
}

char * traiter_prop(const char *prop_joueur, const char *a_deviner, int * nb_lettres_trouvees)
{
	char * wordToSend= malloc(sizeof(char)*6);
	int i;
	*nb_lettres_trouvees = 0;
	for (i = 0; i < 5; i = i + 1) {
		if (prop_joueur[i] == a_deviner[i]) {
			wordToSend[i]= prop_joueur[i];
			*nb_lettres_trouvees = *nb_lettres_trouvees + 1;
		} else if (strchr(a_deviner, prop_joueur[i])) {
			wordToSend[i]=tolower(prop_joueur[i]);
		} else {
			wordToSend[i]='_';
		}
	}
	wordToSend[i]='\0';
	return wordToSend;
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
void * saisir_prop(void *arg)
{	
	pthread_t t;
	wrdlpClient * propEtSock = (wrdlpClient *) arg;
	propEtSock->prop=malloc(sizeof(char)*6);
	char * message;
	int correct = 0;
	uint32_t newGame = 0;
	int nbLettres;
	mot_alea5(propEtSock->a_deviner);
	printf("%s \n" , propEtSock->a_deviner);
	while (!correct) {
		if(exact_read(propEtSock->sock,propEtSock->prop,sizeof(char)*6)<0){
			close(propEtSock->sock);
			return NULL;
		}
		//Ecriture de la requête 
		
		time_t now=time(NULL);
		char dateEtHeure[32];
		char lgmessage[256];
		strftime(dateEtHeure,32,"%F:%T",localtime(&now));
		sprintf(lgmessage, "%s : mot reçu: %s\n mot à deviner: %s \n",dateEtHeure, propEtSock->prop, propEtSock->a_deviner);
		pthread_mutex_lock(propEtSock->mutlog);
		write(propEtSock->logfile,lgmessage,strlen(lgmessage));
		pthread_mutex_unlock(propEtSock->mutlog);
		
		//Réponse à la requête 
		if((strlen(propEtSock->prop))<5){
			message = "Mot trop court, entrer un mot de 5 lettres.\n\0";
		}else {
			chaine_toupper(propEtSock->prop);
			if (!est_dans_liste_mots(propEtSock->prop)){
				message = "Ce mot n'est pas dans la liste de mots.\n\0";
			}
			else{
				message=traiter_prop(propEtSock->prop,propEtSock->a_deviner,&nbLettres);
				if(nbLettres>=5){
					correct = 1;
				}
			}
		}
		if(write(propEtSock->sock,message,sizeof(char)*256)<0){
			close(propEtSock->sock);
			return NULL;
		}
	}
	newGame=htonl(newGame);
	read(propEtSock->sock, &newGame, sizeof(uint32_t));
	newGame=ntohl(newGame);
	if(newGame != 0){
		pthread_create(&t, NULL, saisir_prop, propEtSock);
		pthread_join(t,NULL);
	}
	close(propEtSock->sock);
	return NULL;
}

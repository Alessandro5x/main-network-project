#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h> 
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define palavraChave "ornitorrinco"

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

//Função que checa se o palpite já foi digitado antes
bool checaPalpite(char palpite, char *palpitesFeitos){

	for(int i = 0; i<sizeof(palpitesFeitos);i++){
		if (palpite == palpitesFeitos[i]){
			return true;
		}
	}
	return false;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("Conexão realizada em %s\n", addrstr);


//Vetor de palpites e contador dos mesmos
	int countPalpite = 0;
	char palpitesFeitos[26];
	memset(palpitesFeitos, 0, sizeof(palpitesFeitos));

//Função que preenche o vetor de palpites já realizados
	void preenchePalpites(char newpalpite, int cnt){
		palpitesFeitos[cnt]= newpalpite;
	}

//Loop para receber e enviar as mensagens até o Fim do jogo	
	while(1){
		char buf[8];
		memset(buf, 0, BUFSZ);

// Recebe a mensagem de inicio após a conexão e esvazia o palpite
		size_t cnt1 = recv(s, buf, BUFSZ, 0);
		char palpite[2];
		palpite[1] = ' ';


// Opção para inicio do jogo
	if (buf[0]==1){
		printf("Inicio do jogo\n\n");
		printf("A palavra a ser respondida tem %d letras\n\n", buf[1]);

		memset(palpite, 0, sizeof(palpite));
		printf("Atencão!!\n\nDigite apenas uma letra\n\nSe mais de uma letra for digitada será considerada apenas a primeira\n\n");
		printf("Digite seu palpite e pressione enter> ");
		scanf(" %c", &palpite[1]);

		preenchePalpites(palpite[1],countPalpite);
		countPalpite++;
		size_t count = send(s, &palpite[1], sizeof(palpite[1]), 0);
		if (count != sizeof(palpite[1])) {
			logexit("send");
		}
	}

// Opção após realização do primeiro palpite e realização dos próximos
	if (buf[0]==3){
		printf("\n*********Resposta do seu palpite********\n");
		printf("Quantas vezes seu palpite aparece na palavra> %d\n", buf[1]);
		if(buf[1]>0){
			printf("\nParabéns!!!\n\nSeu palpite está presente na palavra\n\nNas posicões\n\n");
		}
		if(buf[1]==0){
			printf("\nAhh, que pena!\n\nSeu palpite não está presente na palavra, mas tente novamente!\n\n");
		}
		for( int i = 0; i<(int)cnt1-4;i++){
			printf("Posição %d \n",buf[i+2]+1);
		}

		printf("Digite seu novo palpite> ");
		scanf(" %c", &palpite[1]);

		while(checaPalpite(palpite[1],palpitesFeitos)){
			printf("\n----------Voce ja enviou essa letra----------\n");
			printf("Digite outro palpite> ");
			scanf(" %c", &palpite[1]);
		}
		preenchePalpites(palpite[1],countPalpite);
		countPalpite++;
		size_t count = send(s, &palpite[1], sizeof(palpite[1]), 0);
		if (count != sizeof(palpite[1])) {
			logexit("send");
		}
	}

//Opção após acertar todas as letras da palavra
	if (buf[0]==4){
		printf("\nParabéns campeão\n\n");
		printf("\nYou Win!\n\n");
		printf("A palavra era %s\n\n",palavraChave);
		printf("Fim de jogo\n");
		
		break;
		close(s);
	}
	}

	exit(EXIT_SUCCESS);
}
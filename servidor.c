#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define palavraChave "ornitorrinco"

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("Escutando %s, esperando conexões\n", addrstr);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("Conexão feita por %s\n", caddrstr);

// Envia a mensagem de inicio e o tamanho da palavra após a conexão       
        char msg[2];
        unsigned int lenKey = strlen(palavraChave);
        memset(msg, 0, BUFSZ);
        msg[0] = 1;
        msg[1] = lenKey;
        size_t cnt = send(csock, msg, strlen(msg) + 1, 0);
        if (cnt != strlen(msg) + 1) {
            logexit("send");
        }
        
        int i;
        int soma;    
// Loop para o jogo
    while(1){
        char palpite[2];
        memset(palpite, 0, sizeof(palpite));
        size_t count = recv(csock, palpite, BUFSZ - 1, 0);

        if((int)count ==0){
        printf("Palpite vazio e o Jogador desistiu\n");
            break;
        }
        printf("Dados do jogador: %s\nTamanho da mensagem %d bytes\n O palpite foi> %s\n", caddrstr, (int)count, palpite);

// Variaveis e função para verificar se o palpite do jogador esta na palavra
        int aux[lenKey];
        int j = 1;
        aux[0] = 0;
        int *verificapalpite(char palp , char word[26]){
            for(int i = 0; i <strlen(word);i++){
                if (palp == word[i]){
                    aux[0] = aux[0]+1;
                    aux[j] = i;
                    j = j+1;
                }
            }
            return aux;
        }
      
        int *result = verificapalpite(palpite[0],palavraChave);
        char resultado[j+2];
        resultado[0] = 3;
        resultado[1] = result[0];

//Loop que colocada os valores das posições de forma alinhada no vetor resultado e incrementa a variável soma que será usada para verificar o fim do jogo
        for( i=1;i <sizeof(resultado)-1;i++){
            resultado[i+1] = result[i];
            soma = soma + (int)result[i-1];
        }

// Função complementar para somar a posição de cada letra e determinar o fim do jogo
        int somaRecursiva(int n){
            if (n ==1){
                return n;
            }else
                return n + somaRecursiva(n-1);
        }

//Verifica se todas as palavras já foram acertadas e envia o resultado         
        if (soma == somaRecursiva(lenKey)){
           resultado[0] = 4;
            send(csock, resultado, sizeof(resultado) + 1, 0);            
            printf("\nFIM DE JOGO\n\n");
            printf("Aguardando o proximo jogador...\n");
            break;
        }

//Envia a resposta de quantas vezes o palpite aparece na palavra 
        memset(palpite, 0, sizeof(palpite));
        size_t cntresult = send(csock, resultado, sizeof(resultado) + 1, 0);
        if (cntresult != sizeof(resultado) + 1) {
            logexit("send");
        }
}

//Finaliza a conexão
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        size_t cntf = send(csock, buf, strlen(buf) + 1, 0);
        if (cntf != strlen(buf) + 1) {
            logexit("send");
        }
        close(csock);
    }
    exit(EXIT_SUCCESS);
}
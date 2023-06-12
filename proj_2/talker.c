#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <poll.h>

#define SERVERPORT "8000"   //Número da porta
#define MAX_CHUNK_SIZE 1024 //Tamanho máximo de cada chunk de mensagem
#define TIMEOUT_MS 2500     //Valor de timeout em ms
#define MAX_RETRY_COUNT 10  //Máximo de tentativas

//Recebe os chunks da imagem
bool receive_chunks(int sockfd, struct sockaddr* addr, socklen_t addr_len, FILE* file) {
    char buf[MAX_CHUNK_SIZE];
    socklen_t sender_addr_len;
    int numbytes;

    while (1) {
        sender_addr_len = addr_len;
        numbytes = recvfrom(sockfd, buf, MAX_CHUNK_SIZE, 0, addr, &sender_addr_len);
        if (numbytes == -1) {
            perror("talker: recvfrom");
            exit(1);
        }
        if (strncmp(buf, "Imagem nao existe", strlen("Imagem nao existe")) == 0){
            printf("server: Imagem não existe\n");
            exit(1);
        }
        fwrite(buf, 1, numbytes, file);

        //Assumindo que o último chunk recebido tem um tamanho menor que MAX_CHUNK_SIZE, indica o final do arquivo
        if (numbytes < MAX_CHUNK_SIZE) {
            printf("talker: arquivo recebido salvo como 'received.png'\n");
            break;
        }
    }
    return false;
}

int main(int argc, char* argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv, op, numbytes;

    if (argc != 3) {
        fprintf(stderr,"usage: talker hostname operation\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //Itera por todos os resultados e faz um socket
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    char request[MAX_CHUNK_SIZE];
    strcpy(request, argv[2]);
    op = -1;
    if (!(strncmp(request, "download", strlen("download")))){
        printf("talker: qual email do usuário para baixar a imagem?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 0;
    }else if(!(strncmp(request, "help", strlen("help")))){
        op = 9;              
    }else if(!(strncmp(request, "register", strlen("register")))){
        printf("talker: qual é o email do usuário?\n");
        int n = strlen(request);
        request[n] = '\n';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\n';
        printf("talker: qual é o nome do usuário?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\n';
        printf("talker: qual é o sobrenome do usuário?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\n';
        printf("talker: qual é a residência do usuário?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\n';
        printf("talker: qual é a formação acadêmica do usuário?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\n';
        printf("talker: qual é o ano de formatura do usuário?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\n';
        printf("talker: quais são as habilidades do usuário?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op=1;
    }else if(!(strncmp(request, "srch_frmt", strlen("srch_frmt")))){
        printf("talker: qual formação acadêmica pesquisar?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 2;
    }else if(!(strncmp(request, "srch_skill", strlen("srch_skill")))){
        printf("talker: qual habilidade pesquisar?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 3;
    }else if(!(strncmp(request, "srch_year", strlen("srch_year")))){
        printf("talker: qual ano pesquisar?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 4;
    }else if(!(strncmp(request, "list", strlen("list")))){
        op = 5;
    }else if(!(strncmp(request, "srch_usr", strlen("srch_usr")))){
        printf("talker: qual email pesquisar?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 7;
    }else if(!(strncmp(request, "remove", strlen("remove")))){
        printf("talker: qual email remover?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 8;
    }
    bool rcv = true;
    freeaddrinfo(servinfo);
    //Configura a struct pollfd
    struct pollfd fds[1];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    
    if(op == 0){
        FILE* file = fopen("received.png", "wb");
        if (file == NULL) {
            perror("talker: fopen");
            exit(1);
        }
        while(rcv){
            if (sendto(sockfd, request, strlen(request), 0, p->ai_addr, p->ai_addrlen) == -1) {
                perror("talker: sendto");
                exit(1);
            }

            //Aguarda a resposta com um Timeout
            int poll_result = poll(fds, 1, TIMEOUT_MS);
            if (poll_result == -1) {
                perror("talker: poll");
                exit(1);
            } else if (poll_result == 0) {
                printf("talker: Tempo limite expirado. Tentando novamente...\n");
                continue;
            }

            //Indica que a resposta está disponível
            rcv = receive_chunks(sockfd, p->ai_addr, p->ai_addrlen, file);
        }

        fclose(file);
    } else {
        bool receivedResponse = false;
        int retryCount = 0;

        while (!receivedResponse && retryCount < MAX_RETRY_COUNT) {
            //Envia a requisição
            if (sendto(sockfd, request, strlen(request), 0, p->ai_addr, p->ai_addrlen) == -1) {
                perror("talker: sendto");
                exit(1);
            }

            int poll_result = poll(fds, 1, TIMEOUT_MS);
            if (poll_result == -1) {
                perror("talker: poll");
                exit(1);
            } else if (poll_result == 0) {
                printf("talker: Tempo limite expirado. Tentando novamente...\n");
                retryCount++;
            } else {
                //Recebe a mensagem
                char buf[MAX_CHUNK_SIZE];
                numbytes = recvfrom(sockfd, buf, MAX_CHUNK_SIZE - 1, 0, p->ai_addr, &p->ai_addrlen);
                if (numbytes == -1) {
                    perror("talker: recvfrom");
                    exit(1);
                }
                buf[numbytes] = '\0';
                printf("server: %s", buf);
                receivedResponse = true;
            }
        }

        if (!receivedResponse) {
            printf("talker: Contagem máxima de tentativas alcançada. Nenhuma resposta recebida.\n");
        }
    }
    close(sockfd);
    return 0;
}

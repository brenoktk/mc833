#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 8081
#define SA struct sockaddr



typedef struct Pessoa{
    char email[MAX];
    char name[MAX];
    char surname[MAX];
	struct Pessoa* next;
    /*char city[MAX];
    char formation[MAX];
    int formation_year;
    char abilities[MAX];*/
}Pessoa;

// Function designed for chat between client and server.
void func(int connfd)
{
	char buff[MAX], email[MAX], name[MAX], surname[MAX];
	int n;
	FILE* user;
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);

		// read the message from client and copy it in buffer
		read(connfd, buff, sizeof(buff));
		// print buffer which contains the client contents
		printf("From client: %s\t", buff);
        if (strncmp("1", buff, 1) == 0) {
            bzero(buff, MAX);
            char msg[] = "Cadastro iniciado. Insira um e-mail\n";
            write(connfd, msg, sizeof(msg));
            read(connfd, buff, sizeof(buff));
            strcpy(email, buff);
            
            char msg1[] = "Insira seu nome\n";
            write(connfd, msg1, sizeof(msg1));
            read(connfd, buff, sizeof(buff));
            strcpy(name, buff);

            char msg2[] = "Insira seu sobrenome\n";
            write(connfd, msg2, sizeof(msg2));
            read(connfd, buff, sizeof(buff));
            strcpy(surname, buff);

            /*Pessoa p1;
			memset(&p1, 0, sizeof(Pessoa));
			strcpy(p1.email, email);
			strcpy(p1.name, name);
			strcpy(p1.surname, surname);*/
			user = fopen("teste.txt", "w+");
			fprintf(user, "%s%s%s", email, name, surname);
			fclose(user);
            //write(connfd, p1.name, sizeof(p1.name));
            //printf("%s %s %s", p1.email, p1.name, p1.surname);
		}
		bzero(buff, MAX);
		n = 0;
		// copy server message in the buffer
		//while ((buff[n++] = getchar()) != '\n')
		//	;

		// and send that buffer to client
		write(connfd, buff, sizeof(buff));

		// if msg contains "Exit" then server exit and chat ended.
		//if (strncmp("exit", buff, 4) == 0) {
		//	printf("Server Exit...\n");
		//	break;
		//}
	}
}

// Driver function
int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");

	// Function for chatting between client and server
	func(connfd);

	// After chatting close the socket
	close(sockfd);
}

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> // read(), write(), close()
#include <regex.h>
#include <dirent.h>
#define MAX 80
#define PORT 8080
#define SA struct sockaddr


// Function designed for chat between client and server.
void func(int connfd)
{
	char buff[MAX], email[MAX], name[MAX], surname[MAX], residence[MAX], formation[MAX], year[MAX], skills[MAX];
	regex_t regex;
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

			// Compile the regex
			if (regcomp(&regex, "^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.com\n$", REG_EXTENDED) != 0) {
				printf("Could not compile regex\n");
				break;
			}

			// Execute the regex
			if (regexec(&regex, email, 0, NULL, 0) != 0) {
				printf("Invalid email\n");
				char msg1[] = "Email invalido. Tente novamente.\n";
				write(connfd, msg1, sizeof(msg1));
				continue;
			}
            
            char msg1[] = "Insira seu nome\n";
            write(connfd, msg1, sizeof(msg1));
            read(connfd, buff, sizeof(buff));
            strcpy(name, buff);

            char msg2[] = "Insira seu sobrenome\n";
            write(connfd, msg2, sizeof(msg2));
            read(connfd, buff, sizeof(buff));
            strcpy(surname, buff);

			char msg3[] = "Insira sua Residência\n";
            write(connfd, msg3, sizeof(msg3));
            read(connfd, buff, sizeof(buff));
            strcpy(residence, buff);

			char msg4[] = "Insira sua Formação Acadêmica\n";
            write(connfd, msg4, sizeof(msg4));
            read(connfd, buff, sizeof(buff));
            strcpy(formation, buff);

			char msg5[] = "Insira seu Ano de Formatura\n";
            write(connfd, msg5, sizeof(msg5));
            read(connfd, buff, sizeof(buff));
            strcpy(year, buff);

			char msg6[] = "Insira suas Habilidades\n";
            write(connfd, msg6, sizeof(msg6));
            read(connfd, buff, sizeof(buff));
            strcpy(skills, buff);

			size_t len = strlen(email);
			if (email[len-1] == '\n') {
				email[len-1] = '\0';
			}
			char filename[MAX];
			sprintf(filename, "users/%s.txt", email);
			user = fopen(filename, "w+");
			fprintf(user, "%s\n%s%s%s%s%s%s", email, name, surname, residence, formation, year, skills);
			fclose(user);
			char msg7[] = "Cadastro realizado!\n";
			write(connfd, msg7, sizeof(msg7));
            //write(connfd, p1.name, sizeof(p1.name));
            //printf("%s %s %s", p1.email, p1.name, p1.surname);
		}
		else if (strncmp("2", buff, 1) == 0) {
			bzero(buff, MAX);
            char msg[] = "Descadastro iniciado. Insira um e-mail\n";
			write(connfd, msg, sizeof(msg));
            read(connfd, buff, sizeof(buff));
            strcpy(email, buff);

			size_t len = strlen(email);
			if (email[len-1] == '\n') {
				email[len-1] = '\0';
			}
			char filename[MAX];
			sprintf(filename, "users/%s.txt", email);
			user = fopen(filename, "r+");
			if(user){
				fscanf(user, "%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n", email, name, surname, residence, formation, year, skills);
				fclose(user);
				char msg1[] = "Insira o nome do usuário para o descadastramento\n";
				char new_name[MAX];
				write(connfd, msg1, sizeof(msg1));
				read(connfd, buff, sizeof(buff));
				strcpy(new_name, buff);
				if (strcmp(name, new_name) == 0){
					if (remove(filename) == 0) {
						char msg2[] = "Conta removida com sucesso!\n";
						write(connfd, msg2, sizeof(msg2));
					} else {
						char msg2[] = "Erro ao remover a conta!\n";
						write(connfd, msg2, sizeof(msg2));
					}
				} else {
					char msg2[] = "Nome do usuário incorreto!\n";
					write(connfd, msg2, sizeof(msg2));
				}
			}
			else{
				char msg2[] = "Conta não encontrada!\n";
				write(connfd, msg2, sizeof(msg2));
			}
			
		}
		else if (strncmp("3", buff, 1) == 0) {
			DIR *dir;
			struct dirent *ent;
			struct stat st;
			char filename[MAX], target[MAX];
			dir = opendir("users");
			bzero(buff, MAX);
            char msg[] = "Qual ano deseja pesquisar?\n";
			write(connfd, msg, sizeof(msg));
            read(connfd, buff, sizeof(buff));
            strcpy(target, buff);
			char msg1[MAX*2] = "\n";
			while ((ent = readdir(dir)) != NULL) {
				sprintf(filename, "users/%s", ent->d_name);
				user = fopen(filename, "r+");
				if(user){
					fscanf(user, "%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n", email, name, surname, residence, formation, year, skills);
					fclose(user);
					if (strncmp(year, target, strlen(year)) == 0){
						sprintf(msg1, "%sEmail:%s | Nome:%s\n", msg1, email, name);
					}
				}
			}
			write(connfd, msg1, sizeof(msg1));
		}
		else {
			write(connfd, buff, sizeof(buff));
		}
		bzero(buff, MAX);
		n = 0;
		// copy server message in the buffer
		//while ((buff[n++] = getchar()) != '\n')
		//	;

		// and send that buffer to client

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

	//make sure /users exist
	struct stat st = {0};
	if (stat("/users", &st) == -1) {
		// create directory if it does not exist
		mkdir("/users", 0700);
	}

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
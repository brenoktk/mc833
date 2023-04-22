
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX 80
#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold


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
			size_t len = strlen(email);
			if (email[len-1] == '\n') {
				email[len-1] = '\0';
			}
			char filename[MAX];
			sprintf(filename, "users/%s.txt", email);
			user = fopen(filename, "w+");
			fprintf(user, "%s\n%s%s", email, name, surname);
			fclose(user);
			char msg3[] = "Cadastro realizado!\n";
			write(connfd, msg, sizeof(msg));
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
				fclose(user);
				if (remove(filename) == 0) {
					char msg[] = "Conta removida com sucesso!\n";
					write(connfd, msg, sizeof(msg));
				} else {
					char msg[] = "Erro ao remover a conta!\n";
					write(connfd, msg, sizeof(msg));
    			}
			}
			else{
				char msg[] = "Conta não encontrada!\n";
				write(connfd, msg, sizeof(msg));
			}
			
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

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
        // Function for chatting between client and server
	        func(new_fd);
            close(sockfd); // child doesn't need the listener
            if (send(new_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}
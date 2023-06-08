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

#define SERVERPORT "8000"    // the port users will be connecting to
#define MAX_CHUNK_SIZE 1024  // Maximum size of each message chunk

// Function to receive chunks from the listener
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

        fwrite(buf, 1, numbytes, file);

        // Assuming the last chunk received has size less than MAX_CHUNK_SIZE, it indicates the end of the file
        if (numbytes < MAX_CHUNK_SIZE) {
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
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
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
        printf("talker: what user to download image from?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 0;
    }else if(!(strncmp(request, "register", strlen("register")))){
        printf("talker: what is the user email?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = ' ';
        printf("talker: what is the user's name?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = ' ';
        printf("talker: what is the user's surname?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = ' ';
        printf("talker: what is the user's residence?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = ' ';
        printf("talker: what is the user's formation?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = ' ';
        printf("talker: what is the user's graduation year?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = ' ';
        printf("talker: what is the user's skills?\n");
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op=1;
    }else if(!(strncmp(request, "srch_frmt", strlen("srch_frmt")))){
        printf("talker: what formation to search?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 2;
    }else if(!(strncmp(request, "srch_skill", strlen("srch_skill")))){
        printf("talker: what skill to search?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 3;
    }else if(!(strncmp(request, "srch_year", strlen("srch_year")))){
        printf("talker: what year to search?\n");
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
        printf("talker: what email to search?\n");
        int n = strlen(request);
        request[n] = ' ';
        n++;
        while ((request[n++] = getchar()) != '\n')
            ;
        request[n-1] = '\0';
        op = 7;
    }else if(!(strncmp(request, "remove", strlen("remove")))){
        printf("talker: what email to remove?\n");
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
            rcv = receive_chunks(sockfd, p->ai_addr, p->ai_addrlen, file);
        }

        fclose(file);

        printf("talker: received file saved as 'received.png'\n");
    } else {
        char buf[MAX_CHUNK_SIZE];
        if (sendto(sockfd, request, strlen(request), 0, p->ai_addr, p->ai_addrlen) == -1) {
            perror("talker: sendto");
            exit(1);
        }
        numbytes = recvfrom(sockfd, buf, MAX_CHUNK_SIZE - 1, 0, p->ai_addr, &p->ai_addrlen);
        if (numbytes == -1) {
            perror("talker: recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';
        printf("server: %s", buf);
    }
    close(sockfd);
    return 0;
}

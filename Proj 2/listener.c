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

#define MYPORT "8005"    // the port users will be connecting to
#define MAX_CHUNK_SIZE 1024  // Maximum size of each message chunk

// Function to send chunks to the talker
void send_chunks(int sockfd, const struct sockaddr* addr, socklen_t addr_len, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("listener: fopen");
        exit(1);
    }

    char buf[MAX_CHUNK_SIZE];
    size_t numbytes;

    while ((numbytes = fread(buf, 1, MAX_CHUNK_SIZE, file)) > 0) {
        if (sendto(sockfd, buf, numbytes, 0, addr, addr_len) == -1) {
            perror("listener: sendto");
            exit(1);
        }
    }

    fclose(file);
}

int main(void) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char buf[MAX_CHUNK_SIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("listener: waiting to receive requests...\n");

    for (;;) {
        addr_len = sizeof their_addr;
        int numbytes = recvfrom(sockfd, buf, MAX_CHUNK_SIZE - 1, 0, (struct sockaddr*)&their_addr, &addr_len);
        if (numbytes == -1) {
            perror("listener: recvfrom");
            exit(1);
        }

        printf("buf");
        buf[numbytes] = '\0';

        printf("listener: received request - %s\n", buf);

        // Check if the request is for downloading an image
        if (strncmp(buf, "download",  strlen("download")) == 0) {
            // Send the image in chunks to the talker
            send_chunks(sockfd, (struct sockaddr*)&their_addr, addr_len, "../gaton.png");
        }else if (strncmp(buf, "exit", strlen("exit")) == 0) {
            // Send the image in chunks to the talker
            char* buf = "done";
            sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&their_addr, addr_len);
            break;
        }else if(!(strncmp(buf, "register", strlen("register")))){
            char* buf = "done";
            sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&their_addr, addr_len);
        }
    }

    close(sockfd);

    return 0;
}

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

#define SERVERPORT "8005"    // the port users will be connecting to

#define MAX_CHUNK_SIZE 1024  // Maximum size of each message chunk

// Function to split the message into chunks and send them
void send_chunks(int sockfd, const struct sockaddr* addr, socklen_t addr_len, const char* message, size_t message_len) {
    size_t offset = 0;
    size_t remaining = message_len;

    while (remaining > 0) {
        size_t chunk_size = remaining > MAX_CHUNK_SIZE ? MAX_CHUNK_SIZE : remaining;

        ssize_t numbytes = sendto(sockfd, message + offset, chunk_size, 0, addr, addr_len);
        if (numbytes == -1) {
            perror("talker: sendto");
            exit(1);
        }

        printf("talker: sent %zd bytes\n", numbytes);

        offset += numbytes;
        remaining -= numbytes;
    }
}

int main(int argc, char* argv[]) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    if (argc != 3) {
        fprintf(stderr, "usage: talker hostname file.png\n");
        exit(1);
    }

    char* filename = argv[2];
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(file_size);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for file\n");
        fclose(file);
        exit(1);
    }

    size_t read_bytes = fread(buffer, 1, file_size, file);
    fclose(file);

    if (read_bytes != file_size) {
        fprintf(stderr, "Failed to read file: %s\n", filename);
        free(buffer);
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

    send_chunks(sockfd, p->ai_addr, p->ai_addrlen, buffer, file_size);

    freeaddrinfo(servinfo);
    free(buffer);

    close(sockfd);

    return 0;
}

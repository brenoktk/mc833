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
#include <dirent.h>
#include <sys/stat.h>

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

void retrieve_user_list(int sockfd, const struct sockaddr* addr, socklen_t addr_len) {
    DIR *dir;
    struct dirent *ent;
    struct stat st;
    char filename[MAX_CHUNK_SIZE];
    dir = opendir("../users");

    char msg1[MAX_CHUNK_SIZE] = "";
    while ((ent = readdir(dir)) != NULL) {
        sprintf(filename, "../users/%s", ent->d_name);
        FILE *user = fopen(filename, "r+");
        if (user) {
            char email[MAX_CHUNK_SIZE], name[MAX_CHUNK_SIZE], surname[MAX_CHUNK_SIZE], residence[MAX_CHUNK_SIZE], formation[MAX_CHUNK_SIZE], year[MAX_CHUNK_SIZE], skills[MAX_CHUNK_SIZE];
            fscanf(user, "%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n", email, name, surname, residence, formation, year, skills);
            fclose(user);
            sprintf(msg1, "%sEmail:%s | Nome:%s | Sobrenome:%s | Residência:%s | Curso:%s | Ano:%s | Habilidades: %s\n", msg1, email, name, surname, residence, formation, year, skills);
        }
    }
    if (sendto(sockfd, msg1, sizeof(msg1), 0, addr, addr_len) == -1) {
        perror("listener: sendto");
        exit(1);
    }
}

void search_field(int sockfd, const struct sockaddr* addr, socklen_t addr_len, const char* field_name, const char* target) {
    DIR *dir;
    struct dirent *ent;
    struct stat st;
    char filename[MAX_CHUNK_SIZE];
    dir = opendir("../users");

    char msg1[MAX_CHUNK_SIZE] = "";
    int check = 0;
    while ((ent = readdir(dir)) != NULL) {
        sprintf(filename, "../users/%s", ent->d_name);
        FILE *user = fopen(filename, "r+");
        if (user) {
            char email[MAX_CHUNK_SIZE], name[MAX_CHUNK_SIZE], surname[MAX_CHUNK_SIZE], residence[MAX_CHUNK_SIZE], formation[MAX_CHUNK_SIZE], year[MAX_CHUNK_SIZE], skills[MAX_CHUNK_SIZE];
            fscanf(user, "%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n", email, name, surname, residence, formation, year, skills);
            fclose(user);

            char* target_field;
            if (strcmp(field_name, "frmt") == 0) {
                target_field = formation;
            } else if (strcmp(field_name, "skill") == 0) {
                target_field = skills;
            } else if (strcmp(field_name, "year") == 0) {
                target_field = year;
            } else {
                // Invalid field name
                return;
            }

            if (strstr(target_field, target) != NULL) {
                sprintf(msg1, "%sEmail:%s | Nome:%s\n", msg1, email, name);
                check = 1;
            }
        }
    }

    if (!(check)){
        char msgnf[] = "Não encontramos resultados";
        strcpy(msg1, msgnf);
    }

    if (sendto(sockfd, msg1, sizeof(msg1), 0, addr, addr_len) == -1) {
        perror("listener: sendto");
        exit(1);
    }
}

int main(void) {
    struct stat st = {0};
	if (stat("../users", &st) == -1) {
		// create directory if it does not exist
		mkdir("../users", 0700);
	}
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

        buf[numbytes] = '\0';

        printf("listener: received request - %s\n", buf);

        // Check if the request is for downloading an image
        if (strncmp(buf, "download", strlen("download")) == 0) {
            // Send the image in chunks to the talker
            send_chunks(sockfd, (struct sockaddr*)&their_addr, addr_len, "../gaton.png");
        } else if (strncmp(buf, "exit", strlen("exit")) == 0) {
            char* buf = "done";
            sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&their_addr, addr_len);
            break;
        } else if (strncmp(buf, "register", strlen("register")) == 0) {
            char* buf = "done";
            sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&their_addr, addr_len);
        } else if (strncmp(buf, "srch_frmt", strlen("srch_frmt")) == 0) {
            // Parse the target formation from the received message
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "srch_frmt %s", target);
            // Perform the search by calling the search_field function
            search_field(sockfd, (struct sockaddr*)&their_addr, addr_len, "frmt", target);
        } else if (strncmp(buf, "srch_skill", strlen("srch_skill")) == 0) {
            // Parse the target skill from the received message
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "srch_skill %s", target);
            // Perform the search by calling the search_field function
            search_field(sockfd, (struct sockaddr*)&their_addr, addr_len, "skill", target);
        } else if (strncmp(buf, "srch_year", strlen("srch_year")) == 0) {
            // Parse the target year from the received message
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "srch_year %s", target);
            // Perform the search by calling the search_field function
            search_field(sockfd, (struct sockaddr*)&their_addr, addr_len, "year", target);
        } else if (strncmp(buf, "list", strlen("list")) == 0) {
            // Retrieve the list of users and send it to the talker
            retrieve_user_list(sockfd, (struct sockaddr*)&their_addr, addr_len);
        } else if (strncmp(buf, "srch_usr", strlen("srch_usr")) == 0) {
            char* buf = "done";
            sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&their_addr, addr_len);
        } else if (strncmp(buf, "remove", strlen("remove")) == 0) {
            char* buf = "done";
            sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&their_addr, addr_len);
        }
    }

    close(sockfd);

    return 0;
}

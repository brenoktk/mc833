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
#include <dirent.h>
#include <sys/stat.h>
#include <regex.h>

#define MYPORT "8000"       //Número da porta
#define MAX_CHUNK_SIZE 1024 //Tamanho máximo de cada chunk de mensagem
#define MAX 100             // Tamanho máximo de cada informação de usuário

//Envia os chunks da imagem ao talker
void send_chunks(int sockfd, const struct sockaddr* addr, socklen_t addr_len, const char* target_email) {
    char filename[MAX_CHUNK_SIZE], buf[MAX_CHUNK_SIZE];
    sprintf(filename, "../imgs/%s.png", target_email);
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        sprintf(buf, "Imagem nao existe");
        if (sendto(sockfd, buf, sizeof(buf), 0, addr, addr_len) == -1) {
            perror("listener: sendto");
            exit(1);
        }
    } else{
        size_t numbytes;

        while ((numbytes = fread(buf, 1, MAX_CHUNK_SIZE, file)) > 0) {
            if (sendto(sockfd, buf, numbytes, 0, addr, addr_len) == -1) {
                perror("listener: sendto");
                exit(1);
            }
        }

        fclose(file);
    }
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
            char email[MAX], name[MAX], surname[MAX], residence[MAX], formation[MAX], year[MAX], skills[MAX];
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
            char email[MAX], name[MAX], surname[MAX], residence[MAX], formation[MAX], year[MAX], skills[MAX];
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
                // Nome inválido
                return;
            }

            if (strcasestr(target_field, target) != 0) {
                if (strcmp(field_name, "year") == 0){
                    sprintf(msg1, "%sEmail:%s | Nome:%s | Curso:%s\n", msg1, email, name, formation);
                    check = 1;
                }
                else{
                    sprintf(msg1, "%sEmail:%s | Nome:%s\n", msg1, email, name);
                    check = 1;
                }
            }
        }
    }

    if (!(check)){
        char msgnf[] = "Não encontramos resultados\n";
        strcpy(msg1, msgnf);
    }

    if (sendto(sockfd, msg1, sizeof(msg1), 0, addr, addr_len) == -1) {
        perror("listener: sendto");
        exit(1);
    }
}

void remove_user(int sockfd, const struct sockaddr* addr, socklen_t addr_len, const char* target_email) {
    FILE* user;
    char filename[MAX_CHUNK_SIZE], msg2[MAX_CHUNK_SIZE];
    sprintf(filename, "../users/%s.txt", target_email);
    user = fopen(filename, "r+");
    if (user) {
        if (remove(filename) == 0) {
            sprintf(msg2, "Conta removida com sucesso!\n");
        } else {
            sprintf(msg2, "Erro ao remover a conta!\n");
        }
    } else {
        sprintf(msg2, "Conta não encontrada!\n");
    }
    if (sendto(sockfd, msg2, sizeof(msg2), 0, addr, addr_len) == -1) {
        perror("listener: sendto");
        exit(1);
    }
}

void show_user_data(int sockfd, const struct sockaddr* addr, socklen_t addr_len, const char* target_email) {
    char email[MAX], name[MAX], surname[MAX], residence[MAX], formation[MAX], year[MAX], skills[MAX];
    FILE* user;
    char filename[MAX_CHUNK_SIZE], msg1[MAX_CHUNK_SIZE];;
    sprintf(filename, "../users/%s.txt", target_email);
    user = fopen(filename, "r+");
    if (user) {
        fscanf(user, "%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n%[^\n]%*c\n", email, name, surname, residence, formation, year, skills);
        fclose(user);
        sprintf(msg1, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n", email, name, surname, residence, formation, year, skills);
        
    } else {
        sprintf(msg1, "Conta não encontrada!\n");
    }
    if (sendto(sockfd, msg1, sizeof(msg1), 0, addr, addr_len) == -1) {
        perror("listener: sendto");
        exit(1);
    }
}

#include <regex.h>

void save_user_data(int sockfd, const struct sockaddr* addr, socklen_t addr_len, const char* email, const char* name, const char* surname, const char* residence, const char* formation, const char* year, const char* skills) {
    FILE* user;
    char filename[MAX_CHUNK_SIZE];
    sprintf(filename, "../users/%s.txt", email);
    user = fopen(filename, "w+");
    fprintf(user, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n", email, name, surname, residence, formation, year, skills);
    fclose(user);

    char msg[] = "Cadastro realizado!\n";
    if (sendto(sockfd, msg, sizeof(msg), 0, addr, addr_len) == -1) {
        perror("listener: sendto");
        exit(1);
    }
}

int validate_email(const char* email) {
    regex_t regex;
    int ret;

    //Compila a regex
    ret = regcomp(&regex, "^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.com$", REG_EXTENDED);
    if (ret != 0) {
        printf("Não foi possível compilar a regex\n");
        return 0;
    }

    //Executa a regex
    ret = regexec(&regex, email, 0, NULL, 0);
    if (ret != 0) {
        return 0;
    }

    //Libera os recursos da regex
    regfree(&regex);

    return 1;
}

int main(void) {
    struct stat st = {0};
	if (stat("../users", &st) == -1) {
		//cria a pasta se ela não existir
		mkdir("../users", 0700);
	}
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char buf[MAX_CHUNK_SIZE];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    //Itera por todos os resultados e vincula ao primeiro que puder
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
        fprintf(stderr, "listener: falha ao vincular o socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("listener: aguardando receber solicitações...\n");

    for (;;) {
        addr_len = sizeof their_addr;
        int numbytes = recvfrom(sockfd, buf, MAX_CHUNK_SIZE - 1, 0, (struct sockaddr*)&their_addr, &addr_len);
        if (numbytes == -1) {
            perror("listener: recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';

        printf("listener: solicitação recebida - %s\n", buf);

        //Verifica qual é a solicitação
        if (strncmp(buf, "download", strlen("download")) == 0) {
            //Envia a imagem em chunks para o talker
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "download %s", target);
            send_chunks(sockfd, (struct sockaddr*)&their_addr, addr_len, target);
        } else if (strncmp(buf, "exit", strlen("exit")) == 0) {
            char* buf = "done\n";
            sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&their_addr, addr_len);
            break;
        } else if (strncmp(buf, "register", strlen("register")) == 0) {
            char* token = strtok(buf, "\n");  //Separa a string de entrada pelo caractere de quebra de linha
            char* email = strtok(NULL, "\n");  //Obtém o email
            char* name = strtok(NULL, "\n");  //Obtém o nome
            char* surname = strtok(NULL, "\n");  //Obtém o sobrenome
            char* residence = strtok(NULL, "\n");  //Obtém a residência
            char* formation = strtok(NULL, "\n");  //Obtém a formação acadêmica
            char* year = strtok(NULL, "\n");  //Obtém o ano de formatura
            char* skills = strtok(NULL, "");  //Obtém as habilidades

            if (email == NULL || name == NULL || surname == NULL || residence == NULL || formation == NULL || year == NULL || skills == NULL) {
                char invalidFormatMsg[] = "Formato inválido. Tente novamente.\n";
                sendto(sockfd, invalidFormatMsg, sizeof(invalidFormatMsg), 0, (struct sockaddr*)&their_addr, addr_len);
                continue;
            }

            //Valida o email
            if (!validate_email(email)) {
                char invalidEmailMsg[] = "Email inválido. Tente novamente.\n";
                sendto(sockfd, invalidEmailMsg, sizeof(invalidEmailMsg), 0, (struct sockaddr*)&their_addr, addr_len);
                continue;
            }

            save_user_data(sockfd, (struct sockaddr*)&their_addr, addr_len, email, name, surname, residence, formation, year, skills);

            char successMsg[] = "Cadastro realizado!";
            sendto(sockfd, successMsg, sizeof(successMsg), 0, (struct sockaddr*)&their_addr, addr_len);
        } else if (strncmp(buf, "srch_frmt", strlen("srch_frmt")) == 0) {
            //Busca a formação alvo
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "srch_frmt %s", target);
            search_field(sockfd, (struct sockaddr*)&their_addr, addr_len, "frmt", target);
        } else if (strncmp(buf, "srch_skill", strlen("srch_skill")) == 0) {
            //Busca a habilidade alvo
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "srch_skill %s", target);
            search_field(sockfd, (struct sockaddr*)&their_addr, addr_len, "skill", target);
        } else if (strncmp(buf, "srch_year", strlen("srch_year")) == 0) {
            //Busca o ano alvo
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "srch_year %s", target);
            search_field(sockfd, (struct sockaddr*)&their_addr, addr_len, "year", target);
        } else if (strncmp(buf, "list", strlen("list")) == 0) {
            //Recupera a lista de usuários
            retrieve_user_list(sockfd, (struct sockaddr*)&their_addr, addr_len);
        } else if (strncmp(buf, "srch_usr", strlen("srch_usr")) == 0) {
            //Busca o email alvo
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "srch_usr %s", target);
            show_user_data(sockfd, (struct sockaddr*)&their_addr, addr_len, target);
        } else if (strncmp(buf, "remove", strlen("remove")) == 0) {
            //Remove o email alvo
            char target[MAX_CHUNK_SIZE];
            sscanf(buf, "remove %s", target);
            remove_user(sockfd, (struct sockaddr*)&their_addr, addr_len, target);
        } else if (strncmp(buf, "help", strlen("help")) == 0) {
            char helpMsg[] = "Comandos disponíveis:\n"
                            "register - Registrar um novo perfil\n"
                            "remove - Remover um perfil\n"
                            "list - Listar todos os perfis\n"
                            "srch_usr - Retornar informações sobre um perfil\n"
                            "srch_frmt - Listar todas as pessoas formadas em um curso\n"
                            "srch_skill - Listar todas as pessoas que possuem uma habilidade\n"
                            "srch_year - Listar todas as pessoas que se formaram em um ano\n"
                            "download - Baixar a imagem de um perfil\n"
                            "exit - Fechar o servidor\n";
            sendto(sockfd, helpMsg, sizeof(helpMsg), 0, (struct sockaddr*)&their_addr, addr_len);
        } else {
            char invalidCommandMsg[] = "Comando inválido. Tente novamente.\n";
            sendto(sockfd, invalidCommandMsg, sizeof(invalidCommandMsg), 0, (struct sockaddr*)&their_addr, addr_len);
        }
    }

    close(sockfd);

    return 0;
}

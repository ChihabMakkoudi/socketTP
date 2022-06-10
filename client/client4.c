#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT    5500
#define IP      "127.0.0.1"
#define MAXBUF  1024

int main() {
    struct sockaddr_in serv_addr;
    int     socket_ref;
    int     file;
    char    buf[MAXBUF];
    int     file_name_len, read_len;
    /* socket() */
    socket_ref = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_ref == -1) {
        return 1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);

    if(connect(socket_ref, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect : ");
        printf("fail to connect.\n");
        close(socket_ref);
        return 1;
    }
    printf("Files:\n______________________\n");
    while(1) {
        memset(buf, 0x00, MAXBUF);
        read_len = read(socket_ref, buf, MAXBUF);
        send(socket_ref, "ok", strlen("ok"), 0);
        if(read_len == EOF | read_len == 0 | strcmp(buf,"end")==0) {
            printf("______________________\n");
            break;
        }
        printf("%s\n",buf);
    }
    askServ:
    memset(buf, 0x00, MAXBUF);
    printf("requested file from the server:  ");
    scanf("%s", buf);
    if (strcmp(buf,"/end/")==0){
        send(socket_ref, buf, strlen(buf), 0);
        return 0;
        // read(socket_ref, buf, MAXBUF);
    }
    printf(" getting > %s\n", buf);
    file_name_len = strlen(buf);
    
    send(socket_ref, buf, file_name_len, 0);

    file = open(buf, O_WRONLY | O_CREAT | O_EXCL, 0700);
    if(!file) {
        perror("file open error : ");
        return -1;
    } 

    while(1) {
        memset(buf, 0x00, MAXBUF);
        read_len = read(socket_ref, buf, MAXBUF);
        write(file, buf, read_len);
        if(read_len == EOF | read_len == 0 | read_len < 1024) {
            printf("finish file\n");
            goto askServ;
            return -1;
        }
    }
    close(file);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // read, write
#include <arpa/inet.h>
#include <sys/types.h>  // socket, bind, accept, open
#include <sys/socket.h> // socket, bind, listen, accept
#include <sys/stat.h>   // open
#include <fcntl.h>      // open
#include <errno.h>
#include <dirent.h>
#include <stdio.h>

#define PORT    5500
#define MAXBUF  1024

int main() {
    int server_socket;
    int client_socket;
    int file; // file num
    struct sockaddr_in serveraddr, clientaddr;
    int client_len, read_len, file_read_len;    // length
    char buf[MAXBUF];

    int check_bind;
    client_len = sizeof(clientaddr);

    /* socket() */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1) {
        perror("socket error : ");
        exit(0);
    }

    /* bind() */
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family       = AF_INET;
    serveraddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    serveraddr.sin_port         = htons(PORT);

    if(bind(server_socket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) > 0) {
        perror("bind error : ");
        exit(0);
    }

    /* listen */
    if(listen(server_socket, 5) != 0) {
        perror("listen error : ");
    }

    while(1) {
        char file_name[MAXBUF]; // local val
        memset(buf, 0x00, MAXBUF);

        /* accept() */
        client_socket = accept(server_socket, (struct sockaddr *)&clientaddr, &client_len);
        printf("New Client Connect : %s\n", inet_ntoa(clientaddr.sin_addr));

 
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                // printf("dir:%s\n",dir->d_name);
                
                send(client_socket, dir->d_name, strlen(dir->d_name), 0);
                read_len = read(client_socket, buf, MAXBUF);
            }
            closedir(d);
            send(client_socket,"end",strlen("end"),0);
        }
        read_len = read(client_socket, buf, MAXBUF);
        getFile:
        printf("reading file \n");
        /* file name */
        read_len = read(client_socket, buf, MAXBUF);
        if (strcmp(buf,"/end/")==0){
            goto closeSocket;
        }
        if(read_len > 0) {
            strcpy(file_name, buf);
            printf("%s > %s\n", inet_ntoa(clientaddr.sin_addr), file_name);
        } else {
            close(client_socket);
            break;
        }

        /* create file */
        file = open(buf, O_RDONLY);
        if(!file) {
            perror("Error : ");
            return 1;
        }

        while(1) {
            memset(buf, 0x00, MAXBUF);
            read_len = read(file, buf, MAXBUF);
            send(client_socket, buf, read_len, 0);
            if(read_len == 0) {
                send(client_socket, buf, -1, 0);
                break;
            }
        }
        printf("finish file\n");
        close(file);
        goto getFile;
        closeSocket:
        close(client_socket);
    }
    close(server_socket);
    return 0;
}
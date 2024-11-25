#define _GNU_SOURCE
#include <limits.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <pthread.h>
#include <limits.h>
#include <stdbool.h>
#include "myqueue.h"

#define SERVERPORT 8989
#define BUFSIZE 12098
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100
#define THREAD_POOL_SIZE 20

pthread_t thread_pool[THREAD_POOL_SIZE];


typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void*  handle_connection(void* p_client_socket);
int check(int exp, const char *msg);
void* thread_function(void *arg);

int main(int argc, char **argv) {
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    // some threads to handle future connections
    for(int i=0;i<THREAD_POOL_SIZE;i++){
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), "Failed to create socket");

    // initialize the address struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVERPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    check(bind(server_socket, (SA *)&server_addr, sizeof(server_addr)), "Bind Failed!");
    check(listen(server_socket, SERVER_BACKLOG), "Listen Failed");

    while (true) {
        printf("Waiting for connection...\n");
        // wait for, and accept an incoming connection
        addr_size = sizeof(SA_IN);
        check(client_socket = accept(server_socket, (SA *)&client_addr, (socklen_t *)&addr_size),
              "accept failed");
        printf("Connected\n");

        // do something with the connection

    //     printf("closing connection\n");
	// pthread_t t;
	int *pclient = malloc(sizeof(int));
	*pclient = client_socket;
    enqueue(pclient);

	// pthread_create(&t, NULL, handle_connection, pclient);
	//handle_connection(pclient);

    }

    return 0;
}

int check(int exp, const char *msg) {
    if (exp == SOCKETERROR) {
        perror(msg);
        exit(1);
    }
    return exp;
}

void * thread_function(void *arg){
    while (true){
        int *pclient = dequeue();
        if(pclient != NULL){
            handle_connection(pclient);
        }
    }
}

void* handle_connection(void* p_client_socket) {
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket); //free the pointer since we won't be needing it anymore
    char buffer[BUFSIZE];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[PATH_MAX + 1];

    // read the client's message -- the name of the file to read
    while ((bytes_read = read(client_socket, buffer + msgsize, sizeof(buffer) - msgsize - 1)) > 0) {
        msgsize += bytes_read;
        if (msgsize > BUFSIZE - 1 || buffer[msgsize - 1] == '\n')
            break;
    }
    check(bytes_read, "recv error");
    buffer[msgsize - 1] = 0; // null terminate the message and remove the \n

    printf("REQUEST: %s\n", buffer);
    fflush(stdout);

    // validity check
    if (realpath(buffer, actualpath) == NULL) {
        printf("ERROR(bad path): %s\n", buffer);
        close(client_socket);
        return NULL;
    }

    // read file and send its contents to the client
    FILE *fp = fopen(actualpath, "r");
    if (fp == NULL) {
        printf("ERROR(open): %s\n", buffer);
        close(client_socket);
        return NULL;
    }
    sleep(1);

    // read file contents and send them to the client
    // note this is insecure
    // a real program would limit the client to certain files.
    while ((bytes_read = fread(buffer, 1, BUFSIZE, fp)) > 0) {
        printf("sending %zu bytes\n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    close(client_socket);
    fclose(fp);
    printf("closing connection\n");

    return NULL;
}


// Cwk2: server.c - multi-thgeted server using readn() and writen()

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "rdwrn.h"
#include <time.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/time.h>
#include <signal.h>

// thget function
void *client_handler(void *);

void handler(int sig);
void getStudentDetails(int);
void getRandomNumbers(int);
void getServerDetails(int);
void getFileNamesInServer(int);
void sendFile(int,char*);
void receiveFileName(int, char *);

//Global variables
char IP_ADDRESS[14];
int CONNFD;
struct timeval tv1, tv2;

// you shouldn't need to change main() in the server except the port number
int main(void)
{
    int listenfd = 0, connfd = 0;
    gettimeofday(&tv1, NULL);
    gettimeofday(&tv2, NULL);

    struct sigaction act;

    act.sa_handler = &handler;
    act.sa_flags = SA_SIGINFO;

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t socksize = sizeof(struct sockaddr_in);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(&act, '\0', sizeof(act));

    sigaction(SIGINT, &act, NULL);
      
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(50001);

    strcat(IP_ADDRESS, inet_ntoa(serv_addr.sin_addr));

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1) {
    	perror("Failed to listen");
    	exit(EXIT_FAILURE);
    }
    // end socket setup

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    printf("Waiting for a client to connect...\n");

    while (1) {

    	CONNFD = connfd = accept(listenfd, (struct sockaddr *) &client_addr, &socksize);

    	printf("Connection accepted...\n");
    	pthread_t sniffer_thget;
            // third parameter is a pointer to the thget function, fourth is its actual parameter
    	if (pthread_create(&sniffer_thget, NULL, client_handler, (void *) &connfd) < 0) {
    	    perror("could not create thget");
    	    exit(EXIT_FAILURE);
    	}

    	//Now join the thget , so that we dont terminate before the thget
    	pthread_join( sniffer_thget , NULL);
    	printf("Handler assigned\n");
    }

    // never reached...
    // ** should include a signal handler to clean up
    exit(EXIT_SUCCESS);
} // end main()

int get_option(int socket){
    size_t payload_length;
    int option = 0;

    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) &option, payload_length);

    return option;
}

// thget function - one instance of each for each connected client
// this is where the do-while loop will go
void *client_handler(void *socket_desc)
{
    //Get the socket descriptor
    int socket = *(int *) socket_desc;
    int option;
    char filename[30];
    
    do{
        option = get_option(socket);

        switch(option){
            case 1: getStudentDetails(socket); 
                break;
            case 2: getRandomNumbers(socket);
                break;
            case 3: getServerDetails(socket);
                break;
            case 4: getFileNamesInServer(socket);
                break;
            case 5: 
                    receiveFileName(socket, filename);
                    sendFile(socket, filename);
                break;
            case 6: 
                    shutdown(socket, SHUT_RDWR);
                    close(socket);
                break;
        }

        memset(&option, 0, sizeof(int));
        memset(filename, 0, strlen(filename)*sizeof(char));
    }while(option != 6);

    return 0;
}  // end client_handler()

void sendFile(int socket, char * name){
    unsigned char buff[BUFSIZ];
    char filename[50];

    strcat(filename,"./upload/");
    strcat(filename,name);

    printf("%s\n", filename);

    FILE *file = fopen(filename, "r");

    if (file != NULL) {
        size_t newLen = fread(buff, sizeof(unsigned char), BUFSIZ, file);
        if ( ferror(file) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            buff[newLen++] = '\0'; /* Just to be safe. */
        }

        writen(socket, (unsigned char *) &newLen, sizeof(size_t));   
        writen(socket, (unsigned char *) buff, newLen);

        printf("%s\n", buff);

        fclose(file);
    }else{

        filename[0]='-';
        filename[1]='\0';

        size_t n = strlen(filename) + 1;

        writen(socket, (unsigned char *) &n, sizeof(size_t));   
        writen(socket, (unsigned char *) filename, n);

        printf("No file found\n");
    }

    memset(filename, 0, strlen(filename) + 1);
}

void receiveFileName(int socket, char * name){
    size_t n;
    
    readn(socket, (unsigned char *) &n, sizeof(size_t));  
    readn(socket, (unsigned char *) name, n);
}

void getStudentDetails(int socket){
    char stundetDetails[100] = "Student Name: Ticio Victoriano, Student ID: S1803453, Server IP: ";
    strcat(stundetDetails, IP_ADDRESS);

    size_t n = strlen(stundetDetails) + 1;
    
    writen(socket, (unsigned char *) &n, sizeof(size_t));   
    writen(socket, (unsigned char *) stundetDetails, n);

    memset(stundetDetails, 0, strlen(stundetDetails) + 1);
}

void getRandomNumbers(int socket){
    char randNumbers[25];
    char number[4];

    srand(time(NULL));
    
    int i;

    for(i=0; i < 5; ++i){
        sprintf(number, "%d", ((rand()%1000)+1)); 
        strcat(randNumbers, number);
        strcat(randNumbers, " ");

        number[0]='\0';
    }

    size_t n = strlen(randNumbers) + 1;
    
    writen(socket, (unsigned char *) &n, sizeof(size_t));   
    writen(socket, (unsigned char *) randNumbers, n);

    memset(randNumbers, 0, strlen(randNumbers) + 1);
    memset(number, 0, strlen(number) + 1);
}   

void getServerDetails(int socket){ 
    struct utsname uts;

    if (uname(&uts) == -1) {
        perror("uname error");
    }

    size_t n = sizeof(uts);
    
    writen(socket, (unsigned char *) &n, sizeof(size_t));   
    writen(socket, (unsigned char *) &uts, n);

    memset(&uts, 0, sizeof(uts));
}

void getFileNamesInServer(int socket){
    char files[200];

    DIR *mydir;

    if ((mydir = opendir("upload")) == NULL) {
        perror("error reading the directory");
        files[0]='-';
        files[1]='\0';

        size_t n = strlen(files) + 1;

        writen(socket, (unsigned char *) &n, sizeof(size_t));   
        writen(socket, (unsigned char *) files, n);

        printf("\nNo file found\n");
        memset(files, 0, strlen(files) + 1);
        return;
    }

    struct dirent *entry = NULL;

    // returns NULL when dir contents all processed
    while ((entry = readdir(mydir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        strcat(files, entry->d_name);
        strcat(files, "\n"); 
    }

    size_t n = strlen(files) + 1;

    writen(socket, (unsigned char *) &n, sizeof(size_t));   
    writen(socket, (unsigned char *) files, n);
    
    memset(files, 0, strlen(files)+1);
}

void handler(int sig){
    printf("shutting down the server ...\n");
    printf ("Server execution time: = %f seconds\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
    
    shutdown(CONNFD, SHUT_RDWR);
    close(CONNFD);
    exit(0);
}

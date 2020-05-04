// Cwk2: client.c - message length headers with variable sized payloads
//  also use of readn() and writen() implemented in separate code module

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "rdwrn.h"
#include <sys/utsname.h>
#include <ctype.h>

void readStudentDetails(int);
void readRandomNumbers(int);
void readServerDetails(int);
void receiveFile(int, char*);
void readFileNamesInServer(int);
void sendFileName(int, char*);

void send_option(int socket, int *option){
    size_t n = sizeof(char);
    writen(socket, (unsigned char *) &n, sizeof(size_t));  
    writen(socket, (unsigned char *) option, n);
}

int main(void)
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    int option = 0;
    char filename[30];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	   perror("Error - could not create socket");
	   exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(50001);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
    	perror("Error - connect failed");
    	exit(1);
    } else{
        printf("Connected to server...\n");
    }

    do{

        printf("\n\n========================================================================");
        printf("\n                                   Menu                                  ");
        printf("\n========================================================================");
        printf("\n1. Display Student details and Server IP");
        printf("\n2. Display 5 random numbers");
        printf("\n3. Display the “uname” information of the server");
        printf("\n4. Display a list of regular files in server");
        printf("\n5. Copy a file from server");
        printf("\n6. Exit");
        printf("\noption: ");

        scanf("%d", &option);
        send_option(sockfd, &option);

        switch(option){
            case 1: readStudentDetails(sockfd); 
                break;
            case 2: readRandomNumbers(sockfd);
                break;
            case 3: readServerDetails(sockfd);
                break;
            case 4: readFileNamesInServer(sockfd);
                break;
            case 5:
                    sendFileName(sockfd, filename); 
                    receiveFile(sockfd, filename);
                break;
            case 6:
                    exit(0);
                break;
            default:
                printf("\n\nInvalid option\n\n");
            ;
        }

    }while(1);

    close(sockfd);
    exit(EXIT_SUCCESS);
} // end main()

void readStudentDetails(int socket){
    size_t payload_length;
    char string[100];

    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) string, payload_length);

    printf("\n\n*************************************************************************");
    printf("\n                   Student details & IP address                           ");
    printf("\n*************************************************************************");

    printf("\n%s\n", string);
}

void readRandomNumbers(int socket){
    size_t payload_length;
    char string[25];

    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) string, payload_length);

    printf("\n\n*************************************************************************");
    printf("\n                     Random genereted numbers:                           ");
    printf("\n*************************************************************************");

    printf("\n%s\n", string);
}

void readServerDetails(int socket){
    size_t payload_length;
    struct utsname uts;
    
    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) &uts, payload_length);

    printf("\n\n*************************************************************************");
    printf("\n                     “uname” information of the server                    ");
    printf("\n*************************************************************************");

    printf("\n");
    printf("Node name:    %s\n", uts.nodename);
    printf("System name:  %s\n", uts.sysname);
    printf("Release:      %s\n", uts.release);
    printf("Version:      %s\n", uts.version);
    printf("Machine:      %s\n", uts.machine);
    printf("\n");
}

void sendFileName(int socket, char * filename){
    printf("\nenter filename: ");
    scanf("%s", filename);
    size_t n = strlen(filename);
    
    writen(socket, (unsigned char *) &n, sizeof(size_t));  
    writen(socket, (unsigned char *) filename, n);
}

void receiveFile(int socket, char * filename){
    FILE *file;
    char buff[BUFSIZ];
    size_t n;

    readn(socket, &n, sizeof(size_t));
    readn(socket, buff, n);

    if(strcmp(buff,"-") == 0){
        printf("File %s not found\n", filename);
    }else{
        file = fopen(filename,"wb");
        rewind(file);
        fputs (buff, file);

        printf("\nFile %s copied successfully\n", filename);

        fclose(file);
    }
}

void readFileNamesInServer(int socket){
    size_t payload_length;
    char string[200];

    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) string, payload_length);
    
    if(strcmp(string,"-") == 0){
        printf("None file not found\n");
    }else{

        printf("\n\n*************************************************************************");
        printf("\n                     List of files in server                    ");
        printf("\n*************************************************************************");
        printf("\n\n%s\n", string);
    }
}
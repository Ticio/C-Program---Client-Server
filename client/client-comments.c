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


/*
 * Author: Ticio Victoriano
 * Parameters: socket and the menu option
 * Purpose: Send the option entered by the user to the server
 * Algorithm description: get the size of the int option as a character and send the option and its size as character to the server. 
 */
void send_option(int socket, int *option){
    size_t n = sizeof(char);
    writen(socket, (unsigned char *) &n, sizeof(size_t));  
    writen(socket, (unsigned char *) option, n);
}

/*
 * Author: Ticio Victoriano
 * Parameters: socket and the menu option
 * Purpose: run the program 
 */
int main(void)
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    int option = 0;
    char filename[30]; // variable hold the name of the filename to be copied - for option 5 (to copy a file from the server)

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

        scanf("%d", &option); // Read the option to be enter by the user
        send_option(sockfd, &option); // send the option to the server

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


/*
 * Author: Ticio Victoriano
 * Parameters: socket
 * Purpose: read the size of te data and the data sent by the server for option 1
 */
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


/*
 * Author: Ticio Victoriano
 * Parameters: socket
 * Purpose: read the size of te data and the data sent by the server for option 2
 */
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

/*
 * Author: Ticio Victoriano
 * Parameters: socket
 * Purpose: read the size of te data and the data sent by the server for option 3
 */
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

/*
 * Author: Ticio Victoriano
 * Parameters: socket adn filename
 * Purpose: send the filename to the server for the option 5
 */
void sendFileName(int socket, char * filename){
    printf("\nenter filename: ");
    scanf("%s", filename);
    size_t n = strlen(filename);
    
    writen(socket, (unsigned char *) &n, sizeof(size_t));  
    writen(socket, (unsigned char *) filename, n);
}

/*
 * Author: Ticio Victoriano
 * Parameters: socket and filename
 * Purpose: read the size of the data and the data sent by the server for option 5 - data of the file to be copied
 */
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
        rewind(file); //positions pointer to the start of the file
        fputs (buff, file); //write the content into the file in the client folder

        printf("\nFile %s copied successfully\n", filename);

        fclose(file);
    }
}

/*
 * Author: Ticio Victoriano
 * Parameters: socket
 * Purpose: read the size of te data and the data sent by the server for option 4
 */
void readFileNamesInServer(int socket){
    size_t payload_length;
    char string[200];

    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) string, payload_length);
    
    printf("\n\n*************************************************************************");
    printf("\n                     List of files in server                    ");
    printf("\n*************************************************************************");
    printf("\n\n%s\n", string);
}
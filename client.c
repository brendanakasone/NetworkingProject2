/*///////////////////////////////////////////////////////////
*
* FILE:		client.c
* AUTHOR:	Brenda Nakasone
* PROJECT:	CNT 4007 Project 1 - Professor Traynor
* DESCRIPTION:	Network Client Code
*
*////////////////////////////////////////////////////////////

/* Included libraries */

#include <stdio.h>		    /* for printf() and fprintf() */
#include <sys/socket.h>		    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <openssl/evp.h>	    /* for OpenSSL EVP digest libraries/SHA256 */

/* Constants */
#define RCVBUFSIZE 512		    /* The receive buffer size */
#define SNDBUFSIZE 512		    /* The send buffer size */
#define MDLEN 128

/* The main function */
int main(int argc, char *argv[])
{
    int clientSock;		    /* socket descriptor */
    struct sockaddr_in serv_addr;   /* The server address */

    char *studentName;		    /* Your Name */

    char sndBuf[SNDBUFSIZE];	    /* Send Buffer */
    char rcvBuf[RCVBUFSIZE];	    /* Receive Buffer */
    
    int i;			    /* Counter Value */

    /* Get the Student Name from the command line */
    if (argc != 2) 
    {
	printf("Incorrect input format. The correct format is:\n\tnameChanger your_name\n");
	exit(1);
    }
    studentName = argv[1];
    memset(&sndBuf, 0, RCVBUFSIZE);
    memset(&rcvBuf, 0, RCVBUFSIZE);

    /* Create a new TCP socket*/
    if ((clientSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("socket failed\n");
    };

    /* Construct the server address structure */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8080);

    /* Establish connecction to the server */
    if(connect(clientSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("connect failed\n");
    };

    // START OF SENDING/RECEIVING MESSAGES: 

    /* receiving menu options */
    int bytesReceived = recv(clientSock, rcvBuf, RCVBUFSIZE - 1, 0);
    rcvBuf[bytesReceived] = '\0';
    for(i = 0; i < MDLEN; i++) printf("%c", rcvBuf[i]);
    printf("\n");

    /* getting user menu selection from terminal */
    char temp[25];
    fgets(temp, sizeof(temp), stdin);
    temp[strcspn(temp, "\n")] = 0;
    temp[24] = '\0';
    char *menuOption = temp;

    /* send user menu selection to server */
    send(clientSock, menuOption, strlen(menuOption), 0);

    /* Send name to server */
    // if(send(clientSock, studentName, strlen(studentName), 0) < 0){
    //     printf("send failed\n");
    // };

    // CLEAR MEMORY 
    memset(rcvBuf, 0, RCVBUFSIZE);

    /* Receive transformed name from server */
    // bytesReceived = recv(clientSock, rcvBuf, RCVBUFSIZE - 1, 0);

    // printf("%s\n", studentName);
    /* Printing out transformed name from server */
    // printf("Transformed input is: ");
    // for(i = 0; i < MDLEN; i++) printf("%02x", rcvBuf[i]);
    // printf("\n");

    close(clientSock);

    return 0;
}
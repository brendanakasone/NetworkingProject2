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

typedef struct{
    char name[50];
    long contents; 
    FILE *fileptr;
} file;

void listFilesFunc(int clientSock, char rcvBuf[RCVBUFSIZE]){
  // receiving list files 
  recv(clientSock, rcvBuf, RCVBUFSIZE - 1, 0);
  printf("List of Server Files:\n");
  for(int i = 0; i < MDLEN; i++) printf("%c", rcvBuf[i]);
  printf("\n");
} 

/* The main function */
int main(int argc, char *argv[])
{
    int clientSock;		    /* socket descriptor */
    struct sockaddr_in serv_addr;   /* The server address */

    char *studentName;		    /* Your Name */

    char sndBuf[SNDBUFSIZE];	    /* Send Buffer */
    char rcvBuf[RCVBUFSIZE];	    /* Receive Buffer */
    
    int i;			    /* Counter Value */

    /* File Storage */
    int fileStorageSize = 5; // hard coded for now 
    file* fileStorage = (file*)malloc(fileStorageSize * sizeof(file));

    if (fileStorage == NULL){
      printf("memory allocation failed\n");
    }

    // hard code for now 
    for (int i = 0; i < fileStorageSize; i++){
      snprintf(fileStorage[i].name, sizeof(fileStorage[i].name), "File %d", i + 2);
      fileStorage[i].contents = 99 + i;
    }

    // making sure the dynamic array works 
    printf("\nDynamic array checking (Client Files):\n");
    for(int i = 0; i < fileStorageSize; i++){
      printf("Names of files: %s, Contents: %ld\n", fileStorage[i].name, fileStorage[i].contents);
    }

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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.4");
    serv_addr.sin_port = htons(8086);

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

    /*clearing rcvBuf*/
    memset(rcvBuf, 0, RCVBUFSIZE);

    /*if list files is called*/
    if (strcmp(menuOption, "List Files") == 0){
      listFilesFunc(clientSock, rcvBuf);
    }
    /*if diff is called*/
    else if(strcmp(menuOption, "Diff") == 0){
      // calling list files for server files
      listFilesFunc(clientSock, rcvBuf);

      // getting full list of client files
      char clientFileList[50*fileStorageSize];
      strcpy(clientFileList, fileStorage[0].name);
      strcat(clientFileList, "\n");
      for (int i = 1; i < fileStorageSize; i++){
        strcat(clientFileList, fileStorage[i].name);
        strcat(clientFileList, "\n");
      }

      // comparing file names and creating a new diff file list 
      char *token; 
      char diffFileList[50*fileStorageSize];
      token = strtok(rcvBuf, "\n");
      while(token != NULL){
        const char *found = strstr(clientFileList, token);
        if (found) {
          printf("%s was found\n", token);
        }
        else{
          strcpy(diffFileList, token);
          printf("%s was NOT found\n", token);
        }
        token = strtok(NULL, "\n");
      }

      printf("This is diff file: %s", diffFileList);
    }

    close(clientSock);

    return 0;
}
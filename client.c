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
    long long contents; 
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

    char sndBuf[SNDBUFSIZE];	    /* Send Buffer */
    char rcvBuf[RCVBUFSIZE];	    /* Receive Buffer */
    
    int i;			    /* Counter Value */

    /* File Storage */
    int fileStorageSize = 0;
    file* fileStorage = (file*)malloc(fileStorageSize * sizeof(file));

    if (fileStorage == NULL){
      printf("memory allocation failed\n");
    }

    // opening the file
    FILE *f1 = fopen("clientFiles/test1.txt", "rb");
    if (f1 == NULL){
      printf("error opening file\n");
    }
    else {
      printf("test1.txt opened successsfully\n");
    }

    // adding binary file together for contents
    int v; 
    long long cl = 0; 
    size_t readCount1; 
    // loops through the file and adds the contents of the file together
    while((readCount1 = fread(&v, sizeof(int), 1, f1)) == 1){
      cl += v;
    }

    printf("Here are the values to everything: %lld\n", cl);

    char fname[] = "clientFiles/test1.txt";

    // creating new struct
    file *fstruct = malloc(sizeof(f1) + sizeof(fname) + sizeof(cl)); 
    strcpy(fstruct->name, fname);
    fstruct->contents = cl;
    fstruct->fileptr = f1;

    fclose(f1);

    // reallocating memory array 
    fileStorageSize++;
    file* fs = (file*)realloc(fileStorage, sizeof(fileStorage) + sizeof(fstruct));
    if (fs == NULL){
      printf("Reallocation failed\n");
      free(fileStorage);
      return 1;
    }
    fileStorage = fs;

    // adding new struct to the array
    fileStorage[fileStorageSize-1] = *fstruct;

    // making sure the dynamic array works 
    printf("\nDynamic array checking (Client Files):\n");
    for(int i = 0; i < fileStorageSize; i++){
      printf("Names of files: %s, Contents: %lld\n", fileStorage[i].name, fileStorage[i].contents);
      if(fileStorage[i].fileptr != NULL){
        printf("file is set");
      }
    }

    // clearing memory 
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
    serv_addr.sin_port = htons(8087);

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
      printf("This is diff file: %s\n", diffFileList);

      memset(clientFileList, 0, sizeof(clientFileList));
      memset(diffFileList, 0, sizeof(diffFileList));
    }

    free(fileStorage);
    close(clientSock);

    return 0;
}
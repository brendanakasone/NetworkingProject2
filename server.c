/*///////////////////////////////////////////////////////////
*
* FILE:		server.c
* AUTHOR:	Brenda Nakasone
* PROJECT:	CNT 4007 Project 1 - Professor Traynor
* DESCRIPTION:	Network Server Code
*
*////////////////////////////////////////////////////////////

/*Included libraries*/

#include <stdio.h>	  /* for printf() and fprintf() */
#include <sys/socket.h>	  /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>	  /* supports all sorts of functionality */
#include <unistd.h>	  /* for close() */
#include <string.h>	  /* support any string ops */
#include <openssl/evp.h>  /* for OpenSSL EVP digest libraries/SHA256 */

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define BUFSIZE 40		/* Your name can be as many as 40 chars*/

typedef struct{
    char name[50];
    long contents; 
    FILE *fileptr;
} file;

void listFilesFunc(int fileStorageSize, file* fileStorage, int clientSock){
  // sending server files to client 
  size_t sizeOfFileNames = 0;
  for(int i = 0; i < fileStorageSize; i++){
    sizeOfFileNames = sizeOfFileNames + strlen(fileStorage[i].name + 5);
  }
  char *allFileNames = (char *)malloc(sizeOfFileNames * sizeof(char));
  strcpy(allFileNames, fileStorage[0].name);
  for(int i = 1; i < fileStorageSize; i++){
    strcat(allFileNames, "\n");
    strcat(allFileNames, fileStorage[i].name);
  }

  send(clientSock, allFileNames, strlen(allFileNames), 0);
  printf("Successfully sent server files\n");
} 

/* The main function */
int main(int argc, char *argv[])
{
    int serverSock;				/* Server Socket */
    int clientSock;				/* Client Socket */
    struct sockaddr_in changeServAddr;		/* Local address */
    struct sockaddr_in changeClntAddr;		/* Client address */
    unsigned short changeServPort;		/* Server port */
    unsigned int clntLen;			/* Length of address data struct */

    char nameBuf[BUFSIZE];			/* Buff to store name from client */
    unsigned char md_value[EVP_MAX_MD_SIZE];	/* Buff to store change result */
    EVP_MD_CTX *mdctx;				/* Digest data structure declaration */
    const EVP_MD *md;				/* Digest data structure declaration */
    int md_len;					/* Digest data structure size tracking */

    /* File Storage */
    int fileStorageSize = 5; // hard coded for now 
    file* fileStorage = (file*)malloc(fileStorageSize * sizeof(file));

    if (fileStorage == NULL){
      printf("memory allocation failed\n");
    }

    // hard code for now 
    for (int i = 0; i < fileStorageSize; i++){
      snprintf(fileStorage[i].name, sizeof(fileStorage[i].name), "File %d", i + 1);
      fileStorage[i].contents = 100 + i;
    }

    // making sure the dynamic array works 
    printf("\nDynamic array checking (Server files):\n");
    for(int i = 0; i < fileStorageSize; i++){
      printf("Names of files: %s, Contents: %ld\n", fileStorage[i].name, fileStorage[i].contents);
    }

    /* Create a new TCP socket*/
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("socket failed\n");
    };

    /* Construct local address structure*/
    memset(&changeServAddr, 0, sizeof(changeServAddr));
    changeServAddr.sin_family = AF_INET;
    changeServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    changeServAddr.sin_port = htons(8086);
    
    /* Bind to local address structure */
    if (bind(serverSock, (struct sockaddr *)&changeServAddr, sizeof(changeServAddr)) < 0){
      printf("bind failed\n");
    };

    /* Listen for incoming connections */
    if(listen(serverSock, 10) < 0){
      printf("listen failed\n");
    };

    int leave = 0;

    /* Loop server forever*/
    while(leave == 0)
    {
      clntLen = sizeof(changeClntAddr);
	    /* Accept incoming connection */
      if((clientSock = accept(serverSock, (struct sockaddr *)&changeClntAddr, &clntLen)) < 0){
        printf("accept failed\n");
      };

      // START OF SENDING/RECEIVING MESSAGES

      // menu options being sent to client
      char *message = "Please pick one of the following options:\n1. List Files\n2. Diff\n3. Pull\n4. Leave";
      send(clientSock, message, strlen(message), 0);

	    /* Get user option from client */
      int bytesReceived = recv(clientSock, nameBuf, BUFSIZE - 1, 0); 
      nameBuf[bytesReceived] = '\0'; 
      for(int i = 0; i < BUFSIZE; i++) printf("%c", nameBuf[i]);
      printf("\n");

      nameBuf[strcspn(nameBuf, "\n")] = 0; 

      if (strcmp(nameBuf, "List Files") == 0) 
      {
        listFilesFunc(fileStorageSize, fileStorage, clientSock);
      } 
      else if (strcmp(nameBuf, "Diff") == 0)
      {
        listFilesFunc(fileStorageSize, fileStorage, clientSock);
        printf("Successfully received Diff\n");
      }
      else if (strcmp(nameBuf, "Pull") == 0)
      {
        printf("Successfully received Pull\n");
      }
      else if (strcmp(nameBuf, "Leave") == 0)
      {
        printf("Successfully received Leave\n");
        leave = 1;
      }
      else 
      {
        printf("Please retry!\n");
      }

      // size_t nameBufSize = sizeof(nameBuf);
      // size_t listFilesSize = sizeof(listFiles);
      // size_t actualNameBufLength = strlen(nameBuf); // Length of actual data in nameBuf

      // printf("This is the size of nameBuf (array size): %zu bytes.\n", nameBufSize);
      // printf("This is the actual length of data in nameBuf: %zu bytes.\n", actualNameBufLength);
      // printf("Tshis is the size of listFiles: %zu bytes.\n", listFilesSize);

    
    }
    free(fileStorage);
    close(clientSock);
    close(serverSock);

    return 0;
}
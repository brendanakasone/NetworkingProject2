/*///////////////////////////////////////////////////////////
*
* FILE:		server.c
* AUTHOR:	Brenda Nakasone
* PROJECT:	CNT 4007 Project 1 - Professor Traynor
* DESCRIPTION:	Network Server Code
*
*////////////////////////////////////////////////////////////

/*Included libraries*/

// fix diff so that contents are not the same 
// update documentation 
// figure out client history txt 

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

  // calculating how much memory to allocate
  size_t sizeOfFileNames = 0;
  for(int i = 0; i < fileStorageSize; i++){
    sizeOfFileNames += strlen(fileStorage[i].name);
  }

  // allocating memory
  char *allFileNames = (char *)malloc((sizeOfFileNames + fileStorageSize) * sizeof(char));
  if (allFileNames == NULL){
    printf("allFileNames failed memoryalloc");
    exit(1);
  }

  // adding strings to the array
  strcpy(allFileNames, fileStorage[0].name);
  for(int i = 1; i < fileStorageSize; i++){
    strcat(allFileNames, "\n");
    strcat(allFileNames, fileStorage[i].name);
  }

  // sending server file names and freeing memory 
  send(clientSock, allFileNames, strlen(allFileNames), 0);
  printf("Successfully sent server files\n");
  free(allFileNames);
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
    int fileStorageSize = 0; // hard coded for now 
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

    char fname[] = "serverFiles/test1.txt";

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

    // // hard code for now 
    // for (int i = 0; i < fileStorageSize; i++){
    //   snprintf(fileStorage[i].name, sizeof(fileStorage[i].name), "File %d", i + 1);
    //   fileStorage[i].contents = 100 + i;
    // }

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
    changeServAddr.sin_port = htons(8087);
    
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

      /* removing new line */
      nameBuf[strcspn(nameBuf, "\n")] = 0; 

      /*user options*/
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

      memset(nameBuf, 0, BUFSIZE);
    
    }
    free(fileStorage);
    close(clientSock);
    close(serverSock);

    return 0;
}
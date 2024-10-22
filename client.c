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

#include <libgen.h>

/* Constants */
#define RCVBUFSIZE 512		    /* The receive buffer size */
#define SNDBUFSIZE 512		    /* The send buffer size */
#define MDLEN 128

typedef struct{
    char name[50];
    long long contents; 
    FILE *fileptr;
} file;

typedef struct {
    int clientSock;
} threadArgs;

void listFilesFunc(int clientSock, char rcvBuf[RCVBUFSIZE]){
  // receiving list files 
  recv(clientSock, rcvBuf, RCVBUFSIZE - 1, 0);
  printf("List of Server Files:\n");
  for(int i = 0; i < MDLEN; i++) printf("%c", rcvBuf[i]);
  printf("\n");
} 

void *receiveMessages(void *args) {
    threadArgs *tArgs = (threadArgs *)args;
    int clientSock = tArgs->clientSock;
    char rcvBuf[RCVBUFSIZE];
    
    while (1) {
        memset(rcvBuf, 0, RCVBUFSIZE);
        int bytesReceived = recv(clientSock, rcvBuf, RCVBUFSIZE - 1, 0);
        if (bytesReceived <= 0) {
            printf("Connection closed or error occurred\n");
            break;
        }
        rcvBuf[bytesReceived] = '\0';
        printf("Received: %s\n", rcvBuf);
        pthread_exit(NULL);
    }
    
    close(clientSock);
    free(tArgs);
    pthread_exit(NULL);
}

void handleMenu(int clientSock, file* fileStorage, int fileStorageSize, char *folderPath){
    char rcvBuf[RCVBUFSIZE];
    char sndBuf[SNDBUFSIZE];
    char temp[25];

    int bytesReceived = recv(clientSock, rcvBuf, RCVBUFSIZE - 1, 0);
    rcvBuf[bytesReceived] = '\0';
    printf("%s\n", rcvBuf);

    fgets(temp, sizeof(temp), stdin);
    temp[strcspn(temp, "\n")] = 0;
    temp[24] = '\0';
    char *menuOption = temp;

    send(clientSock, menuOption, strlen(menuOption), 0);
    memset(rcvBuf, 0, RCVBUFSIZE);

    if (strcmp(menuOption, "List Files") == 0){
        listFilesFunc(clientSock, rcvBuf);
    } else if(strcmp(menuOption, "Diff") == 0){
        listFilesFunc(clientSock, rcvBuf);

        char clientFileList[50*fileStorageSize];
        strcpy(clientFileList, fileStorage[0].name);
        strcat(clientFileList, "\n");
        for (int i = 1; i < fileStorageSize; i++){
            strcat(clientFileList, fileStorage[i].name);
            strcat(clientFileList, "\n");
        }

        char *token;
        char diffFileList[50*fileStorageSize];
        token = strtok(rcvBuf, "\n");
        while(token != NULL){
            const char *found = strstr(clientFileList, token);
            if (found) {
                printf("%s was found\n", token);
            } else{
                strcpy(diffFileList, token);
                printf("%s was NOT found\n", token);
            }
            token = strtok(NULL, "\n");
        }
        printf("This is diff file: %s\n", diffFileList);
    } else if(strcmp(menuOption, "Pull") == 0) {
        printf("Requesting missing files from server...\n");

        send(clientSock, "Pull", strlen("Pull"), 0);

        for (int i = 0; i < fileStorageSize; i++) {
            char fileName[100];
            memset(fileName, 0, sizeof(fileName));
            int bytesReceived = recv(clientSock, fileName, sizeof(fileName) - 1, 0);
            if (bytesReceived <= 0) {
                printf("No more files to receive or error occurred.\n");
                break;
            }

            fileName[bytesReceived] = '\0';
            printf("Receiving file: %s\n", fileName);

            char *baseFileName = basename(fileName);
            printf("Extracted base file name: %s\n", baseFileName);

            long fileSize;
            recv(clientSock, &fileSize, sizeof(fileSize), 0);
            printf("File size: %ld bytes\n", fileSize);

            char fullFilePath[100];
            if (folderPath[(strlen(folderPath) - 1)] != '/') {
                snprintf(fullFilePath, sizeof(fullFilePath), "%s/%s", folderPath, baseFileName);
            } else {
                snprintf(fullFilePath, sizeof(fullFilePath), "%s%s", folderPath, baseFileName);
            }

            // open file in write mode
            FILE *f = fopen(fullFilePath, "wb");
            if (f == NULL) {
                printf("Error opening file to write: %s\n", fullFilePath);
                continue;
            }

            long totalReceived = 0; 
            char fileBuffer[RCVBUFSIZE];
            while(totalReceived < fileSize) {
                bytesReceived = recv(clientSock, fileBuffer, sizeof(fileBuffer), 0);
                fwrite(fileBuffer, 1, bytesReceived, f);
                totalReceived += bytesReceived;
            }

            fclose(f);
            printf("File received successfully.\n");
        }
    } else if (strcmp(menuOption, "Leave") == 0) {
        printf("Leaving the server...\n");
        close(clientSock);
        exit(0);
    } else {
        printf("Invalid option. Please try again!\n");
    }
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
    int fileStorageSize = 0;
    file* fileStorage = (file*)malloc(fileStorageSize * sizeof(file));

    if (fileStorage == NULL){
      printf("memory allocation failed\n");
    }

    if (argc < 2) {
        printf("Usage: ./client folder_path\n");
        exit(1);
    }

    char *folderPath;
    // char fname[100];
    // opening the file
    // snprintf(fname, sizeof(fname), "%s/test1.txt", folderPath);

    // opening the file
    FILE *f1 = fopen("/cise/homes/b.nakasone/NetworkingProject2/clientFiles1/text0.txt", "rb");
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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.9");
    serv_addr.sin_port = htons(8083);

    /* Establish connecction to the server */
    if(connect(clientSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("connect failed\n");
    };

    while (1) {
        handleMenu(clientSock, fileStorage, fileStorageSize, folderPath);
    }
    // // START OF SENDING/RECEIVING MESSAGES: 

    // /* receiving menu options */
    // int bytesReceived = recv(clientSock, rcvBuf, RCVBUFSIZE - 1, 0);
    // rcvBuf[bytesReceived] = '\0';
    // for(i = 0; i < MDLEN; i++) printf("%c", rcvBuf[i]);
    // printf("\n");

    // /* getting user menu selection from terminal */
    // char temp[25];
    // fgets(temp, sizeof(temp), stdin);
    // temp[strcspn(temp, "\n")] = 0;
    // temp[24] = '\0';
    // char *menuOption = temp;

    // /* send user menu selection to server */
    // send(clientSock, menuOption, strlen(menuOption), 0);

    // /*clearing rcvBuf*/
    // memset(rcvBuf, 0, RCVBUFSIZE);

    // /*if list files is called*/
    // if (strcmp(menuOption, "List Files") == 0){
    //   listFilesFunc(clientSock, rcvBuf);
    // }
    // /*if diff is called*/
    // else if(strcmp(menuOption, "Diff") == 0){
    //   // calling list files for server files
    //   listFilesFunc(clientSock, rcvBuf);

    //   // getting full list of client files
    //   char clientFileList[50*fileStorageSize];
    //   strcpy(clientFileList, fileStorage[0].name);
    //   strcat(clientFileList, "\n");
    //   for (int i = 1; i < fileStorageSize; i++){
    //     strcat(clientFileList, fileStorage[i].name);
    //     strcat(clientFileList, "\n");
    //   }

    //   // comparing file names and creating a new diff file list 
    //   char *token; 
    //   char diffFileList[50*fileStorageSize];
    //   token = strtok(rcvBuf, "\n");
    //   while(token != NULL){
    //     const char *found = strstr(clientFileList, token);
    //     if (found) {
    //       printf("%s was found\n", token);
    //     }
    //     else{
    //       strcpy(diffFileList, token);
    //       printf("%s was NOT found\n", token);
    //     }
    //     token = strtok(NULL, "\n");
    //   }
    //   printf("This is diff file: %s\n", diffFileList);

    //   memset(clientFileList, 0, sizeof(clientFileList));
    //   memset(diffFileList, 0, sizeof(diffFileList));
    // }

    close(clientSock);
    free(fileStorage);
    
    return 0;
}
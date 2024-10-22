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
#include <dirent.h>

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
  printf("\n\n");
} 

void *receiveMessages(void *args) {
    // threading
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

    // receiving menu
    char *message = "Please pick one of the following options:\n1. List Files\n2. Diff\n3. Pull\n4. Leave";
    printf("%s\n", message);

    // command line user interaction
    fgets(temp, sizeof(temp), stdin);
    temp[strcspn(temp, "\n")] = 0;
    temp[24] = '\0';
    char *menuOption = temp;

    // sending user selection to server 
    send(clientSock, menuOption, strlen(menuOption), 0);
    memset(rcvBuf, 0, RCVBUFSIZE);

    if (strcmp(menuOption, "List Files") == 0){
        listFilesFunc(clientSock, rcvBuf);
    } 
    else if(strcmp(menuOption, "Diff") == 0){
        listFilesFunc(clientSock, rcvBuf);

        // get the list of file names from server
        // get the list of contents from server
        // compare the contents of the server to the client 
        // list the file names of different server files

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
    } 
    else if(strcmp(menuOption, "Pull") == 0) {
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
    } 
    else if (strcmp(menuOption, "Leave") == 0) {
        printf("Leaving the server...\n");
        close(clientSock);
        exit(0);
    } 
    else {
        printf("Invalid option. Please try again!\n");
    }
}

file openFile(char* filePath){
    // opening the file
    FILE *f1 = fopen(filePath, "rb");
    if (f1 == NULL){
      printf("error opening file\n");
    }
    else {
      printf("file opened successsfully\n");
    }

    int v; 
    long long cl = 0; 
    size_t readCount1; 
    // loops through the file and adds the contents of the file together
    while((readCount1 = fread(&v, sizeof(int), 1, f1)) == 1){
      cl += v;
    }
    printf("Here are the values to everything: %lld\n", cl);

    // creating a new file struct
    file fstruct; 
    strcpy(fstruct.name, filePath);
    fstruct.contents = cl;
    fstruct.fileptr = f1;

    printf("Values after struct created: %lld\n", cl);

    fclose(f1);

    return fstruct;
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

    char *filePath;

    // initializing directory
    struct dirent *pDirent; 
    DIR *pDir; 

    // initializing file storage
    int fileStorageSize = 0;
    file* fileStorage = (file*)malloc(fileStorageSize* sizeof(file));
    if (fileStorage == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    // opening directory
    pDir = opendir(argv[1]);
    if (pDir == NULL){
        printf("directory did not open correctly");
    }

    // looping through files in directory
    while((pDirent = readdir(pDir)) != NULL){
        // skip hidden files
        if (pDirent->d_name[0] == '.') { 
            continue;
        }
        // adding files to filestorage
        else {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", argv[1], pDirent->d_name);

            file newFile = openFile(path);
            printf("new file: %s, %lld\n", newFile.name, newFile.contents);
            // reallocating memory array 
            fileStorageSize++;
            file* temp = (file*)realloc(fileStorage, fileStorageSize * sizeof(file));
            if (temp == NULL){
                printf("Reallocation failed\n");
                free(fileStorage);
                return 1;
            }
            fileStorage = temp;
            fileStorage[fileStorageSize-1] = newFile;
            memset(path, 0, 1024);            
        }
    }

    closedir(pDir);

    // making sure the dynamic array works 
    printf("\nDynamic array checking (Client Files):\n");
    for(int i = 0; i < fileStorageSize; i++){
      printf("Names of files: %s, Contents: %lld\n", fileStorage[i].name, fileStorage[i].contents);
      if(fileStorage[i].fileptr != NULL){
        printf("file is set\n");
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
    serv_addr.sin_port = htons(8082);

    /* Establish connecction to the server */
    if(connect(clientSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("connect failed\n");
    };

    while (1) {
        handleMenu(clientSock, fileStorage, fileStorageSize, filePath);
    }

    close(clientSock);
    free(fileStorage);
    
    return 0;
}
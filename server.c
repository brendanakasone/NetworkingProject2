/*///////////////////////////////////////////////////////////
*
* FILE:		server.c
* AUTHOR:	Brenda Nakasone and Catrina Lim
* PROJECT:	CNT 4007 Project 2 - Professor Traynor
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
#include <pthread.h>
#include <dirent.h>

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define BUFSIZE 40		    /* Your name can be as many as 40 chars */

typedef struct {
    char name[1024];
    long long contents; 
    FILE *fileptr;
} file;

typedef struct {
    int clientSock;
    int fileStorageSize;
    file* fileStorage;
} clientThreadArgs;

/* Function to send a list of files to the client */
void listFilesFunc(int fileStorageSize, file* fileStorage, int clientSock) {
    size_t sizeOfFileNames = 0;
    for (int i = 0; i < fileStorageSize; i++) {
        sizeOfFileNames += strlen(fileStorage[i].name);
    }

    char *allFileNames = (char *)malloc((sizeOfFileNames + fileStorageSize) * sizeof(char));
    if (allFileNames == NULL) {
        printf("Memory allocation failed for allFileNames\n");
        exit(1);
    }

    strcpy(allFileNames, fileStorage[0].name);
    for (int i = 1; i < fileStorageSize; i++) {
        strcat(allFileNames, "\n");
        strcat(allFileNames, fileStorage[i].name);
    }

    send(clientSock, allFileNames, strlen(allFileNames), 0);
    printf("Successfully sent server files\n");
    free(allFileNames);
}

long long htonll(long long value) {
    return ((long long)htonl(value & 0xFFFFFFFF) << 32) | htonl(value >> 32);
}

/* Function to handle the menu options for the client */
void handleClientMenu(int clientSock, int fileStorageSize, file* fileStorage) {
    char nameBuf[BUFSIZE];

    while (1) {
        // receiving user response
        int bytesReceived = recv(clientSock, nameBuf, BUFSIZE - 1, 0);
        if (bytesReceived < 0) {
            fprintf(stderr, "Failed to receive data from client\n");
            break;
        }
        nameBuf[bytesReceived] = '\0';
        printf("Client selected: %s\n", nameBuf);
        nameBuf[strcspn(nameBuf, "\n")] = 0;

        // if list files
        if (strcmp(nameBuf, "List Files") == 0) {
            listFilesFunc(fileStorageSize, fileStorage, clientSock);
        }
        //if diff files 
        else if (strcmp(nameBuf, "Diff") == 0) {
            // listFilesFunc(fileStorageSize, fileStorage, clientSock);  // Placeholder for diff functionality

            // sending size of server storage
            memset(nameBuf, 0, BUFSIZE);
            int sendSize = htonl(fileStorageSize);
            send(clientSock, &sendSize, sizeof(sendSize), 0);

            // ack
            memset(nameBuf, 0, BUFSIZE);
            recv(clientSock, nameBuf, BUFSIZE - 1, 0);
            printf("Ok received\n");

            // sending list of file names
            for (int i = 0; i < fileStorageSize; i++){
                // sending length of name
                int len = strlen(fileStorage[i].name);
                send(clientSock, &len, sizeof(len), 0);
                // ack
                memset(nameBuf, 0, BUFSIZE);
                recv(clientSock, nameBuf, BUFSIZE - 1, 0);
                printf("Ok received\n");
                // sending name
                send(clientSock, fileStorage[i].name, strlen(fileStorage[i].name), 0);
                printf("server sending name: %s\n", fileStorage[i].name);
            }

            // ack
            memset(nameBuf, 0, BUFSIZE);
            recv(clientSock, nameBuf, BUFSIZE - 1, 0);
            printf("Ok received\n");

            // sending list of contents
            for (int i = 0; i < fileStorageSize; i++){
                long long sendContent = htonll(fileStorage[i].contents);
                send(clientSock, &sendContent, sizeof(sendContent), 0);
            }

            printf("Successfully received Diff\n");
        } 
        // if pull files
        else if (strcmp(nameBuf, "Pull") == 0) {
            printf("Client requested to pull files.\n");

            for (int i = 0; i < fileStorageSize; i++) {
                send(clientSock, fileStorage[i].name, strlen(fileStorage[i].name), 0);

                FILE *file = fopen(fileStorage[i].name, "rb");
                if (file == NULL) {
                    printf("Error opening file: %s\n", fileStorage[i].name);
                    continue;
                }

                fseek(file, 0, SEEK_END);
                long fileSize = ftell(file);
                rewind(file);

                send(clientSock, &fileSize, sizeof(fileSize), 0);

                char fileBuffer[SNDBUFSIZE];
                int bytesRead;
                while ((bytesRead = fread(fileBuffer, 1, sizeof(fileBuffer), file)) > 0) {
                    send(clientSock, fileBuffer, bytesRead, 0);
                }
                fclose(file);
                printf("File %s sent to client.\n", fileStorage[i].name);
            }
            printf("All files sent to client.\n");
        } 
        // if leave
        else if (strcmp(nameBuf, "Leave") == 0) {
            printf("Client has chosen to leave.\n");
            break;
        } 
        // if invalid input
        else {
            printf("Invalid option. Please retry!\n");
        }
        // clear buf
        memset(nameBuf, 0, BUFSIZE);
    }
    close(clientSock);
}

void* clientThread(void* args) {
    // threading
    clientThreadArgs* clientArgs = (clientThreadArgs*)args;
    int clientSock = clientArgs->clientSock;
    int fileStorageSize = clientArgs->fileStorageSize;
    file* fileStorage = clientArgs->fileStorage;

    handleClientMenu(clientSock, fileStorageSize, fileStorage);

    free(clientArgs);
    pthread_exit(NULL);
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
int main(int argc, char *argv[]) {
    int serverSock, clientSock;
    struct sockaddr_in serv_addr, clnt_addr;
    unsigned short serverPort = 8086;
    unsigned int clntLen;

    char nameBuf[BUFSIZE];

    if (argc < 2) {
        printf("Usage: %s <directory>\n", argv[0]);
        exit(1);
    }
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
        printf("Directory not found. Please try again.");
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

    /* Create a new TCP socket*/
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printf("socket failed\n");
        exit(1);
    }

    /* Construct local address structure*/
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serverPort);

    /* Bind to local address structure */
    if (bind(serverSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("bind failed\n");
        exit(1);
    }

    /* Listen for incoming connections */
    if (listen(serverSock, 10) < 0) {
        printf("listen failed\n");
        exit(1);
    }

    // loop forever
    while (1) {
        clntLen = sizeof(clnt_addr);

        // accept incoming connections
        if ((clientSock = accept(serverSock, (struct sockaddr *)&clnt_addr, &clntLen)) < 0) {
            printf("accept failed\n");
            continue;
        }

        printf("Client connected.\n");

        // threading 
        clientThreadArgs *clientArgs = malloc(sizeof(clientThreadArgs));
        clientArgs->clientSock = clientSock;
        clientArgs->fileStorageSize = fileStorageSize;
        clientArgs->fileStorage = fileStorage;

        pthread_t threadID;
        if (pthread_create(&threadID, NULL, clientThread, (void*)clientArgs) != 0) {
            printf("Failed to create thread\n");
        } else {
            printf("Thread created for client\n");
        }

        pthread_detach(threadID);
    }

    free(fileStorage);
    close(serverSock);

    return 0;
}
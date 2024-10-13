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
#include "file.h"

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define BUFSIZE 40		/* Your name can be as many as 40 chars*/

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

    /* Create a new TCP socket*/
    if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("socket failed\n");
    };

    /* Construct local address structure*/
    memset(&changeServAddr, 0, sizeof(changeServAddr));
    changeServAddr.sin_family = AF_INET;
    changeServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    changeServAddr.sin_port = htons(8084);
    
    /* Bind to local address structure */
    if (bind(serverSock, (struct sockaddr *)&changeServAddr, sizeof(changeServAddr)) < 0){
      printf("bind failed\n");
    };

    /* Listen for incoming connections */
    if(listen(serverSock, 10) < 0){
      printf("listen failed\n");
    };

    /* Loop server forever*/
    while(1)
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

      /* clear buf */
      memset(nameBuf, 0, BUFSIZE);

      // recv(clientSock, nameBuf, BUFSIZE - 1, 0);
      // nameBuf[bytesReceived] = '\0'; 

      /* Run this and return the final value in md_value to client */
      /* Takes the client name and changes it */
      /* Students should NOT touch this code */
      // OpenSSL_add_all_digests();
      // md = EVP_get_digestbyname("SHA256");
      // mdctx = EVP_MD_CTX_create();
      // EVP_DigestInit_ex(mdctx, md, NULL);
      // EVP_DigestUpdate(mdctx, nameBuf, strlen(nameBuf));
      // EVP_DigestFinal_ex(mdctx, md_value, &md_len);
      // EVP_MD_CTX_destroy(mdctx);

      // /* Return md_value to client */
      // send(clientSock, md_value, md_len, 0);

        close(clientSock);
    }
    close(serverSock);

    return 0;
}
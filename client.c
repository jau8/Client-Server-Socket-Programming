/* Included libraries */

#include <stdio.h>       /* for printf() and fprintf() */
#include <sys/socket.h>	 /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>   /* for sockaddr_in and inet_addr() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Constants */
#define RCVBUFSIZE 512	 /* The receive buffer size */
#define SNDBUFSIZE 512	 /* The send buffer size */


/*----------------------------------------------------------------------
 *
 * main --
 *
 * Results:
 *      Returns int 0 after executing
 *
 *----------------------------------------------------------------------
 */

int
main(int argc, char *argv[])
{

   int clientSock;               /* socket descriptor */
   struct sockaddr_in serv_addr; /* server address structure */

   char *accountName;       /* Account Name  */
   char *servIP;            /* Server IP address  */
   unsigned short servPort; /* Server Port number */
   char *command;           /* Command (transfer withdraw or balance) */
   char *amount;            /* Amount to withdraw or transfer */
   char *recipientName;     /* Name of transfer recipient account*/

   char sndBuf[SNDBUFSIZE]; /* Send Buffer */
   char rcvBuf[RCVBUFSIZE]; /* Receive Buffer */

   int balance; /* Temporary balance */

   memset(&sndBuf, 0, SNDBUFSIZE);
   memset(&rcvBuf, 0, RCVBUFSIZE);

   /* Get the addditional parameters from the command line */
   accountName = argv[1];
   servIP = argv[2];
   servPort = atoi(argv[3]);
   command = argv[4];

   if (argc == 5) {
      amount = "0";
      recipientName = "";
   } else if (argc == 6) {
      amount = argv[5];
      recipientName = "";
   } else if (argc == 7) {
      amount = argv[6];
      recipientName = argv[5];
   }

   /* Create a new TCP socket*/
   clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (clientSock < 0) {
      printf("Fail to create socket\n");
      exit(1);
   }

   /* Construct the server address structure */
   memset(&serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = inet_addr(servIP);
   serv_addr.sin_port = htons(servPort);

   /* Establish connection to the server */
   if (connect(clientSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
       0) {
      printf("Connection failed\n");
      exit(1);
   }

   /* Send the string to the server */
   int sendBytes = send(clientSock, accountName, strlen(accountName), 0);
   int recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
   memset(&rcvBuf, 0, RCVBUFSIZE);
   sendBytes = send(clientSock, command, strlen(command), 0);
   recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);

   /* Receive and print response from the server */
   // Withdraw command
   if (argc == 6) {
      memset(&rcvBuf, 0, RCVBUFSIZE);
      sendBytes = send(clientSock, amount, strlen(amount), 0);
      recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
      printf("%s\n", rcvBuf); // Print withdraw message

      /* Ask for the current balance */
      strcpy(sndBuf, "BAL");
      memset(&rcvBuf, 0, RCVBUFSIZE);
      sendBytes = send(clientSock, sndBuf, strlen(sndBuf), 0);
      recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
      printf("%s\n", rcvBuf);
   }

   // Transfer command
   if (argc == 7) {
      memset(&rcvBuf, 0, RCVBUFSIZE);
      sendBytes = send(clientSock, recipientName, strlen(recipientName), 0);
      recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);

      sendBytes = send(clientSock, amount, strlen(amount), 0);
      memset(&rcvBuf, 0, RCVBUFSIZE);
      recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
      printf("%s\n", rcvBuf); /* Print the withdraw message */

      /* Ask for the current balance */
      memset(&rcvBuf, 0, RCVBUFSIZE);
      strcpy(sndBuf, "BAL");
      sendBytes = send(clientSock, sndBuf, strlen(sndBuf), 0);
      recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
      printf("%s\n", rcvBuf);
   }

   balance = atoi(rcvBuf);
   close(clientSock);

   printf("%s\n", accountName);
   printf("Balance is: %i\n", balance);

   return 0;
}

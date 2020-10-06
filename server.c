/*Included libraries*/

#include <stdio.h>	  /* for printf() and fprintf() */
#include <sys/socket.h>	  /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>	  /* supports all sorts of functionality */
#include <unistd.h>	  /* for close() */
#include <string.h>	  /* support any string ops */
#include <time.h>

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define BUFSIZE 40		/* Your name can be as many as 40 chars*/
#define BALANCESIZE 10      /* The balance buffer size */


/*----------------------------------------------------------------------
 *
 * Balances --
 *
 *      Struct containing the int balances that are supported
 *
 *----------------------------------------------------------------------
 */

typedef struct Balances {
   int mySavingsBalance;
   int myCheckingBalance;
   int myCDBalance;
   int my401kBalance;
   int my529Balance;
} Balances;


/*----------------------------------------------------------------------
 *
 * updateTime --
 *
 *      Calculates difference between two parameterized timestamps
 *
 * Results:
 *      Returns a double value of the difference of the timestamps
 *
 *----------------------------------------------------------------------
 */
double
updateTime(time_t *timestamp, time_t time)
{
   if (timestamp[0] == 0) {
      timestamp[0] = time;
      return 100.0;
   } else if (timestamp[1] == 0) {
      timestamp[1] = time;
      return 100.0;
   } else if (timestamp[2] == 0) {
      timestamp[2] = time;
      return 100.0;
   } else {
      time_t temp = timestamp[0];
      timestamp[0] = timestamp[1];
      timestamp[1] = timestamp[2];
      timestamp[2] = time;
      return difftime(time, temp);
   }
}


/*----------------------------------------------------------------------
 *
 * main --
 *
 * Results:
 *      Returns a 1 after executing
 *
 *----------------------------------------------------------------------
 */

int
main(int argc, char *argv[])
{
   struct sockaddr_in changeServAddr; /* Local address */
   struct sockaddr_in changeClntAddr; /* Client address */
   unsigned short changeServPort;     /* Server port */
   unsigned int clntLen;              /* Length of address data struct */

   char nameBuf[BUFSIZE];      /* Buff to store account name from client */
   char recipientBuf[BUFSIZE]; /* Buff to store name from transfer recipient */
   int balance;                /* Place to record account balance result */
   int recipientBalance;

   /* Timestamps for each balance */
   time_t myCheckingTimestamp[3] = {0};
   time_t mySavingsTimestamp[3] = {0};
   time_t myCDTimestamp[3] = {0};
   time_t my401kTimestamp[3] = {0};
   time_t my529Timestamp[3] = {0};

   int serverSock; /* Server Socket */
   int clientSock; /* Client Socket */

   /* Pre-configure each balances */
   Balances balances = {10, 20, 30, 40, 50};

   char rcvBuf[RCVBUFSIZE];      /* Buffer to receive message from client */
   char sndBuf[SNDBUFSIZE];      /* Buffer to send message to client */
   char balanceBuf[BALANCESIZE]; /* Buffer to store the current balance */

   /* Parse arguments from command line */
   if (argc > 3) {
      printf("Too many arguments\n");
      exit(1);
   }
   char *changeServIP = "127.0.0.1"; /* Local Server IP address */
   changeServPort = 3000;
   if (argc == 3) {
      changeServIP = argv[1];
      changeServPort = atoi(argv[2]);
   } else if (argc == 2) {
      changeServPort = atoi(argv[1]);
   }

   /* Create new TCP Socket for incoming requests*/
   serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (serverSock < 0) {
      printf("Fail to create socket\n");
      exit(1);
   }

   /* Construct local address structure*/
   memset(&changeServAddr, 0, sizeof(changeServAddr));
   changeServAddr.sin_family = AF_INET;
   changeServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   changeServAddr.sin_port = htons(changeServPort);

   /* Bind to local address structure */
   if (bind(serverSock, (struct sockaddr *)&changeServAddr,
            sizeof(changeServAddr)) < 0) {
      printf("Bind failed\n");
      exit(1);
   }

   /* Listen for incoming connections */
   if (listen(serverSock, 32) < 0) {
      printf("Listen failed\n");
      exit(1);
   }

   /* Loop server forever*/
   while (1) {
      /* Accept incoming connection */
      int changeClntAddrLen = sizeof(changeClntAddr);
      clientSock = accept(serverSock, (struct sockaddr *)&changeClntAddr,
                          &changeClntAddrLen);
      if (clientSock < 0) {
         printf("Connection failed.\n");
      } else {
         printf("Connection accepted.\n");
      }

      /* Extract the account name from the packet, store in nameBuf */
      memset(&nameBuf, 0, BUFSIZE);
      int recvBytes = recv(clientSock, nameBuf, sizeof(nameBuf), 0);

      /* Look up account balance, store in balance */
      if (strcmp(nameBuf, "myChecking") == 0) {
         balance = balances.myCheckingBalance;
      } else if (strcmp(nameBuf, "mySavings") == 0) {
         balance = balances.mySavingsBalance;
      } else if (strcmp(nameBuf, "myCD") == 0) {
         balance = balances.myCDBalance;
      } else if (strcmp(nameBuf, "my401k") == 0) {
         balance = balances.my401kBalance;
      } else if (strcmp(nameBuf, "my529") == 0) {
         balance = balances.my529Balance;
      }

      memset(&sndBuf, 0, SNDBUFSIZE);
      strcpy(sndBuf, "Balance, Withdraw, or Transfer");
      int sendBytes = send(clientSock, sndBuf, strlen(sndBuf), 0);

      /* Receive the specific operation */
      memset(&rcvBuf, 0, RCVBUFSIZE);
      recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
      if (strcmp(rcvBuf, "WITHDRAW") == 0) {

         /* Ask for the amount */
         memset(&sndBuf, 0, SNDBUFSIZE);
         strcpy(sndBuf, "Please enter the amount");
         sendBytes = send(clientSock, sndBuf, strlen(sndBuf), 0);

         /* Receive the amount from client */
         memset(&rcvBuf, 0, RCVBUFSIZE);
         recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);

         /* Timeout */
         time_t currentTime;
         time(&currentTime);
         double diff = 0.0;
         if (strcmp(nameBuf, "myChecking") == 0) {
            diff = updateTime(myCheckingTimestamp, currentTime);
         } else if (strcmp(nameBuf, "mySavings") == 0) {
            diff = updateTime(mySavingsTimestamp, currentTime);
         } else if (strcmp(nameBuf, "myCD") == 0) {
            diff = updateTime(myCDTimestamp, currentTime);
         } else if (strcmp(nameBuf, "my401k") == 0) {
            diff = updateTime(my401kTimestamp, currentTime);
         } else if (strcmp(nameBuf, "my529") == 0) {
            diff = updateTime(my529Timestamp, currentTime);
         }

         /* Send the withdraw message to client */
         memset(&sndBuf, 0, SNDBUFSIZE);
         int amount = atoi(rcvBuf);
         if (diff <= 60.0) {
            strcpy(sndBuf, "Error: Account Timed Out!");
         } else if (amount <= balance) {
            balance -= amount;
            strcpy(sndBuf, "Withdraw Succeeded!");
         } else {
            strcpy(sndBuf, "Error: Insufficient Funds!");
         }
         sendBytes = send(clientSock, sndBuf, strlen(sndBuf), 0);

         /* Update the balance of the specific account */
         if (strcmp(nameBuf, "myChecking") == 0) {
            balances.myCheckingBalance = balance;
         } else if (strcmp(nameBuf, "mySavings") == 0) {
            balances.mySavingsBalance = balance;
         } else if (strcmp(nameBuf, "myCD") == 0) {
            balances.myCDBalance = balance;
         } else if (strcmp(nameBuf, "my401k") == 0) {
            balances.my401kBalance = balance;
         } else if (strcmp(nameBuf, "my529") == 0) {
            balances.my529Balance = balance;
         }

         /* Receive the next command from client */
         memset(&rcvBuf, 0, RCVBUFSIZE);
         recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
      }

      if (strcmp(rcvBuf, "TRANSFER") == 0) {
         /* Ask for name of recipient account */
         strcpy(sndBuf, "Name of recipient account");
         sendBytes = send(clientSock, sndBuf, strlen(sndBuf), 0);

         /* Get name of recipient account */
         memset(&recipientBuf, 0, BUFSIZE);
         recvBytes = recv(clientSock, recipientBuf, sizeof(recipientBuf), 0);

         /* Look up account balance, store in balance */
         if (strcmp(recipientBuf, "myChecking") == 0) {
            recipientBalance = balances.myCheckingBalance;
         } else if (strcmp(recipientBuf, "mySavings") == 0) {
            recipientBalance = balances.mySavingsBalance;
         } else if (strcmp(recipientBuf, "myCD") == 0) {
            recipientBalance = balances.myCDBalance;
         } else if (strcmp(recipientBuf, "my401k") == 0) {
            recipientBalance = balances.my401kBalance;
         } else if (strcmp(recipientBuf, "my529") == 0) {
            recipientBalance = balances.my529Balance;
         }

         /* Ask for the amount */
         memset(&sndBuf, 0, SNDBUFSIZE);
         strcpy(sndBuf, "Please enter the amount");
         sendBytes = send(clientSock, sndBuf, strlen(sndBuf), 0);

         /* Receive the amount from client */
         memset(&rcvBuf, 0, RCVBUFSIZE);
         recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);

         /* Send the transfer message to client */
         memset(&sndBuf, 0, SNDBUFSIZE);
         int amount = atoi(rcvBuf);
         if (amount <= balance) {
            balance -= amount;
            recipientBalance += amount;
            strcpy(sndBuf, "Transfer Succeeded!");
         } else {
            strcpy(sndBuf, "Error: Insufficient Funds!");
         }
         sendBytes = send(clientSock, sndBuf, strlen(sndBuf), 0);

         /* Update the balance of the sender account */
         if (strcmp(nameBuf, "myChecking") == 0) {
            balances.myCheckingBalance = balance;
         } else if (strcmp(nameBuf, "mySavings") == 0) {
            balances.mySavingsBalance = balance;
         } else if (strcmp(nameBuf, "myCD") == 0) {
            balances.myCDBalance = balance;
         } else if (strcmp(nameBuf, "my401k") == 0) {
            balances.my401kBalance = balance;
         } else if (strcmp(nameBuf, "my529") == 0) {
            balances.my529Balance = balance;
         }

         /* Update the balance of the recipient account */
         if (strcmp(recipientBuf, "myChecking") == 0) {
            balances.myCheckingBalance = recipientBalance;
         } else if (strcmp(recipientBuf, "mySavings") == 0) {
            balances.mySavingsBalance = recipientBalance;
         } else if (strcmp(recipientBuf, "myCD") == 0) {
            balances.myCDBalance = recipientBalance;
         } else if (strcmp(recipientBuf, "my401k") == 0) {
            balances.my401kBalance = recipientBalance;
         } else if (strcmp(recipientBuf, "my529") == 0) {
            balances.my529Balance = recipientBalance;
         }

         /* Receive the next command from client */
         memset(&rcvBuf, 0, RCVBUFSIZE);
         recvBytes = recv(clientSock, rcvBuf, RCVBUFSIZE, 0);
      }

      if (strcmp(rcvBuf, "BAL") == 0) {
         memset(&balanceBuf, 0, BALANCESIZE);
         sprintf(balanceBuf, "%d", balance);
         sendBytes = send(clientSock, balanceBuf, strlen(balanceBuf), 0);
      }

      close(clientSock);
   }

   return 0;
}

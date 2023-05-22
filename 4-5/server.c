#include "stdio.h"
#include "fcntl.h"
#include "time.h"
#include <sys/stat.h>
#include "stdlib.h"
#include "semaphore.h"
#include "signal.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "string.h"
#include "netinet/in.h"
#include "arpa/inet.h"
#include "unistd.h"

#define BUF_SIZE 1000
#define CLADDR_LEN 100
#define CNT "count.txt"
#define F1 "field1.txt"
#define F2 "field2.txt"

int port, n, count, count1, count2;

#define SEM "/my_semaphore"

sem_t *mutex;

void my_handler(int nsig)
{
   sem_unlink(SEM);
   exit(0);
}

void main(int argc, char *argv[])
{
   (void)signal(SIGINT, my_handler);
   if (argc != 4)
   {
      printf("usage: server <port> <size of field> <number of weapons>\n");
      exit(1);
   }
   port = atoi(argv[1]);
   n = atoi(argv[2]);
   count = atoi(argv[3]);
   if (port < 5000 || port > 52000)
   {
      printf("port must be in range [5000, 52000]");
      exit(1);
   }

   if (n < 3 || n > 8)
   {
      printf("size of field must be in range [3, 8]");
      exit(1);
   }

   if (count < 3 || count > 8)
   {
      printf("number of weapons must be in range [3, 8]");
      exit(1);
   }

   srand(time(NULL));

   count1 = count;
   count2 = count;
   FILE *countFile;
   countFile = fopen(CNT, "w");
   fprintf(countFile, "%d %d", count1, count2);
   fclose(countFile);

   FILE *file1 = fopen(F1, "w");
   FILE *file2 = fopen(F2, "w");
   int field1[n * n];
   int field2[n * n];
   for (int i = 0; i < n * n; ++i)
   {
      field1[i] = 1;
      field2[i] = 1;
      fprintf(file1, "%d ", field1[i]);
      fprintf(file2, "%d ", field2[i]);
   }
   fclose(file1);
   fclose(file2);

   int weapons1[count];
   int weapons2[count];
   for (int i = 0; i < count; ++i)
   {
      while (1 > 0)
      {
         int x = rand() % (n * n);
         if (field1[x] == 2)
         {
            continue;
         }
         field1[x] = 2;
         weapons1[i] = x;
         break;
      }
      while (1 > 0)
      {
         int x = rand() % (n * n);
         if (field2[x] == 2)
         {
            continue;
         }
         field2[x] = 2;
         weapons2[i] = x;
         break;
      }
   }

   struct sockaddr_in addr, cl_addr;
   int sockfd, len, ret, newsockfd;
   char buffer[BUF_SIZE];
   char buffer2[BUF_SIZE];
   pid_t childpid;
   char clientAddr[CLADDR_LEN];

   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0)
   {
      printf("Error creating socket!\n");
      exit(1);
   }
   printf("Socket created...\n");

   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
   addr.sin_port = port;

   ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
   if (ret < 0)
   {
      printf("Error binding!\n");
      exit(1);
   }

   printf("Waiting for a connection...\n");
   listen(sockfd, 5);

   for (;;)
   { // infinite loop
      len = sizeof(cl_addr);
      newsockfd = accept(sockfd, (struct sockaddr *)&cl_addr, &len);
      if (newsockfd < 0)
      {
         printf("Error accepting connection!\n");
         exit(1);
      }
      printf("Connection accepted...\n");

      inet_ntop(AF_INET, &(cl_addr.sin_addr), clientAddr, CLADDR_LEN);
      if ((childpid = fork()) == 0)
      { // creating a child process

         close(sockfd);
         // stop listening for new connections by the main process.
         // the child will continue to listen.
         // the main process now handles the connected client.

         mutex = sem_open(SEM, O_CREAT, S_IRUSR | S_IWUSR, 1);

         for (;;)
         {
            memset(buffer, 0, BUF_SIZE);
            memset(buffer2, 0, BUF_SIZE);
            ret = recvfrom(newsockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&cl_addr, &len);
            sem_wait(mutex);
            if (ret < 0)
            {
               printf("Error receiving data!\n");
               exit(1);
            }

            countFile = fopen(CNT, "r");
            file1 = fopen(F1, "r");
            file2 = fopen(F2, "r");
            fscanf(countFile, "%d %d", &count1, &count2);
            for (int i = 0; i < n * n; ++i)
            {
               fscanf(file1, "%d ", &field1[i]);
            }
            for (int i = 0; i < n * n; ++i)
            {
               fscanf(file2, "%d ", &field2[i]);
            }
            fclose(countFile);
            fclose(file1);
            fclose(file2);

            if (buffer[0] == '0')
            {
               buffer2[0] = (char)('0' + count1);
               buffer2[1] = (char)('0' + count2);
            }
            else if (buffer[0] == '1')
            {
               int weapon = (int)(buffer[1] - '0');
               int pos = (int)((buffer[2] - '0') * 10 + buffer[3] - '0');
               if (field1[weapons1[weapon]] == 0)
               {
                  buffer2[0] = '9';
               }
               else
               {
                  field2[pos] = 0;
                  for (int i = 0; i < count; ++i)
                  {
                     if (weapons2[i] == pos)
                     {
                        printf("country 2 lost one of their weapons\n");
                        --count2;
                     }
                  }
                  buffer2[0] = (char)(count2 + '0');
               }
            }
            else if (buffer[0] == '2')
            {
               int weapon = (int)(buffer[1] - '0');
               int pos = (int)((buffer[2] - '0') * 10 + buffer[3] - '0');
               if (field2[weapons2[weapon]] == 0)
               {
                  buffer2[0] = '9';
               }
               else
               {
                  field1[pos] = 0;
                  for (int i = 0; i < count; ++i)
                  {
                     if (weapons1[i] == pos)
                     {
                        printf("country 1 lost one of their weapons\n");
                        --count1;
                     }
                  }
                  buffer2[0] = (char)(count1 + '0');
               }
            }

            ret = sendto(newsockfd, buffer2, BUF_SIZE, 0, (struct sockaddr *)&cl_addr, len);

            countFile = fopen(CNT, "w");
            file1 = fopen(F1, "w");
            file2 = fopen(F2, "w");
            fprintf(countFile, "%d %d", count1, count2);
            for (int i = 0; i < n * n; ++i)
            {
               fprintf(file1, "%d ", field1[i]);
            }
            for (int i = 0; i < n * n; ++i)
            {
               fprintf(file2, "%d ", field2[i]);
            }
            fclose(countFile);
            fclose(file1);
            fclose(file2);

            sem_post(mutex);
            if (ret < 0)
            {
               printf("Error sending data!\n");
               exit(1);
            }
         }
         sleep(3);
         sem_close(mutex);
      }
      close(newsockfd);
   }
   
   my_handler(2);
}
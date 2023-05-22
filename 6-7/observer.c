#include "stdio.h"
#include "stdlib.h"
#include "semaphore.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "string.h"
#include "netinet/in.h"
#include "netdb.h"
#include "pthread.h"
#include "unistd.h"
#include <arpa/inet.h>

#define BUF_SIZE 1000

struct sockaddr_in addr, cl_addr;
int sockfd, ret, port, count1, count2;
char buffer[BUF_SIZE];
struct hostent *server;
char *serverAddr;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("usage: client <ip address> <port>\n");
        exit(1);
    }

    serverAddr = argv[1];
    port = atoi(argv[2]);
    if (port < 5000 || port > 52000)
    {
        printf("port must be in range [5000, 52000]\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Error creating socket!\n");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(serverAddr);
    addr.sin_port = port;

    ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        printf("Error connecting to the server!\n");
        exit(1);
    }
    printf("Observer connected to the server...\n");

    while (1)
    {
        memset(buffer, 0, BUF_SIZE);
        buffer[0] = '0';
        ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
        memset(buffer, 0, BUF_SIZE);
        ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (ret < 0)
        {
            printf("Error receiving data!\n");
        }
        count1 = (int)(buffer[0] - '0');
        count2 = (int)(buffer[1] - '0');
        if (count1 == 0) {
            printf("The second country won the war\n");
            break;
        }
        if (count2 == 0) {
            printf("The first country won the war\n");
            break;
        }
        printf("The first country have now %d weapons, and the second have %d\n", count1, count2);
        usleep(300000);
    }
    sleep(3);

    return 0;
}
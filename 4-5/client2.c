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
int sockfd, ret, port, n, count, count1, count2;
char buffer[BUF_SIZE];
struct hostent *server;
char *serverAddr;
int field1[100];
sem_t mutex;

void *weapon(void *number)
{
    int d = *(int *)number;
    usleep(rand() % 2000000);
    while (1)
    {
        sem_wait(&mutex);
        memset(buffer, 0, BUF_SIZE);
        buffer[0] = '0';
        ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
        if (ret < 0)
        {
            printf("Error sending data!\n\t-%s", buffer);
        }
        memset(buffer, 0, BUF_SIZE);
        ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (ret < 0)
        {
            printf("Error receiving data!\n");
        }
        else
        {
            count1 = (int)(buffer[0] - '0');
            count2 = (int)(buffer[1] - '0');
            if (count1 == 0 || count2 == 0) {
                sem_post(&mutex);
                break;
            }
        }
        memset(buffer, 0, BUF_SIZE);
        buffer[0] = '2';
        buffer[1] = (char)(d + '0');
        int x;
        while (1)
        {
            x = rand() % (n * n);
            if (field1[x] == 0)
            {
                continue;
            }
            buffer[2] = (char)(x / 10 + '0');
            buffer[3] = (char)(x % 10 + '0');
            break;
        }
        ret = sendto(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
        if (ret < 0)
        {
            printf("Error sending data!\n\t-%s", buffer);
        }
        memset(buffer, 0, BUF_SIZE);
        ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
        if (ret < 0)
        {
            printf("Error receiving data!\n");
        }
        if (buffer[0] == '9')
        {
            sem_post(&mutex);
            break;
        } else if (count1 > (int)(buffer[0] - '0')) {
            printf("we killed someone\n");
            if ((int)(buffer[0] - '0') == 0) {
                sem_post(&mutex);
                break;
            }
        }
        sem_post(&mutex);
        usleep((1 + x / 10) * 1000000 + rand() % 1000000);
    }
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        printf("usage: client <ip address> <port> <size of field> <number of weapons>\n");
        exit(1);
    }

    serverAddr = argv[1];
    port = atoi(argv[2]);
    if (port < 5000 || port > 52000)
    {
        printf("port must be in range [5000, 52000]\n");
        exit(1);
    }
    n = atoi(argv[3]);
    if (n < 3 || n > 8)
    {
        printf("size of field must be in range [3, 8]\n");
        exit(1);
    }
    count = atoi(argv[4]);
    if (count < 3 || count > 8)
    {
        printf("number of weapons must be in range [3, 8]\n");
        exit(1);
    }

    count1 = count;
    count2 = count;

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
    printf("Country 2 connected to the server...\n");

    for (int i = 0; i < n * n; ++i)
    {
        field1[i] = 1;
    }

    sem_init(&mutex, 0, 1);
    int numbers[count];
    for (int i = 0; i < count; ++i) {
        numbers[i] = i;
    }

    pthread_t base[count];
    for (int i = 0; i < count; i++)
    {
        pthread_create(&base[i], 0, weapon, &numbers[i]);
    }
    for (int i = 0; i < count; ++i)
    {
        pthread_join(base[i], NULL);
    }

    sem_destroy(&mutex);

    if (count2 == 0) {
        printf("we lose :(\n");
    } else if (count1 == 0) {
        printf("we won!\n");
    }
    sleep(3);

    return 0;
}
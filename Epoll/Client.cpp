#include <iostream>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("The input format must be %s [SERVER-IP] [SERVER-PORT]\n", argv[0]);
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in ser_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(argv[1]);
    ser_addr.sin_port = htons(atoi(argv[2]));

    while (1)
    {
        int new_ = connect(sock, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
        if (new_ < 0)
        {
            perror("connect");
            close(sock);
            exit(2);
        }
        while (1)
        {
            printf("Client Say # ");
            fflush(stdout);
            char sendbuf[1024];
            memset(sendbuf, 0, sizeof(sendbuf));
            ssize_t s = read(0, sendbuf, sizeof(sendbuf) - 1);
            if (s > 0)
            {
                sendbuf[s - 1] = 0;
                //printf("%s\n", buf);
            }
            write(sock, sendbuf, strlen(sendbuf));

            char recbuf[1024];
            memset(recbuf, 0, sizeof(recbuf));
            ssize_t _s = read(sock, recbuf, sizeof(recbuf) - 1);
            if (_s > 0)
            {
                if (_s == 4 && strcmp(recbuf, "bye") == 0)
                {
                    printf("Max connection arrive, force exit!\n");
                    break;
                }
                printf("Server Say # %s\n", recbuf);
            }
        }
    }
    close(sock);
    return 0;
}
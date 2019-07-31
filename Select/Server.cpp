#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

const int backlog = 3;
int fdset[backlog] = {};
int conn_amount = 0;
/*
readme:
    exit:
        -1:The input format is error
*/

void showclient(void)
{
    printf("\nClient information\n");
    printf("Client amount: %d\n", conn_amount);
    for (int i = 0; i < backlog; i++)
    {
        printf("[%d]: %d\n", i, fdset[i]);
    }
    printf("End\n\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("The input format must be : %s [SERVER-IP] [SERVER-PORT]\n", argv[0]);
        exit(-1);
    }
    
    //1.socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }
    //reconnectable for accidental disconnection
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(2);
    }
    //2.bind
    struct sockaddr_in ser_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(argv[1]);
    ser_addr.sin_port = htons(atoi(argv[2]));
    if (bind(sock, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) < 0)
    {
        perror("bind");
        exit(3);
    }
    //3.listen
    if (listen(sock, 10) < 0)
    {
        perror("listen");
        exit(4);
    }

    fd_set fd;
    int max_fd = sock;
    struct timeval tv;

    struct sockaddr_in cli_addr;
    memset(&cli_addr, 0, sizeof(cli_addr));
    socklen_t len = sizeof(cli_addr);

    char buf[1024];
    memset(buf, 0, sizeof(buf));

    while (1)
    {
        //fd set
        FD_ZERO(&fd);
        FD_SET(sock, &fd);
        //time set
        tv.tv_sec = 30;
        tv.tv_usec = 0;
        
        //add active connection to fdset
        for (int i = 0; i < backlog; i++)
        {
            if (fdset[i] != 0)
            {
                FD_SET(fdset[i], &fd);
            }
        }

        //select
        int res = select(max_fd + 1, &fd, NULL, NULL, &tv);
        if (res < 0)
        {
            perror("select");
            exit(5);
        }
        else if (res == 0)
        {
            printf("Time is over\n");
            continue;
        }
        //if normal
        for (int i = 0; i < conn_amount; i++)
        {
            if (FD_ISSET(fdset[i], &fd))
            {
                res = recv(fdset[i], buf, sizeof(buf) - 1, 0);
                if (res > 0)
                {
                    buf[res] = 0;//-1
                    printf("Client %d Say # %s\n", i, buf);
                }
                else
                {
                    printf("Client %d Quit!\n", i);
                    close(fdset[i]);
                    FD_CLR(fdset[i], &fd);
                    fdset[i] = 0;
                }
                //
                char str[] = "Good, very nice!";
                send(fdset[i], str, sizeof(str), 0);
            }
        }

        //cheak whether a new connection comes
        if (FD_ISSET(sock, &fd))
        {
            int new_sock = accept(sock, (struct sockaddr *)&cli_addr, &len);
            if (new_sock < 0)
            {
                perror("accept");
                exit(6);
            }

            //add to fd queue
            if (conn_amount < backlog)
            {
                fdset[conn_amount++] = new_sock;
                printf("Client %s %d is connect\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                if (new_sock > max_fd)
                {
                    max_fd = new_sock;
                }
            }
            else
            {
                printf("Max connection arrive, exit!\n");
                char msg[] = "bye";
                send(new_sock, msg, sizeof(msg), 0);
                close(new_sock);
                //break;
            }
            
        }
        showclient();
    }
    
    for (int i = 0; i < backlog; i++)
    {
        if (fdset[i] != 0)
        {
            close(fdset[i]);
        }
    }
    return 0;
}
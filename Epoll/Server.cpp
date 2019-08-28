#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
using namespace std;

const int MAXQUEUE = 10;
const int MAXFD = 1024;
const int EPOLLEVENTS = 1000;

int setsock(const char *IP, int PORT)
{
    //socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    //setsockopt
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(2);
    }

    //bind
    struct sockaddr_in ser_addr;
    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(IP);
    ser_addr.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) < 0)
    {
        perror("bind");
        exit(3);
    }

    //listen
    if (listen(sock, MAXQUEUE) < 0)
    {
        perror("listen");
        exit(4);
    }
    return sock;
}

void do_epoll(int lis_sock)
{
    //epoll_create
    int epollfd = epoll_create(MAXFD);
    if (epollfd < 0)
    {
        perror("epoll_create");
        exit(5);
    }
    

    //epoll_ctl
    struct epoll_event ev;
    ev.data.fd = lis_sock;
    ev.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, lis_sock, &ev) < 0)
    {
        perror("epoll_ctl");
        exit(6);
    }

    //parameter
    int waitnum = 0;
    struct epoll_event evs[EPOLLEVENTS];
    while (1)
    {
        waitnum = epoll_wait(epollfd, evs, EPOLLEVENTS, -1);
        if (waitnum == 0)
        {
            printf("timeover\n");
            continue;
        }
        else
        {
            int fd = 0;
            for (int i = 0; i < waitnum; i++)
            {
                fd = evs[i].data.fd;
                if ((fd == lis_sock) && (evs[i].events & EPOLLIN))
                {
                    struct sockaddr_in cli_addr;
                    memset(&cli_addr, 0, sizeof(cli_addr));
                    socklen_t len = sizeof(cli_addr);
                    int newsock = accept(lis_sock, (struct sockaddr*)&cli_addr, &len);
                    if (newsock < 0)
                    {
                        perror("accept");
                        exit(6);
                    }
                    else
                    {
                        printf("Client %s %d is connect!\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                        ev.data.fd = newsock;
                        ev.events = EPOLLIN;
                        epoll_ctl(epollfd, EPOLL_CTL_ADD, newsock, &ev);
                    }
                }//accept
                else if (evs[i].events & EPOLLIN)
                {
                    char buf[1024] = {};
                    int s = read(fd, buf, sizeof(buf) - 1);
                    if (s < 0)
                    {
                        perror("read");
                        continue;
                    }
                    else if (s == 0)
                    {
                        printf("Client close\n");
                        close(fd);
                        ev.data.fd = fd;
                        ev.events = EPOLLIN;
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
                    }
                    else
                    {
                        buf[s] = 0;
                        printf("Client say # %s\n", buf);
                        ev.data.fd = fd;
                        ev.events = EPOLLOUT;
                        epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
                    }
                    
                }//read
                else if (evs[i].events & EPOLLOUT)
                {
                    char buf[1024] = "receve msg";
                    int s = write(fd, buf, strlen(buf));
                    if (s < 0)
                    {
                        perror("write");
                        close(fd);
                        ev.data.fd = fd;
                        ev.events = EPOLLIN;
                        epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
                    }
                    else
                    {
                        ev.data.fd = fd;
                        ev.events = EPOLLIN;
                        epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
                    }
                    memset(buf, 0, sizeof(buf));
                }//write
            }
        }
        
    }
}


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("The input format must be: %s [SERVER-IP] [SERVER-POER]\n", argv[0]);
        return -1;
    }

    int lis_sock = setsock(argv[1], atoi(argv[2]));
    do_epoll(lis_sock);
    return 0;
}
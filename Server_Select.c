#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

#define IPADDR "127.0.0.1"
#define PORT 8787
#define MAXLINE 1024
#define LISTENQ 5
#define SIZE 10

typedef struct server_context_st
{
    int cli_cnt;       //number of client
    int clifds[SIZE];  //number of client
    fd_set allfda;     //handle set
    int macfd;         //max handle
}server_context_st;
static server_context_st *s_srv_ctx = NULL;

static int create_server_proc(const char *ip, int port)
{
    int fd;
    struct sockaddr_in servaddr;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        fprintf(stderr, "create socket fail, erron: %d, reason : %s\n", errno, strerror(errno));
        return -1;
    }

    /*
    * After a port is released, it will wait for two minutes
    * before it can be used again. SO_REUSEADDR allows the port
    * to be used again immediately after it is released. 
    */
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        return -1;
    }
    bzero (&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind error: ");
        return -1;
    }
    listen(fd, LISTENQ);
    return fd;
}



int main(int argc, char *argv[])
{
    int srvfd;
    /*initial the server context*/
    if (server_init() < 0)
    {
        return -1;
    }
    /*create a server and start listening for client requests*/
    srvfd = create_server_proc(IPADDR, PORT);
    if (srvfd < 0)
    {
        fprintf(stderr, "socket create or bind fail.\n");
        goto err;
    }
    /*start receiving and  dealing client requests*/
    handle_client_proc(srvfd);
    server_uninit();
    return 0;

    err:
    server_uninit();
    return -1;
}
#include "routeBase.inc"

#define BUFSIZE 512


void SetNonBlock(int fd)
{
    int flag = fcntl ( fd, F_GETFL, 0 );
    fcntl ( fd, F_SETFL, flag | O_NONBLOCK );
}

char* gotoClient(char *ip,int port,char *msg)
{
    int iRet = 0;
    static char buffer[BUFSIZE] = {0};

//    if(4 != argc)
//    {
//    	perror("Parameter: ServerIP Message ServerPort -1");
//        return -1;
//    }

//    int i16_port = atoi(port);
    int i16_port = port;
    if(0 >= i16_port)
    {
    	printf("port wrong:%d\n",i16_port);
        return "-1";
    }

    int sk = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(-1 == sk)
    {
    	perror("open socket failed!");
        return "-1";
    }


    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(i16_port);

    struct sockaddr_in *psa = &sa;

    iRet = inet_pton(AF_INET, ip, &psa->sin_addr.s_addr);
    if(0 == iRet)
    {
    	perror("inet_pton failed, invalid address! -1 ");
        close(sk);
        return "-1";
    }
    else if(iRet < 0)
    {
    	perror("inet_pton failed");
        close(sk);
        return "-1";
    }

    if(connect(sk, (struct sockaddr*)&sa, sizeof(sa)) < 0)
    {
    	perror("connect failed");
        close(sk);
        return "-1";
    }

    SetNonBlock(sk);

    int efd;
    efd = epoll_create(10);
    if(efd == -1)
    {
        perror("epoll_create error!");
        exit(1);
    }

    struct epoll_event event;
    struct epoll_event events[10];

    event.events = EPOLLOUT | EPOLLIN | EPOLLET;
    event.data.fd = sk;

    epoll_ctl(efd, EPOLL_CTL_ADD, sk, &event);


    getchar();
    int loop = 0;
    while(1)
    {
        ssize_t numBytesRcvd = 0;
        int n = 0;
        int i = 0;

        if(loop == 1)
        {
            break;
        }

        n = epoll_wait(efd, events, 10, -1);

        printf("epoll_wait:%d\n", n);

        for(i = 0; i < n; i++)
        {
            if(events[i].events & EPOLLOUT)
            {
                printf("EPOLLOUT...............\n");
                snprintf(buffer, BUFSIZE, "%s",msg);

                int n = strlen(buffer);
                int nsend = 0;

                while(n > 0)
                {
                    //nsend = send(events[i].data.fd, buffer + nsend, n, 0);
                    nsend = write(events[i].data.fd, buffer + nsend, n);
                    if(nsend < 0 && errno != EAGAIN)
                    {

                    	perror("send failed");
                        close(events[i].data.fd);
                        return "-1";
                    }
                    n -= nsend;
                }
            }

            if(events[i].events & EPOLLIN)
            {
                printf("EPOLLIN...............\n");
                memset(buffer, 0, BUFSIZE);

                int len = strlen(buffer);
                int n = 0;
                int nrecv = 0;

                //while((nrecv = recv(events[i].data.fd, buffer + n, BUFSIZE - 1, 0)) > 0)
                while(1){
                    nrecv = read(events[i].data.fd, buffer + n, BUFSIZE - 1) ;
                    if(nrecv == -1 && errno != EAGAIN)
                    {
                        perror("read error!");
                    }
                    if((nrecv == -1 && errno == EAGAIN) || nrecv == 0)
                    {
                        break;
                    }
                    n += nrecv;
                }
                loop = 1;
                printf("buffer:%s\n", buffer);
            }
        }
    }
    close(sk);
    close(efd);
    return buffer;
}

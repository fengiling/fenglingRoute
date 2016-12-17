/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fenglingEpoll.c
 * Author: chenzifeng
 *
 * Created on 2016年11月16日, 下午9:25
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/un.h>  
#include <arpa/inet.h>  
#include <signal.h> 

#include "include/fenglingEpoll.h"
#include "include/fenglingHTML.h"
#include "include/fenglingShm.h"
#include "include/fenglingCA.h"
#include "include/fenglingHTML.h"
#include "include/fenglingLog.h"
#include "include/fenglingConf.h"

#define MAXEVENTS 64

extern conf_global_t g_global_conf;
extern conf_filter_t g_filter_conf;
extern log_t *g_log;

int sfd, s;
int efd;
struct epoll_event event;
struct epoll_event *events;
//client_server *clientServer;
//函数:
//功能:创建和绑定一个TCP socket
//参数:端口
//返回值:创建的socket

static int create_and_bind() {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC; /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE; /* All interfaces */

    s = getaddrinfo(NULL, g_global_conf.S0port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            /* We managed to bind successfully! */
            break;
        }

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not bind[%s]\n",g_global_conf.S0port);
        return -1;
    }

    freeaddrinfo(result);

    return sfd;
}

//函数
//功能:设置socket为非阻塞的

static int make_socket_non_blocking(int sfd) {
    int flags, s;

    //得到文件状态标志
    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    //设置文件状态标志
    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

/*
 * 
 */
int EpollServerInit() {

    time_t sync_t, now_t;
    char *retMsg;

    time(&sync_t);

    //	if (argc != 2) {
    //		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
    //		exit(EXIT_FAILURE);
    //	}

    sfd = create_and_bind();
    if (sfd == -1)
        abort();

    s = make_socket_non_blocking(sfd);
    if (s == -1)
        abort();

    s = listen(sfd, SOMAXCONN);
    if (s == -1) {
        perror("listen");
        abort();
    }

    //除了参数size被忽略外,此函数和epoll_create完全相同
    efd = epoll_create1(0);
    if (efd == -1) {
        perror("epoll_create");
        abort();
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET; //读入,边缘触发方式
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1) {
        perror("epoll_ctl");
        abort();
    }

    /* Buffer where events are returned */
    events = calloc(MAXEVENTS, sizeof event);

    return (EXIT_SUCCESS);
}

int EpollServerRun() {
    long currentRec; /* 进程在进程池的位置 */
    long childPid; /* 子进程进程号       */
    time_t sync_t, now_t;
    char *retMsg;

    debug(g_log, "in EpollServerRun\n");
    
    for (;;) {
        if ((currentRec = pool_monitor()) < 0) {
            debug(g_log, "pool_monitor < 0\n");
            continue;
        }
        debug(g_log, "currentRec:[%d]\n", currentRec);

        switch ((childPid = fork())) {
            case -1:
                sleep(1);
                continue;
            case 0:

                /* 设置子进程退出的处理函数 */
                signal(SIGTERM, CLD_OUT);
                /* zwm, 20060810, 屏蔽子进程的跟踪*/
                signal(SIGCHLD, SIG_IGN);
                /* The event loop */
                while (1) {
                    int n, i;
                    time(&now_t);
                    //                    //每小时同步一次配置
                    //                    if (difftime(now_t, sync_t) > (60 * 60)) {
                    //                        time(&sync_t);
                    //                        //读配置文件
                    //                        ReadConfig(configFile);
                    //                    }

                    n = epoll_wait(efd, events, MAXEVENTS, -1);
                    for (i = 0; i < n; i++) {
                        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
                                || (!(events[i].events & EPOLLIN))) {
                            /* An error has occured on this fd, or the socket is not
                             ready for reading (why were we notified then?) */
                            debug(g_log, "epoll error\n");
                            close(events[i].data.fd);
                            continue;
                        } else if (sfd == events[i].data.fd) {
                            /* We have a notification on the listening socket, which
                             means one or more incoming connections. */
                            while (1) {
                                struct sockaddr in_addr;
                                socklen_t in_len;
                                int infd;
                                char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                                in_len = sizeof in_addr;
                                infd = accept(sfd, &in_addr, &in_len);
                                if (infd == -1) {
                                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                                        /* We have processed all incoming
                                         connections. */
                                        break;
                                    } else {
                                        perror("accept");
                                        break;
                                    }
                                }

                                //将地址转化为主机名或者服务名
                                s = getnameinfo(&in_addr, in_len, hbuf, sizeof hbuf, sbuf,
                                        sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV); //flag参数:以数字名返回
                                //主机地址和服务地址

                                if (s == 0) {
                                    debug(g_log, "Accepted connection on descriptor %d "
                                            "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                                }

                                /* Make the incoming socket non-blocking and add it to the
                                 list of fds to monitor. */
                                s = make_socket_non_blocking(infd);
                                if (s == -1)
                                    abort();

                                event.data.fd = infd;
                                event.events = EPOLLIN | EPOLLET;
                                s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                                if (s == -1) {
                                    perror("epoll_ctl");
                                    abort();
                                }
                            }
                            continue;
                        } else {
                            /* We have data on the fd waiting to be read. Read and
                             display it. We must read whatever data is available
                             completely, as we are running in edge-triggered mode
                             and won't get a notification again for the same
                             data. */
                            int done = 0;

                            while (1) {
                                ssize_t count;
                                char buf[51200];
                                char * Subdomain;
                                char toCbuf[PKG_BODY_LEN + 1];

                                char g_pkgBody[PKG_HEAD_LEN + 1]; /* 放置交易包头 */
                                int g_pkgBodyLen; /* 交易包头长度 */

                                SSL *cfd;

                                count = read(events[i].data.fd, buf, sizeof (buf));
                                if (count == -1) {
                                    /* If errno == EAGAIN, that means we have read all
                                     data. So go back to the main loop. */
                                    if (errno != EAGAIN) {
                                        perror("read");
                                        done = 1;
                                    }
                                    break;
                                } else if (count == 0) {
                                    /* End of file. The remote has closed the
                                     connection. */
                                    done = 1;
                                    break;
                                }

                                /* Write the buffer to standard output */
                                //					s = write(1, buf, count);
                                //					if (s == -1) {
                                //						perror("write");
                                //						abort();
                                //					}
                                // 修改配置文件  转发
                                //                                clientServer = routeAllot();
                                //                                printf("clientServer ip:%s\n", clientServer->ip);
                                //                                printf("clientServer port:%d\n", clientServer->port);
                                //                                printf("clientServer msg:%s\n", buf);
                                //                                retMsg = NULL;
                                //                                retMsg = gotoClient(clientServer->ip, clientServer->port,
                                //                                        buf);
                                // 解析报文 得到subdomain
                                Subdomain = parseHtmlHead(buf);
                                cfd = GetFLRoute(Subdomain);
                                memset(toCbuf, 0x00, sizeof (toCbuf));
                                sprintf(toCbuf, "FL%08d", (int) strlen(buf));
                                SSL_write(cfd, toCbuf, (int) strlen(toCbuf));
                                SSL_write(cfd, buf, (int) strlen(buf));
                                char line[PKG_BODY_LEN];
                                memset(line, 0x00, sizeof (line));
                                SSL_read(cfd, line, PKG_HEAD_LEN);
                                //正常处理HTTP协议  
                                if (memcmp(line, "FL", 2) != 0) {
                                    continue;
                                }

                                g_pkgBodyLen = 0;
                                memset(g_pkgBody, 0x00, sizeof (g_pkgBody));
                                memcpy(g_pkgBody, line + 2, PKG_HEAD_LEN - 2);
                                g_pkgBodyLen = atoi(g_pkgBody);
                                if (g_pkgBodyLen > PKG_BODY_LEN) {
                                    continue;
                                }
                                memset(retMsg, 0x00, sizeof (retMsg));
                                SSL_read(cfd, retMsg, g_pkgBodyLen);


                                s = write(events[i].data.fd, retMsg, strlen(retMsg));
                                if (s == -1) {
                                    perror("write");
                                    abort();
                                }
                                done = 1;
                                break;
                            }

                            if (done) {
                                debug(g_log, "Closed connection on descriptor %d\n",
                                        events[i].data.fd);

                                /* Closing the descriptor will make epoll remove it
                                 from the set of descriptors which are monitored. */
                                close(events[i].data.fd);
                            }
                        }
                    }
                }

            default:
                pool_setpid(childPid);
        }
    }
}

/******************************************************************************
函数名称∶CLD_OUT
函数功能∶信号SIGTERM捕捉函数（子进程）
输入参数∶无
输出参数∶无
返 回 值∶无
 ******************************************************************************/
void CLD_OUT(int a) {
    long lRet;
    char buffer[64];

    debug(g_log, "子进程关闭\n");
    //    sprintf(buffer, "子进程关闭时间：%s", GetSysTime());
    //
    //    TransLog("", 0, buffer);
    //    itcp_clear();
    //    CloseTransLog();
    //    DBClose();
    pool_poolclear();

    //    /* 断开共享内存 */
    //    lRet = DetachFssShm();
    //    if (lRet != 0)
    //    {
    //        printf("DetachFssShm error ret=[%ld]!\n", lRet);
    //        ErrLogF(__FILE__,
    //                "DetachFssShm error ret=[%ld]!\n",
    //                lRet);
    //    }
    //
    //    WriteLog();

    exit(0);
}

void SetNonBlock(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

char* gotoClient(char * msg) {
    int iRet = 0;
    static char buffer[PKG_BODY_LEN] = {0};

    //    if(4 != argc)
    //    {
    //    	perror("Parameter: ServerIP Message ServerPort -1");
    //        return -1;
    //    }

    //    int i16_port = atoi(port);
    int i16_port = g_global_conf.C0port;
    if (0 >= i16_port) {
        debug(g_log, "port wrong:%d\n", i16_port);
        return "-1";
    }

    int sk = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == sk) {
        perror("open socket failed!");
        return "-1";
    }


    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(i16_port);

    struct sockaddr_in *psa = &sa;

    iRet = inet_pton(AF_INET, "127.0.0.1", &psa->sin_addr.s_addr);
    if (0 == iRet) {
        perror("inet_pton failed, invalid address! -1 ");
        close(sk);
        return "-1";
    } else if (iRet < 0) {
        perror("inet_pton failed");
        close(sk);
        return "-1";
    }

    if (connect(sk, (struct sockaddr*) &sa, sizeof (sa)) < 0) {
        perror("connect failed");
        close(sk);
        return "-1";
    }

    SetNonBlock(sk);

    int efd;
    efd = epoll_create(10);
    if (efd == -1) {
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
    while (1) {
        ssize_t numBytesRcvd = 0;
        int n = 0;
        int i = 0;

        if (loop == 1) {
            break;
        }

        n = epoll_wait(efd, events, 10, -1);

        debug(g_log, "epoll_wait:%d\n", n);

        for (i = 0; i < n; i++) {
            if (events[i].events & EPOLLOUT) {
                debug(g_log, "EPOLLOUT...............\n");
                snprintf(buffer, PKG_BODY_LEN, "%s", msg);

                int n = strlen(buffer);
                int nsend = 0;

                while (n > 0) {
                    //nsend = send(events[i].data.fd, buffer + nsend, n, 0);
                    nsend = write(events[i].data.fd, buffer + nsend, n);
                    if (nsend < 0 && errno != EAGAIN) {

                        perror("send failed");
                        close(events[i].data.fd);
                        return "-1";
                    }
                    n -= nsend;
                }
            }

            if (events[i].events & EPOLLIN) {
                debug(g_log, "EPOLLIN...............\n");
                memset(buffer, 0, PKG_BODY_LEN);

                int len = strlen(buffer);
                int n = 0;
                int nrecv = 0;

                //while((nrecv = recv(events[i].data.fd, buffer + n, PKG_BODY_LEN - 1, 0)) > 0)
                while (1) {
                    nrecv = read(events[i].data.fd, buffer + n, PKG_BODY_LEN - 1);
                    if (nrecv == -1 && errno != EAGAIN) {
                        perror("read error!");
                    }
                    if ((nrecv == -1 && errno == EAGAIN) || nrecv == 0) {
                        break;
                    }
                    n += nrecv;
                }
                loop = 1;
                debug(g_log, "buffer:%s\n", buffer);
            }
        }
    }
    close(sk);
    close(efd);
    return buffer;
}

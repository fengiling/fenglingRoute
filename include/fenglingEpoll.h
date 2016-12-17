/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fenglingEpoll.h
 * Author: chenzifeng
 *
 * Created on 2016年11月16日, 下午9:43
 */

#ifndef FENGLINGEPOLL_H
#define FENGLINGEPOLL_H

//函数:
//功能:创建和绑定一个TCP socket
//参数:端口
//返回值:创建的socket
static int create_and_bind();
//函数
//功能:设置socket为非阻塞的
static int make_socket_non_blocking(int sfd);

int EpollServerInit();
int EpollServerRun();
void CLD_OUT(int a);;
void SetNonBlock(int fd);
char* gotoClient(char * msg);


#endif /* FENGLINGEPOLL_H */


/*
 * routeServer.h
 *
 *  Created on: 2016年8月11日
 *      Author: chenzifeng
 */

#ifndef ROUTESERVER_H_
#define ROUTESERVER_H_

//函数:
//功能:创建和绑定一个TCP socket
//参数:端口
//返回值:创建的socket
static int create_and_bind(char *port);
//函数
//功能:设置socket为非阻塞的
static int make_socket_non_blocking(int sfd);
//启动服务
//端口由参数 port 指定
int runServer(char *port);

#endif /* ROUTESERVER_H_ */

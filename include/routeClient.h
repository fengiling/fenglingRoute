/*
 * routeClit.h
 *
 *  Created on: 2016��8��11��
 *      Author: chenzifeng
 */

#ifndef ROUTECLIENT_H_
#define ROUTECLIENT_H_

void SetNonBlock(int fd);
char* gotoClient(char *ip,int port,char *msg);

#endif /* ROUTECLIENT_H_ */

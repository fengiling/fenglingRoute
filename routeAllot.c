/*
 * routeAllot.c
 *
 *  Created on: 2016年8月16日
 *      Author: chenzifeng
 */
#include "routeBase.inc"

client_server * routeAllot() {
    //注释
	for (int i = 0; i < MAX_SERVER; i++) {
		printf("%s %d\n", server[i].ip, server[i].port);
	}
	return &server[0];
}

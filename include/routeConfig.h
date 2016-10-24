/*
 * routeConfig.h
 *
 *  Created on: 2016年8月15日
 *      Author: chenzifeng
 */

#ifndef INCLUDE_ROUTECONFIG_H_
#define INCLUDE_ROUTECONFIG_H_

extern char *configFile;

#ifndef MAX_SERVER
#define MAX_SERVER        1
#endif

typedef struct
{
    char    ip[16];
    int    port;
}client_server;

extern client_server server[MAX_SERVER];

extern int serverCount;

int ReadConfig(char *configFile);

#endif /* INCLUDE_ROUTECONFIG_H_ */

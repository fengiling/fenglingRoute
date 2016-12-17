/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <string.h>

#include "include/fenglingconf.h"
extern conf_global_t g_global_conf;

char * parseHtmlHead(char *buf) {
    char *sHtmlHead;
    char *sSubdomainPot;
    char *sSubdomain;
    
    sHtmlHead = strtok(buf, "\r\n\r\n");
    sSubdomainPot = strstr(sHtmlHead, "Referer");
    sSubdomainPot = strstr(sSubdomainPot, "//") + 2;
    sSubdomain = strtok(sSubdomainPot, g_global_conf.Domain);
    return sSubdomain;
}
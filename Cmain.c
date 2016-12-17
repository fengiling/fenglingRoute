/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Cmain.c
 * Author: chenzifeng
 *
 * Created on 2016年11月24日, 下午10:12
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/fenglingConf.h"
#include "include/fenglingLog.h"

//#include "fenglingSem.h"
#include "include/fenglingShm.h"
#include "include/fenglingCA.h"
#include "include/fenglingProcess.h"
#include "include/fenglingEtc.h"


#define VERSION "0.1.0"

conf_global_t g_global_conf;
conf_filter_t g_filter_conf;

log_t *g_log;

//static int cpu_attach(pid_t pid, int cpu);

#define USAGE(){ \
    fprintf(stderr, "Version: %s\n", VERSION); \
    fprintf(stderr, "Usage: %s config\n\n", argv[0]); \
    fprintf(stderr, "Copyright 2011-2013 Alibaba Group Holding Limited. All rights reserved.\n"); \
    fprintf(stderr, "Use and distribution licensed under the GPL license.\n\n"); \
    fprintf(stderr, "Authors: XiaoJinliang <xiaoshi.xjl@taobao.com>\n"); \
}

/*
 * 
 */
int main(int argc, char** argv) {
    int ret, level;
    /* 初始化配置 */

    // argument parse
    if (argc != 2) {
        USAGE();
        exit(-1);
    }

    if (!strcmp(argv[1], "-h")) {
        USAGE();
        exit(0);
    }

    // config file parse
    if ((ret = conf_init(argv[1])) < 0) {
        log(g_log, "conf[%s] init error\n", argv[1]);
        exit(-1);
    } else {
        log(g_log, "conf[%s] init success\n", argv[1]);
    }

    // log init
    if (!strncmp(g_global_conf.log_level, "log", 3)) {
        level = LOG_LEVEL_LOG;
    } else if (!strncmp(g_global_conf.log_level, "debug", 5)) {
        level = LOG_LEVEL_DEBUG;
    } else if (!strncmp(g_global_conf.log_level, "info", 4)) {
        level = LOG_LEVEL_INFO;
    } else if (!strncmp(g_global_conf.log_level, "none", 4)) {
        level = LOG_NONE;
    } else {
        log(g_log, "log_level[%s] unknown\n", g_global_conf.log_level);
        exit(-1);
    }

    if ((g_log = log_init(g_global_conf.log_path, level)) == NULL) {
        log(g_log, "log[%s] init error\n", g_global_conf.log_path);
        exit(-1);
    }

    // signal init
    if ((signal_init()) < 0) {
        log(g_log, "signal init error\n");
        exit(-1);
    } else {
        log(g_log, "signal init success\n");
    }

    daemon_start(1);
    
    /* 初始化端口 */
    EpollShmInit();
    CAShmInit();
    CARun();

    return (EXIT_SUCCESS);
}

//static int cpu_attach(pid_t pid, int cpu)
//{
//    cpu_set_t mask;
//
//    CPU_ZERO(&mask);
//    CPU_SET(cpu, &mask);
//
//    if(sched_setaffinity(pid, sizeof(cpu_set_t), &mask)){
//        log(g_log, "sched_setaffinity: %s\n", strerror(errno));
//        return -1;
//    }
//
//    return 0;
//
//}

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <sys/shm.h>

#include "fenglingShm.h"
#include "fenglingSem.h"
#include "fenglingConf.h"
#include "include/fenglingLog.h"

extern log_t *g_log;
extern int shmid, semid; /* 定义共享内存号码和信号号码 */
extern FLRoute *pFLRoute;

int main(int argc, char** argv) {
    void *share_memory = NULL; /* 保存共享内存的地址 */
    //struct shm_buff **shm_buff_inst; /* 定义共享内存的结构体指针 */
    //char  buff[BUF_SIZE];
    key_t ftokid;
    int i;

    debug(g_log, "in CAShmInit\n");

    IgnoreSignal(); /* 屏蔽中断信号 */
    ftokid = ftok("/dev/null", /*"CAa"*/1);
    debug(g_log, "ftokid:[%d]\n", (int) ftokid);
    if (ftokid == -1) /* 判断共享内存是否获得成功 */ {
        debug(g_log, "Get shared memory failed!\n %s\n", strerror(errno));
        del_sem(shmid);
        exit(-1);
    }
    semid = semget(ftokid, 1, 0666 | IPC_CREAT); /* 获得信号号码 */
    init_sem(semid, 1); /*初始化信号值为1*/

    ftokid = ftok("/dev/null", /*"CAa"*/2);
    debug(g_log, "ftokid:[%d]\n", (int) ftokid);
    if (ftokid == -1) /* 判断共享内存是否获得成功 */ {
        debug(g_log, "Get shared memory failed!\n %s\n", strerror(errno));
        del_sem(shmid);
        exit(-1);
    }
    shmid = shmget(ftokid, sizeof (FLRoute) * MAXROUTE, 0666 | IPC_CREAT); /* 获得共享内存号码 */
    if (shmid == -1) /* 判断共享内存是否获得成功 */ {
        debug(g_log, "Get shared memory failed!\n %s\n", strerror(errno));
        del_sem(shmid);
        exit(-1);
    }

    share_memory = shmat(shmid, (void *) 0, 0); /*让share_memory映射共享内存*/
    if (share_memory == (void *) - 1) /*看映射共享内存是否成功*/ {
        debug(g_log, "Shmat error!\n");
        del_sem(shmid);
        exit(-1);
    }
    debug(g_log, "Memory attached at %X\n", share_memory);

    pFLRoute = (FLRoute *) share_memory; /*将share_memory强制转换为shm_buff结构指针*/
    //shm_buff_inst = (struct shared_usr_st *)share_memory;

    for (i = 0; i < MAXROUTE; ++i) {
        //pFLRoute[i].subdomain = NULL;
        //t->bucket[i].value = NULL;
        //pFLRoute[i].next = NULL;
        //        pFLRoute[i].total = 0;
        //        pFLRoute[i].nowcounts = 0;
        //        pFLRoute[i].now = 0;
        //        memset(pFLRoute[i].subdomain, 0, sizeof (pFLRoute[i].subdomain));
        //        memset(pFLRoute[i].list, 0, sizeof (pFLRoute[i].list));
        //        memset(pFLRoute[i].fd, 0, sizeof (pFLRoute[i].fd));
        //        log(g_log, "line:[%d]", i);
        //        log(g_log, "subdomain:[%s]", pFLRoute[i].subdomain);
        //        log(g_log, "total    :[%d]", pFLRoute[i].total);
        //        log(g_log, "nowcounts:[%d]", pFLRoute[i].nowcounts);
        //        log(g_log, "now      :[%d]", pFLRoute[i].now);
        //        for (int j = 0; j < MAXCLIENTCONN; j++) {
        //            log(g_log, "list     :[%d]", pFLRoute[i].list[j]);
        //        }
        //        for (int j = 0; j < MAXCLIENTCONN; j++) {
        //            log(g_log, "fd       :[%ld]", (long) pFLRoute[i].fd[j]);
        //        }
        //        log(g_log, "\n");

        printf("line:[%02d]\t", i);
        printf("subdomain:[%s]\t", pFLRoute[i].subdomain);
        printf("total:[%d]\t", pFLRoute[i].total);
        printf("nowcounts:[%d]\t", pFLRoute[i].nowcounts);
        printf("now:[%d]\t", pFLRoute[i].now);
        printf("list:");
        for (int j = 0; j < MAXCLIENTCONN; j++) {
            printf("[%d]", pFLRoute[i].list[j]);
        }
        printf("\tfd:");
        for (int j = 0; j < MAXCLIENTCONN; j++) {
            printf("[%ld]", (long) pFLRoute[i].fd[j]);
        }
        printf("\n");
    }

    return (0);
}
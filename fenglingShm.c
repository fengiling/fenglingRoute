/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h> 
#include <time.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/un.h>  
#include <arpa/inet.h>  
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include<sys/sem.h>

#include <openssl/ssl.h>  
#include <openssl/bio.h>  
#include <openssl/err.h>  

#include "include/fenglingShm.h"
#include "include/fenglingSem.h"
#include "include/fenglingConf.h"
#include "include/fenglingLog.h"


extern int shmid, semid; /* 定义共享内存号码和信号号码 */
extern FLRoute *pFLRoute;
extern POOL_SHM *pSProc;
extern conf_global_t g_global_conf;
extern log_t *g_log;

int Epollsemid;
int Epollshmid;
int _lAProc;
long _lCurRec;

int CAShmInit() {
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
        pFLRoute[i].total = 0;
        pFLRoute[i].nowcounts = 0;
        pFLRoute[i].now = 0;
        memset(pFLRoute[i].subdomain, 0, sizeof (pFLRoute[i].subdomain));
        memset(pFLRoute[i].list, 0, sizeof (pFLRoute[i].list));
        memset(pFLRoute[i].fd, 0, sizeof (pFLRoute[i].fd));
    }

    return (0);
}

int SetFLRoute(char* Subdomian, SSL *cli_sock, int total) {
    int index, initindex, ret;
    initindex = index = keyToIndex(Subdomian);

    sem_p(semid); /*P操作*/
    while (1) {
        if (strlen(pFLRoute[index].subdomain) == 0) {
            if (strlen(Subdomian) >= MAXSUBDOMAIN) {
                strncpy(pFLRoute[index].subdomain, Subdomian, MAXSUBDOMAIN - 1);
            } else {
                sprintf(pFLRoute[index].subdomain, "%s", Subdomian);
            }
            pFLRoute[index].total = total;
            pFLRoute[index].nowcounts = 1;
            pFLRoute[index].now = 0;
            //pFLRoute[index].list[0] = 1;
            pFLRoute[index].fd[0] = cli_sock;
            break;
        } else {
            if (strlen(Subdomian) >= MAXSUBDOMAIN) {
                ret = strncmp(pFLRoute[index].subdomain, Subdomian, MAXSUBDOMAIN);
            } else {
                ret = strcmp(pFLRoute[index].subdomain, Subdomian);
            }

            if (ret == 0 && pFLRoute[index].nowcounts <= g_global_conf.S1MaxListen) {
                pFLRoute[index].total = total;
                pFLRoute[index].fd[pFLRoute[index].nowcounts] = cli_sock;
                pFLRoute[index].nowcounts++;
            } else {
                index++;
                if (index == MAXSUBDOMAIN)index = 0;
                if (initindex == index) {
                    sem_v(semid); /*V操作*/
                    return (-1);
                }
            }
        }
    }
    sem_v(semid); /*V操作*/
    return (EXIT_SUCCESS);
}

SSL * GetFLRoute(char * Subdomian) {
    int index, initindex, ret;
    SSL * cli_sock;
    initindex = index = keyToIndex(Subdomian);

    sem_p(semid); /*P操作*/
    while (1) {
        if (strlen(pFLRoute[index].subdomain) == 0) {
            sem_v(semid); /*V操作*/
            return NULL;
        } else {
            if (strlen(Subdomian) >= MAXSUBDOMAIN) {
                ret = strncmp(pFLRoute[index].subdomain, Subdomian, MAXSUBDOMAIN);
            } else {
                ret = strcmp(pFLRoute[index].subdomain, Subdomian);
            }

            if (ret == 0) {
                cli_sock = pFLRoute[index].fd[pFLRoute[index].now];
                pFLRoute[index].now++;
                if (pFLRoute[index].now == pFLRoute[index].total)
                    pFLRoute[index].now = 0;
                break;
            } else {
                index++;
                if (index == MAXSUBDOMAIN)index = 0;
                if (initindex == index) {
                    sem_v(semid); /*V操作*/
                    return NULL;
                }
            }
        }
    }
    sem_v(semid); /*V操作*/

    return (cli_sock);
}

int RmFLRoute(char* Subdomian, SSL *cli_sock) {
    int index, initindex, ret, i;

    initindex = index = keyToIndex(Subdomian);

    sem_p(semid); /*P操作*/
    while (1) {
        if (strlen(pFLRoute[index].subdomain) == 0) {
            sem_v(semid); /*V操作*/
            return (EXIT_SUCCESS);
        } else {
            if (strlen(Subdomian) >= MAXSUBDOMAIN) {
                ret = strncmp(pFLRoute[index].subdomain, Subdomian, MAXSUBDOMAIN);
            } else {
                ret = strcmp(pFLRoute[index].subdomain, Subdomian);
            }

            if (ret == 0) {
                for (i = 0; i < pFLRoute[index].nowcounts; i++) {
                    if (pFLRoute[index].fd[i] == cli_sock) {
                        for (; i < pFLRoute[index].nowcounts - 1; i++) {
                            pFLRoute[index].fd[i] = pFLRoute[index].fd[i + 1];
                        }
                        pFLRoute[index].fd[--i] = 0;
                        pFLRoute[index].nowcounts--;
                        pFLRoute[index].now--;
                        break;
                    }
                }
                break;
            } else {
                index++;
                if (index == MAXSUBDOMAIN)index = 0;
                if (initindex == index) {
                    sem_v(semid); /*V操作*/
                    return (EXIT_SUCCESS);
                }
            }
        }
    }
    sem_v(semid); /*V操作*/

    return (EXIT_SUCCESS);
}

int keyToIndex(const char* key) {
    int index, len, i;
    if (key == NULL)return -1;

    len = strlen(key);
    index = (int) key[0];
    for (i = 1; i < len; ++i) {
        index *= 1103515245 + (int) key[i];
    }
    index >>= 27;
    index &= (MAXROUTE - 1);
    return index;
}

int EpollShmInit() {
    void *share_memory = NULL; /* 保存共享内存的地址 */
    //struct shm_buff **shm_buff_inst; /* 定义共享内存的结构体指针 */
    //char  buff[BUF_SIZE];

    int i;

    debug(g_log, "in EpollShmInit\n");

    _lAProc = g_global_conf.SMaxPoll;
    IgnoreSignal(); /* 屏蔽中断信号 */
    Epollsemid = semget(ftok("/dev/null", /*"Epolla"*/3), 1, 0666 | IPC_CREAT); /* 获得信号号码 */
    init_sem(Epollsemid, 1); /*初始化信号值为1*/

    debug(g_log, "Epollsemid:[%d]\n", Epollsemid);

    Epollshmid = shmget(ftok("/dev/null", /*"Epollb"*/4), sizeof (POOL_SHM) * g_global_conf.SMaxPoll, 0666 | IPC_CREAT); /* 获得共享内存号码 */
    if (Epollshmid == -1) /* 判断共享内存是否获得成功 */ {
        debug(g_log, "Get shared memory failed!\n");
        del_sem(Epollshmid);
        exit(-1);
    }

    share_memory = shmat(Epollshmid, (void *) 0, 0); /*让share_memory映射共享内存*/
    if (share_memory == (void *) - 1) /*看映射共享内存是否成功*/ {
        debug(g_log, "Shmat error!\n");
        del_sem(Epollshmid);
        exit(-1);
    }
    debug(g_log, "Memory attached a:[%X]\n", share_memory);
    //    printf("Memory attached at %X\n", (int) share_memory);
    pSProc = (POOL_SHM *) share_memory; /*将share_memory强制转换为shm_buff结构指针*/
    //shm_buff_inst = (struct shared_usr_st *)share_memory;

    for (i = 0; i < g_global_conf.SMaxPoll; ++i) {
        //pFLRoute[i].subdomain = NULL;
        //t->bucket[i].value = NULL;
        //pFLRoute[i].next = NULL;
        pSProc[i].PID = 0;
        pSProc[i].lStatus = 0;
        debug(g_log, "PID:[%d]\n", pSProc[i].PID);
        debug(g_log, "lStatus:[%d]\n", pSProc[i].lStatus);
    }

    return (0);
}

/************************************************************************
函数名称∶pool_monitor
函数功能∶监控进程池
输入参数∶lDelay 循环间隔时间
          lKillWait 进程最大空闲时间
输出参数∶
返 回 值∶>=  0为进程数 
           = -1为返回最大进程数 
 *************************************************************************/
long pool_monitor() {
    long /*lBusyNum, lProcNum,*/ lCnt;
    //time_t NowTime;

    debug(g_log, "in pool_monitor\n");

    while (1) {
        debug(g_log, "start while\n");
        for (lCnt = 0; lCnt < g_global_conf.SMaxPoll; lCnt++) {
            if (pSProc[lCnt].PID > 0 && kill(pSProc[lCnt].PID, 0) != 0) {
                pSProc[lCnt].PID = 0;
                pSProc[lCnt].lStatus = 0;
                //pSProc[lCnt].Time = 0;
            }

            if (lCnt < g_global_conf.SMaxPoll) {
                if (pSProc[lCnt].PID == 0) {
                    _lCurRec = lCnt;
                    return _lCurRec;
                }
            }
        }

        sleep(LDELAY);
    }
}

/************************************************************************
函数名称∶pool_setstatus
函数功能∶设置进程状态
输入参数∶lStatus : 经常状态
输出参数∶
返 回 值∶
 *************************************************************************/
void pool_setstatus(long lStatus) {
    //pSProc[_lCurRec].lStatus = lStatus;
    //pSProc[_lCurRec].Time = time(NULL);
}

/************************************************************************
函数名称∶pool_setpid
函数功能∶设置进程id
输入参数∶ChildID : 进程id
输出参数∶
返 回 值∶
 *************************************************************************/
void pool_setpid(pid_t ChildID) {
    debug(g_log, "pid:[%ld]\n", ChildID);
    pSProc[_lCurRec].PID = ChildID;
    //pSProc[_lCurRec].Time = time(NULL);
}

/************************************************************************
函数名称∶pool_poolclear
函数功能∶清空进程池（不含父进程）
输入参数∶
输出参数∶
返 回 值∶
 *************************************************************************/
void pool_poolclear() {
    long lCnt;

    if (getppid() == 1) {
        for (lCnt = 0; lCnt < _lAProc; lCnt++) {
            if (pSProc[lCnt].PID > 0) {
                if (kill(pSProc[lCnt].PID, SIGTERM) != 0) {
                    debug(g_log, "kill pid [%d] failed!\n", pSProc[lCnt].PID);
                }
            }
        }

        shmctl(Epollshmid, IPC_RMID, 0);
    }
}

int CEpollShmInit() {
    void *share_memory = NULL; /* 保存共享内存的地址 */
    //struct shm_buff **shm_buff_inst; /* 定义共享内存的结构体指针 */
    //char  buff[BUF_SIZE];

    int i;
    _lAProc = g_global_conf.CMaxPoll
            ;
    IgnoreSignal(); /* 屏蔽中断信号 */
    Epollsemid = semget(ftok("/dev/null", /*"Epolla"*/5), 1, 0666 | IPC_CREAT); /* 获得信号号码 */
    init_sem(Epollsemid, 1); /*初始化信号值为1*/

    Epollshmid = shmget(ftok("/dev/null", /*"Epollb"*/6), sizeof (POOL_SHM) * g_global_conf.CMaxPoll, 0666 | IPC_CREAT); /* 获得共享内存号码 */
    if (Epollshmid == -1) /* 判断共享内存是否获得成功 */ {
        debug(g_log, "Get shared memory failed!\n");
        del_sem(Epollshmid);
        exit(-1);
    }

    share_memory = shmat(Epollshmid, (void *) 0, 0); /*让share_memory映射共享内存*/
    if (share_memory == (void *) - 1) /*看映射共享内存是否成功*/ {
        debug(g_log, "Shmat error!\n");
        del_sem(Epollshmid);
        exit(-1);
    }
    debug(g_log, "Memory attached at %X\n", share_memory);
    pSProc = (POOL_SHM *) share_memory; /*将share_memory强制转换为shm_buff结构指针*/
    //shm_buff_inst = (struct shared_usr_st *)share_memory;

    for (i = 0; i < g_global_conf.CMaxPoll; ++i) {
        //pFLRoute[i].subdomain = NULL;
        //t->bucket[i].value = NULL;
        //pFLRoute[i].next = NULL;
        pSProc[i].PID = 0;
        pSProc[i].lStatus = 0;
    }

    return (EXIT_SUCCESS);
}

/************************************************************************
函数名称∶pool_monitor
函数功能∶监控进程池
输入参数∶lDelay 循环间隔时间
          lKillWait 进程最大空闲时间
输出参数∶
返 回 值∶>=  0为进程数 
           = -1为返回最大进程数 
 *************************************************************************/
long Cpool_monitor() {
    long /*lBusyNum, lProcNum,*/ lCnt;
    //    long _lCurRec;
    //time_t NowTime;

    while (1) {
        debug(g_log, "start while\n");
        for (lCnt = 0; lCnt < g_global_conf.CMaxPoll; lCnt++) {
            if (pSProc[lCnt].PID > 0 && kill(pSProc[lCnt].PID, 0) != 0) {
                pSProc[lCnt].PID = 0;
                pSProc[lCnt].lStatus = 0;
                //pSProc[lCnt].Time = 0;
            }

            if (lCnt < g_global_conf.CMaxPoll) {
                if (pSProc[lCnt].PID == 0) {
                    _lCurRec = lCnt;
                    return _lCurRec;
                }
            }
        }

        sleep(LDELAY);
    }
}

int IgnoreSignal(void) {
    signal(SIGINT, SIG_IGN); /*对信号SIGINT进行忽略*/
    signal(SIGQUIT, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
    return 0;
}
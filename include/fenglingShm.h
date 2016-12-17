/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fenglingShm.h
 * Author: chenzifeng
 *
 * Created on 2016年11月18日, 上午11:30
 */

#ifndef FENGLINGSHM_H
#define FENGLINGSHM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <openssl/ssl.h>  
#include <openssl/bio.h>  
#include <openssl/err.h>  

    /*shm.h文件*/

#define BUF_SIZE     1024
#define SHM_BUF_SIZE 2048
#define END_FLAG     "QUIT"  /*用户输入的退出标志*/

#define BUCKETCOUNT 16
#define MAXCLIENTCONN 10
#define MAXSUBDOMAIN 20
#define MAXROUTE 50





#define LDELAY 30

    typedef struct /* 共享内存中数据结构 */ {
        //int pid; /*保存写入数据进程的ID*/
        //char buff[SHM_BUF_SIZE]; /*保存写入的数据*/
        char subdomain[MAXSUBDOMAIN];
        //char* value;
        int total;
        int nowcounts;
        int now;
        int list[MAXCLIENTCONN];
        SSL * fd[MAXCLIENTCONN];
        //struct hashEntry* next;
    } FLRoute;

    typedef struct {
        pid_t PID;
        long lStatus;
        //time_t Time;
    } POOL_SHM;

    int shmid, semid; /* 定义共享内存号码和信号号码 */

    static FLRoute *pFLRoute;
    static POOL_SHM *pSProc;

    int CAShmInit();
    int SetFLRoute(char* Subdomian, SSL *cli_sock, int total);
    SSL * GetFLRoute(char* Subdomian);
    int RmFLRoute(char* Subdomian, SSL *cli_sock);
    int keyToIndex(const char* key);
    int EpollShmInit();
    long pool_monitor();
    void pool_setstatus(long lStatus);
    void pool_setpid(pid_t ChildID);
    void pool_poolclear();
    int CEpollShmInit();
    long Cpool_monitor();
    int IgnoreSignal(void);

#ifdef __cplusplus
}
#endif

#endif /* FENGLINGSHM_H */


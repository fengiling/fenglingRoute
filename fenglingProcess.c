/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <signal.h> 
#include <errno.h>
#include <stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>

#include "include/fenglingProcess.h"
#include "include/fenglingLog.h"
#include "include/fenglingShm.h"

extern log_t *g_log;
int g_reload;

void cld_out(int a) {
    long lRet;
    int istatus, isig;
    pid_t pid;

    fprintf(stderr, "enter cld_out,begin waitpid,errno=%d\n", errno);
    pid = waitpid(0, &istatus, WNOHANG);

    fprintf(stderr, "after waitpid,errno=%d\n", errno); /* 子进程正常退出*/
    if (WIFEXITED(istatus)) /*正常终止的子进程*/ {
        fprintf(stderr, "子进程正常终止 istatus=%d,pid=%d\n",  istatus, pid);
        return;
    }
    if (WIFSIGNALED(istatus)) /*由于异常而终止的子进程 */ {
        isig = WTERMSIG(istatus);
        printf("abnormal termination,signal istatus=%d,isig=%d,pid=%d,%s:\n", istatus, isig, pid,

#ifdef WCOREDUMP
                WCOREDUMP(istatus) ? ")core file generated)" : "");
#else
                "");
#endif
        /*psignal(isig,"");  会显示信号6的意思：IOT/Abort trap*/
        fprintf(stderr, "子进程异常终止 istatus=%d,isig=%d,pid=%d,errno=%d\n", istatus, isig, pid, errno);
        debug(g_log, "子进程异常终止 istatus=%d,isig=%d,pid=%d,errno=%d\n",  istatus, isig, pid, errno);
        return;
    }
    fprintf(stderr, "子进程未知终止 istatus=%d,pid=%d,errno=%d\n",  istatus, pid, errno);
    debug(g_log, "子进程未知终止 istatus=%d,pid=%d,errno=%d\n",  istatus, pid, errno);

    return;
}

int signal_init(void) {
    struct sigaction new_action_cld;

    //signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGILL, SIG_IGN);
    signal(SIGTRAP, SIG_IGN);
    signal(SIGABRT, SIG_IGN);
    signal(SIGKILL, SIG_IGN);
    signal(SIGUSR1, signal_usr1);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGCONT, SIG_IGN);

    signal(SIGKILL, OUT);
    new_action_cld.sa_handler = cld_out; /* cld_out为SIGCHLD信号处理函数名*/
    sigemptyset(&new_action_cld.sa_mask);
    new_action_cld.sa_flags = 0;
    sigaction(SIGCHLD, &new_action_cld, NULL);

    return 0;
}

void signal_usr1(int signal) {
    g_reload = 1;

    return;
}

/******************************************************************************
函数名称∶OUT
函数功能∶信号SIGTERM捕捉函数
输入参数∶无
输出参数∶无
返 回 值∶无
 ******************************************************************************/
void OUT(int a) {
    char buffer[64];

    debug(g_log, "进程关闭\n");
    //    TransLog("", 0, buffer);
    //    itcp_clear();
    //    CloseTransLog();
    pool_poolclear();
    exit(0);
}


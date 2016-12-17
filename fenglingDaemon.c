/************************************************************************
文件名称∶daemon.c
系统名称∶存管系统
模块名称∶
功能说明∶进程组底层库
系统版本∶1.0
开发人员∶存管系统项目组
开发时间∶
审核人员∶
审核时间∶
相关文档∶
修改记录∶修改记录号{M+修改顺序号.}   修改日期{YYYY/MM/DD}   修改人   修改
          V1.0                        2014/10/31             陈子峰   新建
*************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "include/etc.h"

extern int errno;

/************************************************************************
函数名称∶daemon_start
函数功能∶初始化进程组
输入参数∶ignsigcld : 是否向父进程发送通知
输出参数∶
返 回 值∶
*************************************************************************/
int daemon_start(int ignsigcld)
{
    register int childpid, fd;
    
    if (getppid() == 1)
    {
        goto out;
    }
    
#ifdef SIGTTOU
    signal(SIGTTOU, SIG_IGN);
#endif

#ifdef SIGTTIN
    signal(SIGTTIN, SIG_IGN);
#endif

#ifdef SIGTSTP
    signal(SIGTSTP, SIG_IGN);
#endif

    if ((childpid = fork()) < 0)
    { 
        fprintf(stderr, "Can't fork first child \n");
        exit(1);
    } 
    else 
    if (childpid > 0)
        exit(0);

    if (setpgrp() == -1)
    {
        fprintf(stderr, "Can't change process group \n");
        exit(1);
    }
     
    signal(SIGHUP, SIG_IGN);
     
    if ((childpid = fork()) < 0)
    {
        fprintf(stderr, "Can't fork second child \n");
        exit(1);
    }
    else 
    if (childpid > 0)
        exit(0);

out: 
    /* 
    for (fd = 0; fd <= 2; fd++)
        close(fd);
    */
    errno = 0;
     
    chdir("/");
    
    /* 去掉umask (0)函数
    umask(0);
    */ 
    if (ignsigcld) 
    {
        signal(SIGCLD, SIG_IGN);
    }
    return 0;
}

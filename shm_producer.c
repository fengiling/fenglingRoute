/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*shm_producer.c文件*/

#include "sem.h"
#include "shm.h"
#include <signal.h>
#include <string.h>

//#define  BUF_SIZE 512

int IgnoreSignal(void); /* 忽略信号 */

int main(void) {
    void *share_memory = NULL; /* 保存共享内存的地址 */
    struct shm_buff *shm_buff_inst; /* 定义共享内存的结构体指针 */
    //char  buff[BUF_SIZE];
    int shmid, semid; /* 定义共享内存号码和信号号码 */
    IgnoreSignal(); /* 屏蔽中断信号 */
    semid = semget(ftok(".", 'a'), 1, 0666 | IPC_CREAT); /* 获得信号号码 */
    init_sem(semid, 1); /*初始化信号值为1*/

    shmid = shmget(ftok(".", 'b'), sizeof (struct shm_buff), 0666 | IPC_CREAT); /* 获得共享内存号码 */
    if (shmid == -1) /* 判断共享内存是否获得成功 */ {
        printf("Get shared memory failed!\n");
        del_sem(shmid);
        exit(-1);
    }

    share_memory = shmat(shmid, (void *) 0, 0); /*让share_memory映射共享内存*/
    if (share_memory == (void *) - 1) /*看映射共享内存是否成功*/ {
        printf("Shmat error!\n");
        del_sem(shmid);
        exit(-1);
    }
    printf("Memory attached at %X\n", (int) share_memory);
    shm_buff_inst = (struct shm_buff *) share_memory; /*将share_memory强制转换为shm_buff结构指针*/
    //shm_buff_inst = (struct shared_usr_st *)share_memory;
    while (1) {
        sem_p(semid); /*P操作*/
        printf("Enter some text to the share memory: ");
        if (fgets(shm_buff_inst->buff, SHM_BUF_SIZE, stdin) == NULL) /*查看用户输入数据是否接受成功到shm_buff_inst->buff中*/ {
            printf("fgets error!\n");
            sem_v(semid);
            break;
        }
        shm_buff_inst->pid = getpid(); /*设置共享内存中pid字段的值*/
        sem_v(semid); /*V操作*/
        if (strncmp(shm_buff_inst->buff, END_FLAG, strlen(END_FLAG)) == 0) /*检测用户是否输入了退出信息*/ {
            printf("the flag to exit!\n");
            break;
        }
    } /* while */
    del_sem(semid); /*删除信号变量*/
    if (shmdt(share_memory) == 1) /*取消共享内存的映射*/ {
        printf("Shmdt error\n");
        exit(1);
    }
    return 1;
} /* main */

int IgnoreSignal(void) {
    signal(SIGINT, SIG_IGN); /*对信号SIGINT进行忽略*/
    signal(SIGQUIT, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
    return 0;
}


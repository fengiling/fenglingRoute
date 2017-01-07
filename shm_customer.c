/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*shm_customer.c文件*/

#include "sem.h"
#include "shm.h"
#include <signal.h>
#include <string.h>

//#define  BUF_SIZE 512

int main(void) {
    void *share_memory = NULL; /*定义映射共享内存的指针*/
    struct shm_buff *shm_buff_inst; /*定义共享内存结构类型的指针*/
    //char   buff[BUF_SIZE];
    int shmid, semid; /*定义共享内存变量和信号变量*/
    semid = semget(ftok(".", 'a'), 1, 0666 | IPC_CREAT); /*创建信号变量*/
    if (semid == -1) /*查看创建信号变量是否成功*/ {
        printf("Producer is'nt exist\n");
        exit(-1);
    }
    //init_sem(sem_id);

    shmid = shmget(ftok(".", 'b'), sizeof (struct shm_buff), 0666 | IPC_CREAT); /*创建共享内存空间*/
    if (shmid == -1) /*查看共享内存空间创建是否成功*/ {
        printf("Get shared memory failed!\n");
        del_sem(shmid);
        exit(-1);
    }

    share_memory = shmat(shmid, (void *) 0, 0); /*让share_memory映射共享内存空间*/
    if (share_memory == (void *) - 1) /*查看映射内存空间是否成功*/ {
        printf("Shmat error!\n");
        del_sem(shmid);
        exit(-1);
    }
    printf("Memory attached at %X\n", (int) share_memory);
    shm_buff_inst = (struct shm_buff *) share_memory; /*将share_memory强制转换为结构体shm_buff类型指针*/
    while (1) {
        sem_p(semid); /*P操作*/
        printf("Share memory written by %d process : %s", shm_buff_inst->pid, shm_buff_inst->buff);
        shm_buff_inst->pid = 0; /*使用后清空操作*/
        memset(shm_buff_inst->buff, 0, SHM_BUF_SIZE); /*使用后清空操作*/
        sem_v(semid); /*V操作*/
        if (strncmp(shm_buff_inst->buff, END_FLAG, strlen(END_FLAG)) == 0) /*查看是否退出标志*/ {
            break;
        }
    } /* while */
    del_sem(semid); /*删除信号变量*/
    if (shmdt(share_memory) == 1) /*取消内存映射*/ {
        printf("Shmdt error\n");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) /*删除共享内存空间*/ {
        printf("Shmctl error\n");
        exit(-1);
    }
    exit(0);
    return 1;
} /* main */






/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/fenglingSem.h"

/*初始化信号*/
void init_sem(int sem_id, int init_value) {
    union semun sem_union;
    sem_union.val = init_value; /*为共用体赋值*/
    //int sem_union = init_value;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) /* 判断设置信号初始值是否成功 */ {
        perror("semctl");
        exit(-1);
    }
}

/* 删除信号 */
void del_sem(int sem_id) {
    union semun sem_union;
    //int sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) /* 删除信号 */ {
        perror("semctl");
        exit(-1);
    }
}

/* P 操作 */
void sem_p(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0; /*单个信号量的编号应该为0*/
    sem_b.sem_op = -1; /* 减1, P操作 */
    sem_b.sem_flg = SEM_UNDO; /*系统自动释放残留在系统中的信号量*/
    if (semop(sem_id, &sem_b, 1) == -1) /*设置信号变量是否成功*/ {
        perror("sem_op");
        exit(-1);
    }
}

/* V 操作 */
void sem_v(int sem_id) {
    struct sembuf sem_b;
    sem_b.sem_num = 0; /*单个信号量的编号应该为0*/
    sem_b.sem_op = 1; /* 加1, V操作 */
    sem_b.sem_flg = SEM_UNDO; /*系统自动释放残留在系统中的信号量*/
    if (semop(sem_id, &sem_b, 1) == -1) /*设置信号变量是否成功*/ {
        perror("sem_op");
        exit(-1);
    }
}


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fenglingSem.h
 * Author: chenzifeng
 *
 * Created on 2016年11月18日, 上午11:28
 */

#ifndef FENGLINGSEM_H
#define FENGLINGSEM_H

#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

void init_sem(int sem_id, int init_value); /*初始化信号*/
void del_sem(int sem_id); /* 删除信号 */
void sem_p(int sem_id); /* P 操作 */
void sem_v(int sem_id); /* V 操作 */

#ifndef _SEMUN_H_
#define _SEMUN_H_

union semun /*信号设置的共用体数据结构*/ {
    int val; /*value for SETVAL*/
    struct semid_ds *buf; /*buffer for IPC_STAT,IPC_SET*/
    unsigned short int *arrary; /*arrary for GETALL,SETALL*/
    struct seminfo *_buf; /*buffer for IPC_INFO*/
};
#endif


#endif /* FENGLINGSEM_H */


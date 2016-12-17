/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fenglingProcess.h
 * Author: chenzifeng
 *
 * Created on 2016年12月7日, 上午11:25
 */

#ifndef FENGLINGPROCESS_H
#define FENGLINGPROCESS_H

#ifdef __cplusplus
extern "C" {
#endif


    void cld_out(int); /* 子进程退出后wait守护*/

    int signal_init(void);
    void signal_usr1(int signal);
    void OUT(int a);

#ifdef __cplusplus
}
#endif

#endif /* FENGLINGPROCESS_H */


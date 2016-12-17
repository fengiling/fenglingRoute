/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fenglingCA.h
 * Author: chenzifeng
 *
 * Created on 2016年11月27日, 下午8:19
 */

#ifndef FENGLINGCA_H
#define FENGLINGCA_H

#ifdef __cplusplus
extern "C" {
#endif

#define PKG_HEAD_LEN 10
#define PKG_BODY_LEN 20480

int password_callback(char *buf, int size, int rwflag, void *userdata);
int CAServerInit() ;
int CARun() ;



#ifdef __cplusplus
}
#endif

#endif /* FENGLINGCA_H */


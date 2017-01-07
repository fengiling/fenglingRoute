/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CAClient.c
 * Author: chenzifeng
 *
 * Created on 2016年11月15日, 上午11:15
 */

#include <openssl/ssl.h>  
#include <errno.h>  
#include <openssl/bio.h>  
#include <openssl/err.h>  

#include <stdio.h>  
#include <string.h>  

int main() {
    BIO *sslbio;
    SSL * ssl;
    SSL_CTX * ctx;

    int p;


    /* Set up the library */
    SSL_library_init(); //一定要有，初始化ssl库  
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* Set up the SSL context */

    ctx = SSL_CTX_new(SSLv23_method());

    /* Load the trust store */

    if (!SSL_CTX_load_verify_locations(ctx, "./ca.crt", 0))//读取CA根证书，用这个证书来验证对方的证书是否可信  
    {
        fprintf(stderr, "Error loading trust store\n");
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Setup the connection */

    sslbio = BIO_new_ssl_connect(ctx); //建立ssl类型的bio  

    /* Set the SSL_MODE_AUTO_RETRY flag */

    BIO_get_ssl(sslbio, & ssl); //从已建立的ssl类型的bio sslbio中得到ssl变量  
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY); //设置ssl的模式为SSL_MODE_AUTO_RETRY,使用这个选项进行设置，如果服务器突然希望进行一次新的握手，那么OpenSSL 可以在后台处理它。  


    /* Create and setup the connection */

    BIO_set_conn_hostname(sslbio, "127.0.0.1:4433");

    if (BIO_do_connect(sslbio) <= 0)//发起握手请求  
    {
        fprintf(stderr, "Error attempting to connect\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(sslbio);
        SSL_CTX_free(ctx);
        return 0;
    } else printf("connent to server successful!\n");

    /* Check the certificate */

    if (SSL_get_verify_result(ssl) != X509_V_OK)//验证对方的证书是否是合法的（时间未过期，等等。。。）  
    {
        fprintf(stderr, "Certificate verification error: %ld\n", SSL_get_verify_result(ssl));
        BIO_free_all(sslbio);
        SSL_CTX_free(ctx);
        return 0;
    } else printf("verify server cert successful\n");

    char buf[1024];
    for (;;) {
        printf("\ninput:");
        scanf("%s", &buf[0]);
        printf("buf:[%s]size[%d]\n",buf, strlen(buf));
        BIO_write(sslbio, buf, strlen(buf));
        p = BIO_read(sslbio, buf, 1023);
        if (p <= 0) break;
        buf[p] = 0;
        printf("%s\n", buf);
        memset(buf, 0, 1024);
        p = BIO_read(sslbio, buf, 1023);
        if (p <= 0) break;
        buf[p] = 0;
        printf("%s\n", buf);
        memset(buf, 0, 1024);
    }

    /* Close the connection and free the context */
    BIO_ssl_shutdown(sslbio);
    BIO_free_all(sslbio);
    SSL_CTX_free(ctx);
    return 0;
} 
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CAServer.c
 * Author: chenzifeng
 *
 * Created on 2016年11月15日, 上午11:16
 */

#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>  
#include <unistd.h>  
#include <stdio.h>  
#include <arpa/inet.h>  

#include <openssl/bio.h>  
#include <openssl/ssl.h>  
#include <openssl/err.h>  

#define SERVER_PEM "./server.pem"  
#define SERVER_KRY "./server.key"  

int password_callback(char *buf, int size, int rwflag, void *userdata) {
    /* For the purposes of this demonstration, the password is "ibmdw" */
    printf("*** Callback function called\n");
    strcpy(buf, "123456");
    return strlen(buf);
}

int main() {
    int serv_sock, cli_sock;
    socklen_t client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    SSL_CTX * ctx;
    SSL *ssl;

    int (*callback)(char *, int, int, void *) = &password_callback;

    printf("Serving it up in a secure manner\n\n");
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    ERR_load_SSL_strings();
    OpenSSL_add_all_algorithms();

    printf("Attempting to create SSL context... \n");
    ctx = SSL_CTX_new(SSLv23_server_method());
    if (ctx == NULL) {
        printf("Failed. Aborting.\n");
        return 0;
    }

    printf("\nLoading certificates...\n");
    SSL_CTX_set_default_passwd_cb(ctx, callback);
    if (!SSL_CTX_use_certificate_file(ctx, SERVER_PEM, SSL_FILETYPE_PEM)) {
        ERR_print_errors_fp(stdout);
        SSL_CTX_free(ctx);
        return 0;
    }
    else printf("load server.csr successful!\n");
    if ((SSL_CTX_use_PrivateKey_file(ctx, SERVER_KRY, SSL_FILETYPE_PEM)) <= 0) {
        printf("use private key failed!\n\n");
        ERR_print_errors_fp(stdout);
        SSL_CTX_free(ctx);
        return 0;
    }

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == serv_sock) {
        perror("socket");
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(4433);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(serv_sock, (struct sockaddr*) &server_address, sizeof (struct sockaddr));
    if (-1 == ret) {
        perror("bind");
    }
    listen(serv_sock, 5);
    while (1) {
        cli_sock = accept(serv_sock, (struct sockaddr *) &client_address, (socklen_t *) & client_len);
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, cli_sock);

        SSL_accept(ssl);

        //读取  
        char line[4096];
        SSL_read(ssl, line, 10);
        //正常处理HTTP协议  
        printf("%s\n",line);
        //写入，返回报文。  
        SSL_write(ssl, "HTTP/1.0 200 OK\r\n\r\n", 19);
        SSL_write(ssl, "<html><header></header><body><p1>Hello World<p1></body></html>", 62);
        close(cli_sock);
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    SSL_CTX_free(ctx);
} 
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h> 
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <openssl/ssl.h>  
#include <openssl/bio.h>  
#include <openssl/err.h>  

#include "include/fenglingSem.h"
#include "include/fenglingShm.h"
#include "include/fenglingCA.h"
#include "include/fenglingEpoll.h"

#include "include/fenglingConf.h"
#include "include/fenglingLog.h"

extern conf_global_t g_global_conf;
extern conf_filter_t g_filter_conf;
extern log_t *g_log;

//static dict_t *RouteDict;

int password_callback(char *buf, int size, int rwflag, void *userdata) {
    /* For the purposes of this demonstration, the password is "ibmdw" */
    //    printf("*** Callback function called\n");
    debug(g_log, "in password_callback\n");
    debug(g_log, "*** Callback function called\n");
    strcpy(buf, "123456");
    return strlen(buf);
}

int CAServerInit() {
    int serv_sock, cli_sock;
    socklen_t client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    SSL_CTX * ctx;
    SSL *ssl;
    char g_pkgBody[PKG_HEAD_LEN + 1]; /* 放置交易包头 */
    int g_pkgBodyLen; /* 交易包头长度 */
    char *p;
    char buf[BUF_SIZE + 1];

    if (fork() != 0) {
        return (EXIT_SUCCESS);
    }

    int (*callback)(char *, int, int, void *) = &password_callback;
    debug(g_log, "Serving it up in a secure manner\n\n");

    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    ERR_load_SSL_strings();
    OpenSSL_add_all_algorithms();

    debug(g_log, "Attempting to create SSL context... \n");
    ctx = SSL_CTX_new(SSLv23_server_method());
    if (ctx == NULL) {
        printf("Failed. Aborting.\n");
        return 0;
    }

    debug(g_log, "\nLoading certificates...\n");
    SSL_CTX_set_default_passwd_cb(ctx, callback);
    memset(buf, 0x00, sizeof (buf));
    sprintf(buf, "%s%s", g_global_conf.WorkPath, g_global_conf.Spem);
    if (!SSL_CTX_use_certificate_file(ctx, buf, SSL_FILETYPE_PEM)) {
        ERR_print_errors_fp(stdout);
        SSL_CTX_free(ctx);
        return 0;
    } else printf("load server.csr successful!\n");
    memset(buf, 0x00, sizeof (buf));
    sprintf(buf, "%s%s", g_global_conf.WorkPath, g_global_conf.Skey);
    if ((SSL_CTX_use_PrivateKey_file(ctx, buf, SSL_FILETYPE_PEM)) <= 0) {
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
    server_address.sin_port = htons(g_global_conf.S1port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(serv_sock, (struct sockaddr*) &server_address, sizeof (struct sockaddr));
    if (-1 == ret) {
        perror("bind");
    }
    listen(serv_sock, g_global_conf.S1MaxListen);

    while (1) {
        debug(g_log, "\nin while \n");
        char* Subdomian;
        char* stotal;
        int itotal;

        cli_sock = accept(serv_sock, (struct sockaddr *) &client_address, (socklen_t *) & client_len);
        debug(g_log, "cli_sock:[%d]\n", cli_sock);
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, cli_sock);

        SSL_accept(ssl);

        //读取  
        char line[PKG_BODY_LEN];
        memset(line, 0x00, sizeof (line));
        SSL_read(ssl, line, 6);
        //正常处理HTTP协议  
        if (memcmp(line, "FL", 2) != 0) {
            continue;
        }

        g_pkgBodyLen = 0;
        memset(g_pkgBody, 0x00, sizeof (g_pkgBody));
        memcpy(g_pkgBody, line + 2, PKG_HEAD_LEN - 2);
        g_pkgBodyLen = atoi(g_pkgBody);
        if (g_pkgBodyLen > PKG_BODY_LEN) {
            continue;
        }
        memset(line, 0x00, sizeof (line));
        SSL_read(ssl, line, g_pkgBodyLen);
        // 解析报文设置FLRoute
        Subdomian = strtok(line, "-");
        stotal = strtok(NULL, "-");
        itotal = atoi(stotal);
        SetFLRoute(Subdomian, ssl, itotal);
        //        printf("%s\n", line);
        //        //写入，返回报文。  
        //        SSL_write(ssl, "HTTP/1.0 200 OK\r\n\r\n", 19);
        //        SSL_write(ssl, "<html><header></header><body><p1>Hello World<p1></body></html>", 62);

    }
    close(cli_sock);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);

    return (EXIT_SUCCESS);
}

int CARun() {
    BIO *sslbio;
    SSL * ssl;
    SSL_CTX * ctx;

    int p;
    int currentRec;
    pid_t childPid;
    char buf[BUF_SIZE + 1];

    /* Set up the library */
    SSL_library_init(); //一定要有，初始化ssl库  
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    /* Set up the SSL context */

    ctx = SSL_CTX_new(SSLv23_method());

    /* Load the trust store */
    memset(buf, 0x00, sizeof (buf));
    sprintf(buf, "%s%s", g_global_conf.WorkPath, g_global_conf.Ccrt);
    debug(g_log, "global.Ccrt:[%s]\n", buf);
    if (!SSL_CTX_load_verify_locations(ctx, buf, 0))//读取CA根证书，用这个证书来验证对方的证书是否可信  
    {
        fprintf(stderr, "Error loading trust store\n %s\n", strerror(errno));
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Setup the connection */

    sslbio = BIO_new_ssl_connect(ctx); //建立ssl类型的bio  

    for (;;) {
        if ((currentRec = Cpool_monitor()) < 0) {
            log(g_log, "pool_monitor < 0");
            continue;
        }
        debug(g_log, "currentRec:[%d]\n", currentRec);

        switch ((childPid = fork())) {
            case -1:
                sleep(1);
                continue;
            case 0:
                /* 设置子进程退出的处理函数 */
                signal(SIGTERM, CLD_OUT);
                /* zwm, 20060810, 屏蔽子进程的跟踪*/
                signal(SIGCHLD, SIG_IGN);
                signal(SIGCLD, SIG_IGN);

                /* Set the SSL_MODE_AUTO_RETRY flag */

                BIO_get_ssl(sslbio, & ssl); //从已建立的ssl类型的bio sslbio中得到ssl变量  
                SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY); //设置ssl的模式为SSL_MODE_AUTO_RETRY,使用这个选项进行设置，如果服务器突然希望进行一次新的握手，那么OpenSSL 可以在后台处理它。  


                /* Create and setup the connection */

                BIO_set_conn_hostname(sslbio, g_global_conf.CServerAddr);

                if (BIO_do_connect(sslbio) <= 0)//发起握手请求  
                {
                    fprintf(stderr, "Error attempting to connect\n");
                    ERR_print_errors_fp(stderr);
                    BIO_free_all(sslbio);
                    SSL_CTX_free(ctx);
                    exit(0);
                } else printf("connent to server successful!\n");

                /* Check the certificate */

                if (SSL_get_verify_result(ssl) != X509_V_OK)//验证对方的证书是否是合法的（时间未过期，等等。。。）  
                {
                    fprintf(stderr, "Certificate verification error: %ld\n", SSL_get_verify_result(ssl));
                    BIO_free_all(sslbio);
                    SSL_CTX_free(ctx);
                    return 0;
                } else printf("verify server cert successful\n");

                char buf[PKG_BODY_LEN + 1];
                char toCbuf[PKG_BODY_LEN + 1];
                char Bbuf[30 + 1];
                char *retmsg;
                int g_pkgBodyLen = 0;
                char g_pkgBody[PKG_BODY_LEN + 1];

                memset(Bbuf, 0x00, sizeof (Bbuf));
                sprintf(Bbuf, "%s-%d", g_global_conf.SubDomain, g_global_conf.CMaxPoll);
                memset(buf, 0x00, sizeof (buf));
                sprintf(buf, "FL%08d", (int) strlen(Bbuf));
                BIO_write(sslbio, buf, strlen(buf));
                BIO_write(sslbio, Bbuf, strlen(Bbuf));

                for (;;) {
                    p = BIO_read(sslbio, buf, PKG_HEAD_LEN);
                    if (p <= 0) break;

                    if (memcmp(buf, "FL", 2) != 0) {
                        continue;
                    }
                    g_pkgBodyLen = 0;
                    memset(g_pkgBody, 0x00, sizeof (g_pkgBody));
                    memcpy(g_pkgBody, buf + 2, PKG_HEAD_LEN - 2);
                    g_pkgBodyLen = atoi(g_pkgBody);
                    if (g_pkgBodyLen > PKG_BODY_LEN) {
                        continue;
                    }

                    memset(buf, 0, 1024);
                    p = BIO_read(sslbio, buf, g_pkgBodyLen);

                    //发送到本机服务
                    retmsg = gotoClient(buf);

                    memset(toCbuf, 0x00, sizeof (toCbuf));
                    sprintf(toCbuf, "FL%08d", (int) strlen(retmsg));
                    BIO_write(sslbio, toCbuf, strlen(toCbuf));
                    BIO_write(sslbio, retmsg, strlen(retmsg));
                }

                /* Close the connection and free the context */
                BIO_ssl_shutdown(sslbio);
                BIO_free_all(sslbio);
            default:
                pool_setpid(childPid);
        }
    }
    SSL_CTX_free(ctx);
    return 0;
}
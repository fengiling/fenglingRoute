#include "openssl/bio.h"  
#include "openssl/ssl.h"  
#include "openssl/err.h"  
 
#include <cutil.h>  
 
#define EXIT_IF_TRUE(x) if (x)                              \  
    do {                                                    \  
            fprintf(stderr, "Check '%s' is true\n", #x);    \  
            ERR_print_errors_fp(stderr);                    \  
            exit(2);                                        \  
    }while(0)  
 
int main(int argc, char **argv)  
{  
    SSL_METHOD  *meth;  
    SSL_CTX     *ctx;  
    SSL         *ssl;  
 
    int nFd;  
    int nLen;  
    char szBuffer[1024];  
 
    // 初始化  
    cutil_init();  
    cutil_log_set_level(LOG_ALL);  
    cutil_log_set_stderr(1);  
    SSLeay_add_ssl_algorithms();  
    OpenSSL_add_all_algorithms();  
    SSL_load_error_strings();  
    ERR_load_BIO_strings();  
 
    // 我们使用SSL V3,V2      
    EXIT_IF_TRUE((ctx = SSL_CTX_new (SSLv23_method())) == NULL);  
 
    // 要求校验对方证书  
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);  
 
    // 加载CA的证书  
    EXIT_IF_TRUE (!SSL_CTX_load_verify_locations(ctx, "cacert.cer", NULL));  
 
    // 加载自己的证书  
    EXIT_IF_TRUE (SSL_CTX_use_certificate_file(ctx, "client.cer", SSL_FILETYPE_PEM) <= 0) ;  
 
    // 加载自己的私钥  
    EXIT_IF_TRUE (SSL_CTX_use_PrivateKey_file(ctx, "client.key", SSL_FILETYPE_PEM) <= 0) ;  
 
    // 判定私钥是否正确  
    EXIT_IF_TRUE (!SSL_CTX_check_private_key(ctx));  
      
    // 创建连接  
    nFd = cutil_socket_new(SOCK_STREAM);  
    if(cutil_socket_connect(nFd, "127.0.0.1", 8812, 30) < 0)  
    {  
        cutil_log_error("连接服务器失败\n");  
        return -1;  
    }  
 
    // 将连接付给SSL  
    EXIT_IF_TRUE( (ssl = SSL_new (ctx)) == NULL);  
    SSL_set_fd (ssl, nFd);  
    EXIT_IF_TRUE( SSL_connect (ssl) != 1);  
 
    // 进行操作  
    sprintf(szBuffer, "this is from client %d", getpid());  
    SSL_write(ssl, szBuffer, strlen(szBuffer));  
 
    // 释放资源  
    memset(szBuffer, 0, sizeof(szBuffer));  
    nLen = SSL_read(ssl,szBuffer, sizeof(szBuffer));  
    fprintf(stderr, "Get Len %d %s ok\n", nLen, szBuffer);  
      
    SSL_free (ssl);  
    SSL_CTX_free (ctx);  
    close(nFd);  
}  

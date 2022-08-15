#ifndef _WEB_CLIENT_H_
#define _WEB_CLIENT_H_

#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <wait.h>
#include <time.h>
#include <dirent.h>
#include <stdbool.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include "disk_hash.c"


#define SUCCESS            0
#define ERR_BASE           0
#define ERR_OUT_OF_MEM     ERR_BASE - 1
#define ERR_POINT          ERR_BASE - 2
#define ERR_DIR            ERR_BASE - 1
#define ERR_WRITE          ERR_BASE - 2
#define ERR_OPEN_FILE      ERR_BASE - 3
#define ERR_MY_MALLOC      ERR_BASE - 4
#define ERR_TABLE_SIZE     ERR_BASE - 5
#define ERR_READ_FILE      ERR_BASE - 6
#define ERR_PARAMETER      ERR_BASE - 7
#define ERR_RENAME         ERR_BASE - 8
#define ERR_CLOSE          ERR_BASE - 9
#define ERR_FUNC           ERR_BASE - 10

#define ERR_CHILD          ERR_BASE - 11
#define ERR_SSL_CONNCET    ERR_BASE - 12
#define ERR_URL            ERR_BASE - 13
#define ERR_TODOLIST       ERR_BASE - 14

#define NOT_FOUND          ERR_BASE - 100

//#define MAX_URL_SIZE       (1024)
//#define MAX_HOST_SIZE      (1024)
#define QUEUE_FILE_NAME    "/todolist"
#define WEB_DATA_LIMIT     (4096)
#define FILE_LOCK          "temp_lock"
#define STATE_READY        (0)
#define STATE_RUN          (1)
#define STATE_END          (-1)
#define URL_LIMIT          (8192)
#define CHILD_SLEEP_TIME   (1)
#define PARENT_SLEEP_TIME  (5)


typedef struct _OPENSSL {
   SSL_CTX *ctx;
   SSL *ssl;
   int server;
} _OPENSSL;




int HOST_string_conversion(const char original[], const char new[]);
int to_do_list_url_string_conversion(const char original[], const char new[]);
int create_dir(const char* dir_name);
int create_socket(const char hostname[]);
int openSSL_connect(const char host[], _OPENSSL *SSL);
int openSSL_close(_OPENSSL *SSL);
int parent_func(int process_count, int *child_pid, int *child_state, char *todolist_fname);
void sub_quit_signal_handle(int sig);


#endif
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
#define ERR_STAT           ERR_BASE - 14
#define ERR_OPEN_DIR       ERR_BASE - 15
#define ERR_OUT_OF_MEM     ERR_BASE - 16
#define ERR_POINT          ERR_BASE - 17
#define ERR_OPENSSL        ERR_BASE - 18

#define NOT_FOUND          ERR_BASE - 100
#define REACH_DATA_LIMIT   ERR_BASE - 101
#define TODOLIST_ZERO      ERR_BASE - 102


//#define MAX_URL_SIZE       (1024)
//#define MAX_HOST_SIZE      (1024)
#define QUEUE_FILE_NAME    "/todolist.txt"
#define WEB_DATA_LIMIT     (4096)
#define DIR_TEMP           "temp"
#define DISK_HASH_LOCK     "temp/disk_hash_lock"
#define SHARE_MEM_LOCK     "temp/share_mem_lock"
#define TODOLIST_LOCK      "temp/todolist_lock"
#define DATA_DIR_LOCK      "temp/data_dir_lock"
#define STATE_READY        (0)
#define STATE_RUN          (1)
#define STATE_END          (-1)
#define URL_LIMIT          (0xFFFF)
#define CHILD_SLEEP_TIME   (1)
#define PARENT_SLEEP_TIME  (5)
#define SOCKET_LEN         (0xFFFF)


typedef struct _OPENSSL {
   SSL_CTX *ctx;
   SSL *ssl;
   int server;
} _OPENSSL;

char *host_url = NULL;

int   todolist_fd       = 0;
char  *todolist_fname   = NULL;
char  add_todolist[URL_LIMIT];

int   disk_hash_lock_fd = 0;
int   share_mem_lock_fd = 0;
int   todolist_lock_fd  = 0;
int   data_dir_lock_fd  = 0;

int   *web_data_dir_num    = NULL;
char  *web_data_dir_name   = NULL;


int   *child_state      = NULL;
char  **child_catch_url = NULL;
pid_t *child_pid        = NULL;

int   process_count = 5;
int   process_index = -1;

bool close_header = FALSE;
bool plain_text   = FALSE;

_DISK_HASH dh;
_OPENSSL SSL_server;

int HOST_string_conversion(const char original[], const char new[]);
int to_do_list_url_string_conversion(const char original[], const char new[]);
int create_dir(const char* dir_name);
int create_socket();
int openSSL_connect();
int openSSL_close();
int parent_func();
void sub_quit_signal_handle(int sig);
int child_func();
int get_todolist_url();
int check_dir_num();
int setandsend_request();
int accept_response(int data_file_count);
int open_web_data_file(int data_count, int *data_fd, char *data_fname, int data_len);
int response_head_handle(const char *response, int *body_len, const int data_fd);
int response_body_handle(const int data_fd);
int add_todolist_file(char *add);
int disk_hash_find(const char *find_key);
int URL_percent_encoding(char original[]);

#endif
#include <stdio.h>
#include <stdlib.h>

#define TABLE_NUM          20
#define MIN_ALLOC_SIZE     1 << (TABLE_NUM - 1)

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

#define NOT_FOUND          ERR_BASE - 100
#define REACH_DATA_LIMIT   ERR_BASE - 101
#define TODOLIST_ZERO      ERR_BASE - 102

#define TRUE               1
#define FALSE              0

#ifdef _DEBUG
   #include <time.h>   /* 時間相關函數 */
   #define DEBUG_NUM          100
   #define DEBUG_SIZE         4000
#endif


typedef struct bound{
   void *start;            //8
   void *end;              //8
   struct bound *next;     //8
} bound;

//48 * 2 = 96
typedef struct header{
   struct header *tpre;      //8
   struct header *tnext;     //8
   struct header *hpre;      //8
   struct header *hnext;     //8
   size_t size;              //8
   _Bool using;              //1
} header;


void *my_malloc(size_t num);
int my_free(void *p);

int morecore(size_t num);
void remove_from_table(header *ptr);
void insert_table(header *ptr);
void print_node();
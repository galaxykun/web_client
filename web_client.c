#include "web_client.h"


int main(int argc, char *argv[]){
   int ret_val = 0;

   

   if (argc < 3 || argc > 4){
      printf("parameter error!!\n");
      printf("./web_client “URL” “Output Directory” [“process count”]\n");

      return SUCCESS;
   }

   if(argc == 4){
      process_count = atoi(argv[3]);
   }

   child_pid = (pid_t*)malloc(process_count * sizeof(pid_t));

   host_url = (char*)malloc(strlen(argv[1]) + 1);
   if(!host_url){
      ret_val = ERR_OUT_OF_MEM;
      printf("URL malloc ERROR!\n", ret_val);

      goto ERROR;
   }

   ret_val = HOST_string_conversion(argv[1], host_url);
   if(ret_val){
      ret_val = ERR_URL;
      printf("HOST_string_conversion ERROR!\n", ret_val);

      goto ERROR;
   }

 #ifdef _DEBUG
   printf("HOST URL : %s\n", host_url);
 #endif

   to_do_list_url_string_conversion(argv[1], add_todolist);

 #ifdef _DEBUG
   printf("add to to do list URL : %s\n", add_todolist);
 #endif

   char *output_dir = argv[2];
   int dir_len = strlen(argv[2]);
   if(dir_len > 0 && output_dir[dir_len - 1] == '/'){
      output_dir[dir_len - 1] = '\0';
   }

   if(!strlen(output_dir)){
      output_dir = "webpage";
   }

   ret_val = create_dir(output_dir);
   if(ret_val){
      printf("%s create ERROR!\n", output_dir);

      goto ERROR;
   }

   //"output_dir"+"/0000"
   web_data_dir_name = (char*)malloc(strlen(output_dir) + 6);
   if(!web_data_dir_name){
      printf("web_data_dir_name malloc ERROR!\n");

      goto ERROR;
   }

   char *web_data_dir_ptr = web_data_dir_name;
   strncpy(web_data_dir_ptr, output_dir, strlen(output_dir));
   web_data_dir_ptr += strlen(output_dir);
   strncpy(web_data_dir_ptr, "/", 1);
   web_data_dir_ptr ++;
   sprintf(web_data_dir_ptr, "%d", web_data_dir_num);

 #ifdef _DEBUG
   printf("web data dir name : %s\n", web_data_dir_name);
 #endif

   ret_val = create_dir(web_data_dir_name);
   if(ret_val){
      printf("%s create ERROR!\n", web_data_dir_name);

      goto ERROR;
   }

   
   todolist_fname = (char*)malloc(strlen(output_dir) + 1 + sizeof(QUEUE_FILE_NAME));
   if(!todolist_fname){
      ret_val = ERR_OUT_OF_MEM;
      printf("todolist_fname malloc ERROR!\n");

      goto ERROR;
   }

   strncpy(todolist_fname, output_dir, strlen(output_dir) + 1);
   strcat(todolist_fname, QUEUE_FILE_NAME);

 #ifdef _DEBUG
   printf("todolist file name : %s\n", todolist_fname);
 #endif

   todolist_fd = open(todolist_fname, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0777);
   if(todolist_fd < 0){
      ret_val = errno;
      printf("todolist file open ERROR!\n");

      goto ERROR;
   }

   if(0 >= write(todolist_fd, add_todolist, strlen(add_todolist))){
      printf("write add_todolist to queue ERROR!\n");
      ret_val = errno;

      goto ERROR;
   }

   ret_val = create_dir(DIR_TEMP);
   if(ret_val){
      printf("%s create ERROR!\n", DIR_TEMP);

      goto ERROR;
   }

   disk_hash_lock_fd = open(DISK_HASH_LOCK, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0777);
   if(disk_hash_lock_fd < 0){
      ret_val = errno;
      printf("disk_hash_lock_fd file open ERROR!\n");

      goto ERROR;
   }

   share_mem_lock_fd = open(SHARE_MEM_LOCK, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0777);
   if(share_mem_lock_fd < 0){
      ret_val = errno;
      printf("share_mem_lock_fd file open ERROR!\n");

      goto ERROR;
   }

   child_state = (int*)mmap(NULL, process_count * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   if(child_state == MAP_FAILED){
      printf("child_state mmap ERROR!\n");
      ret_val = errno;

      goto ERROR;
   }
   memset(child_state, STATE_READY, process_count * sizeof(int));

   child_catch_url = (char**)mmap(NULL, process_count * sizeof(char*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   if(child_catch_url == MAP_FAILED){
      printf("child_catch_url mmap ERROR!\n");
      ret_val = errno;

      goto ERROR;
   }
   for(int i = 0; i < process_count; i++){
      child_catch_url[i] = (char*)mmap(NULL, URL_LIMIT * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
      if(child_catch_url[i] == MAP_FAILED){
         printf("child_catch_url[%2d] mmap ERROR!\n", i);
         ret_val = errno;

         goto ERROR;
      }
      child_catch_url[i][URL_LIMIT] = '\0';
   }

 #ifdef _DEBUG
   _OPENSSL SSL;
   ret_val = openSSL_connect(&SSL);

   struct timeval tv;
   tv.tv_sec = 3;
   tv.tv_usec = 0;
   setsockopt(SSL.server, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

   char request[0xfff], response[0xfff]; // 請求 與 回應訊息
   strcpy(request, "GET ");
   add_todolist[strlen(add_todolist) - 1] = '\0';
   strcat(request, add_todolist);
   strcat(request, " HTTP/1.1\r\nHost: ");
   strcat(request, host_url);
   strcat(request, "\r\n");
   strcat(request, "\r\n");

   // 格式化輸出請求訊息
   printf("----------\nRequest:\n----------\n%s\n", request);
   // 發送請求
   int ret = SSL_write(SSL.ssl, request, strlen(request));
   if (ret)
      SSL_get_error(SSL.ssl, ret);


   // 接收回應
   printf("----------\nResponse:\n----------\n");
   ret = 0;
   ret = SSL_read(SSL.ssl, response, (0xfff - 1));
   if(ret <= 0){
      printf("ERROR!!! : num %d\n", ret);
   }
   response[0xfff] = '\0';
   printf("%s\n", response);

   ret_val = openSSL_close(&SSL);
 #endif

   int index = -1;
   for(int i = 0; i < process_count; i++){
      child_pid[i] = fork();
      if(!child_pid[i]) {
         index = i;
         break;
      }

      //fork ERROR
      if(child_pid[i] == -1){
         i--;
      }
   }

   if(index != -1){
      //child do
      child_func(index);
      
      return SUCCESS;
   }
   else{
      //parent do
      ret_val = parent_func();
      if(ret_val){
         printf("parent function ERROR!\n");

         goto ERROR;
      }
   }

   



 ERROR:
   if(ret_val)
      printf("return value : %d\n", ret_val);
   if(host_url)
      free(host_url);
   if(todolist_fname)
      free(todolist_fname);
   if(child_pid)
      free(child_pid);
   if(todolist_fd > 0)
      close(todolist_fd);
   if(disk_hash_lock_fd > 0){
      close(disk_hash_lock_fd);
      remove(DISK_HASH_LOCK);
   }
   if(share_mem_lock_fd > 0){
      close(share_mem_lock_fd);
      remove(SHARE_MEM_LOCK);
   }
   if(rmdir(DIR_TEMP)){
      printf("rmmove dir ERROR! return : %s!\n", strerror(errno));
   }

   if(web_data_dir_name)
      free(web_data_dir_name);
   if(child_state != MAP_FAILED)
      munmap(child_state, process_count * sizeof(int));
   if(child_catch_url){
      for(int i = 0; i < process_count; i++){
         if(child_catch_url[i]){
            munmap(child_catch_url[i], URL_LIMIT * sizeof(char));
         }
      }
      munmap(child_catch_url, process_count * sizeof(char*));
   }


   return ret_val;
}

int HOST_string_conversion(const char original[], const char new[]){
   if(!original || !new){
      return ERR_PARAMETER;
   }

   char *optr = (char*)original;
   char *nptr = (char*)new;

   optr = strstr(original, "://");
   if(!optr){
      return ERR_URL;
   }

   optr += strlen("://");
   for (; *optr != '/'; optr++, nptr++) {
      *nptr = *optr;
   }
   *nptr = '\0';


   return SUCCESS;
}

int to_do_list_url_string_conversion(const char original[], const char new[]){
   if(!original || !new){
      return ERR_PARAMETER;
   }

   char *optr = (char*)original;
   char *nptr = (char*)new;
   memset(nptr, 0, URL_LIMIT);

   optr = strstr(original, "://");
   if(!optr){
      strncpy(nptr, optr, URL_LIMIT - 2);
   }
   else{
      optr += 3;
      for (; *optr != '/'; optr++);
      strncpy(nptr, optr, URL_LIMIT - 2);
   }
   if(!strlen(nptr)){
      nptr[0] = '/';
   }
   nptr[strlen(nptr)] = '\n';


   return SUCCESS;
}

int create_dir(const char* dir_name){
   if(!dir_name){
      return ERR_PARAMETER;
   }

   //dir exist?
   struct stat info;
   if(stat(dir_name, &info)){
      if(errno == ENOENT){
         umask(002);
         if(mkdir(dir_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)){ //775
          #ifdef _DEBUG
            printf("%s\n", strerror(errno));
          #endif

            return errno;
         }
      }
      else{
       #ifdef _DEBUG
         printf("%s\n", strerror(errno));
       #endif

         return errno;
      }
   }
   //is dir?
   else if(!S_ISDIR(info.st_mode)){
    #ifdef _DEBUG
      printf("%s not dir!\n", dir_name);
    #endif

      return errno;
   }


   return SUCCESS;
}

int openSSL_connect(_OPENSSL *SSL){
   if(!SSL){
      return ERR_PARAMETER;
   }

   const SSL_METHOD *method;
   int ret, i;

   /* ---------------------------------------------------------- *
   * These function calls initialize openssl for correct work.  *
   * ---------------------------------------------------------- */
   OpenSSL_add_all_algorithms();
   ERR_load_BIO_strings();
   ERR_load_crypto_strings();
   SSL_load_error_strings();

   /* ---------------------------------------------------------- *
   * initialize SSL library and register algorithms             *
   * ---------------------------------------------------------- */
   if(SSL_library_init() < 0){
    #ifdef _DEBUG
      printf("Could not initialize the OpenSSL library !\n");
    #endif

      return ERR_SSL_CONNCET;
   }

   /* ---------------------------------------------------------- *
   * SSLv23_client_method is deprecated function                *
   * Set TLS client hello by andric                             *
   * ---------------------------------------------------------- */
   method = TLS_client_method();

   /* ---------------------------------------------------------- *
   * Try to create a new SSL context                            *
   * ---------------------------------------------------------- */
   if ( (SSL->ctx = SSL_CTX_new(method)) == NULL) {
    #ifdef _DEBUG
      printf("Unable to create a new SSL context structure.\n");
    #endif

      return ERR_SSL_CONNCET;
   }

   /* ---------------------------------------------------------- *
   * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
   * ---------------------------------------------------------- */
   SSL_CTX_set_options(SSL->ctx, SSL_OP_NO_SSLv2);

   /* ---------------------------------------------------------- *
   * Create new SSL connection state object                     *
   * ---------------------------------------------------------- */
   SSL->ssl = SSL_new(SSL->ctx);

   /* ---------------------------------------------------------- *
   * Make the underlying TCP socket connection                  *
   * ---------------------------------------------------------- */
   SSL->server = create_socket();
   if(SSL->server != 0) {
    #ifdef _DEBUG
      printf("Successfully made the TCP connection to: %s .\n", host_url);
    #endif
   }

   /* ---------------------------------------------------------- *
   * Attach the SSL session to the socket descriptor            *
   * ---------------------------------------------------------- */
   SSL_set_fd(SSL->ssl, SSL->server);

   /* ---------------------------------------------------------- *
   * Set to Support TLS SNI extension by Andric                 *
   * ---------------------------------------------------------- */
   SSL_ctrl(SSL->ssl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void*)host_url);

   /* ---------------------------------------------------------- *
   * Try to SSL-connect here, returns 1 for success             *
   * ---------------------------------------------------------- */
   if ((ret = SSL_connect(SSL->ssl)) != 1 ) {
      int err;
    #ifdef _DEBUG
      printf("Error: Could not build a SSL session to: %s .\n", host_url);
    #endif

      return ERR_SSL_CONNCET;

      /* ---------------------------------------------------------- *
      * Print SSL-connect error message by andric                  *
      * ---------------------------------------------------------- */
      err = SSL_get_error(SSL->ssl, ret);
      if(err == SSL_ERROR_SSL) {
         ERR_load_crypto_strings();
         SSL_load_error_strings(); // just once
         char msg[1024];
         ERR_error_string_n(ERR_get_error(), msg, sizeof(msg));
         printf("%s %s %s %s\n", msg, ERR_lib_error_string(0), ERR_func_error_string(0), ERR_reason_error_string(0));
      }

      return ERR_SSL_CONNCET;
   }
   else {
    #ifdef _DEBUG
      printf("Successfully enabled SSL/TLS session to: %s .\n", host_url);
    #endif
   }


   return SUCCESS;
}

int create_socket(){
   /* if(!host_url){
      return ERR_PARAMETER;
   } */

   int sockfd;
   char portnum[6] = "443";
   char *tmp_ptr = NULL;
   int  port;
   struct hostent *host;
   struct sockaddr_in dest_addr;

   /* ---------------------------------------------------------- *
   * if the host_url contains a colon :, we got a port number   *
   * ---------------------------------------------------------- */
   if (tmp_ptr = strchr(host_url, ':')) {
      // strchr(host_url, ':');
      /* the last : starts the port number, if avail, i.e. 8443 */
      strncpy(portnum, tmp_ptr+1,  sizeof(portnum));
      *tmp_ptr = '\0';
   }

   port = atoi(portnum);

   if ((host = gethostbyname(host_url)) == NULL) {
      #ifdef _DEBUG
      printf("Error: Cannot resolve host_url %s .\n", host_url);
      #endif

      abort();
   }

   /* ---------------------------------------------------------- *
   * create the basic TCP socket                                *
   * ---------------------------------------------------------- */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   dest_addr.sin_family=AF_INET;
   dest_addr.sin_port=htons(port);
   dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);

   /* ---------------------------------------------------------- *
   * Zeroing the rest of the struct                             *
   * ---------------------------------------------------------- */
   memset(&(dest_addr.sin_zero), '\0', 8);

   tmp_ptr = inet_ntoa(dest_addr.sin_addr);

   /* ---------------------------------------------------------- *
   * Try to make the host connect here                          *
   * ---------------------------------------------------------- */
   if(connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr)) == -1) {
      #ifdef _DEBUG
      printf("Error: Cannot connect to host %s [%s] on port %d.\n", host_url, tmp_ptr, port);
      #endif
   }


   return sockfd;
}

int openSSL_close(_OPENSSL *SSL){
   if(!SSL){
      return ERR_PARAMETER;
   }

  /* ---------------------------------------------------------- *
   * Free the structures we don't need anymore                  *
   * -----------------------------------------------------------*/
   SSL_free(SSL->ssl);
   close(SSL->server);
   SSL_CTX_free(SSL->ctx);
   memset(SSL, 0, sizeof(_OPENSSL));

   return SUCCESS;
}

void sub_quit_signal_handle(int sig){
   int status = 0;

   for(int i = 0; i < process_count; i++){
      if(child_pid[i] != -1 && waitpid(child_pid[i], &status, WNOHANG)){
         printf("pid : %5d is end, status : %3d, index : %2d\n", child_pid[i], status, i);
         if(!status){
            child_pid[i] = -1;
         }
         else{
            child_pid[i] = fork();
         }

         if(child_pid[i] == 0){
            child_func(i);

            return;
         }
      }
   }


   return;
}

int parent_func(){
   signal(SIGCHLD, sub_quit_signal_handle);

   while(1){
    #ifdef _DEBUG
      printf("parent sleep...\n");
    #endif
   
      sleep(PARENT_SLEEP_TIME);

      bool end = TRUE;

      lockf(share_mem_lock_fd, F_LOCK, 0);
      for(int i = 0; i < process_count; i++){
         if(child_state[i] != STATE_READY){
            end = FALSE;

            break;
         }
      }
      
      if(end){
         struct stat info;
         if(stat(todolist_fname, &info)){
            printf("parent stat ERROR! : %s %d\n", strerror(errno), errno);

            return ERR_STAT;
         }

         if(!info.st_size){
            for(int i = 0; i < process_count; i++){
               child_state[i] = STATE_END;  
            }
         }
      }
      lockf(share_mem_lock_fd, F_ULOCK, 0);

      if(end){
         break;
      }
   }
 #ifdef _DEBUG
   printf("parent end!\n");
 #endif

   bool wait = TRUE;
   while(wait){
      sleep(PARENT_SLEEP_TIME);

    #ifdef _DEBUG
      printf("wait child...\n");
    #endif

      wait = FALSE;
      for(int i = 0; i < process_count; i++){
         if(child_pid[i] != -1)
            wait = TRUE;
      }
   }



   return SUCCESS;
}

int child_func(int index){
   int ret_val = SUCCESS;
   int data_file_count = 0;
   _DISK_HASH dh;
   
   prctl(PR_SET_PDEATHSIG, SIGTERM);

   _OPENSSL SSL;
   ret_val = openSSL_connect(&SSL);
   if(ret_val){
      printf("child pid : %5d, openssl connect ERROR! return : %2d\n", getpid(), ret_val);

      exit(ret_val);
   }

   
   while(1){
      sleep(CHILD_SLEEP_TIME);

      lockf(share_mem_lock_fd, F_LOCK, 0);
      if(child_state[index] == STATE_END){
         lockf(share_mem_lock_fd, F_ULOCK, 0);

         break;
      }
      lockf(share_mem_lock_fd, F_ULOCK, 0);
      
      child_state[index] = STATE_RUN;
      
      if(get_todolist_url(&dh)){
         child_state[index] = STATE_READY;

         continue;
      }

      if(ret_val = check_dir_num()){
         if(ret_val == REACH_DATA_LIMIT){
            char *temp = web_data_dir_name;
            for(; *temp != '/'; temp++);
            sprintf(++temp, "%d", ++web_data_dir_num);

            ret_val = SUCCESS;
            ret_val = create_dir(web_data_dir_name);
            if(ret_val){
               printf("%s create ERROR!\n", DIR_TEMP);
               sprintf(temp, "%d", web_data_dir_num - 1);
               ret_val = SUCCESS;

               continue;
            }
         }
         else{
            printf("check dir ERROR! return : %d .\n", ret_val);

            continue;
         }
      }

    #ifdef _DEBUG
      printf("web data dir name : %s\n", web_data_dir_name);
    #endif

      char request[SOCKET_LEN], response[SOCKET_LEN]; // 請求 與 回應訊息
      setandsend_request(&SSL, request);
int temp;
open_web_data_file(index, data_file_count, &temp);
      accept_response(&SSL, response);


    #ifdef _DEBUG
      child_state[index] = STATE_READY;
    #endif


   }



   printf("child end!\n");
   return SUCCESS;
}

int get_todolist_url(_DISK_HASH *dh){
   int ret_val = SUCCESS;
   char *todolist_buf      = NULL;
   char *todolist_buf_ptr  = NULL;

   lockf(todolist_fd, F_LOCK, 0);
   struct stat info;
   if(stat(todolist_fname, &info)){
      printf("to do list file ERROR! retrun : %2d, %s .\n", errno, strerror(errno));

      exit(errno);
   }

   if(!info.st_size){
    #ifdef _DEBUG
      printf("to do list file size is 0!\n");
    #endif

      goto ERROR;
   }
   else{
      todolist_buf = (char*)malloc(info.st_size * sizeof(char));
      if(!todolist_buf){
         printf("to do list buffer malloc ERROR! return : %2d, %s .\n", errno, strerror(errno));

         goto ERROR;
      }

      lseek(todolist_fd, SEEK_SET, 0);
      if(info.st_size != read(todolist_fd, todolist_buf, info.st_size)){
         printf("to do list read ERROR! return : %2d, %s .\n", errno, strerror(errno));

         goto ERROR;
      }
      todolist_buf_ptr = todolist_buf;

      bool find_url = TRUE;
      while(find_url){
         char *temp_chr = strchr(todolist_buf_ptr, '\n');
         if(!temp_chr){
            todolist_buf_ptr = NULL;

            break;
         }

         *temp_chr = '\0';
         strncpy(add_todolist, todolist_buf_ptr, strlen(todolist_buf_ptr) + 1);
         todolist_buf_ptr = temp_chr + 1;
         if(todolist_buf_ptr > todolist_buf + info.st_size){
            todolist_buf_ptr = NULL;

            break;
         }

         lockf(disk_hash_lock_fd, F_LOCK, 0);
         if(ret_val = open_table(dh)){
            printf("open disk hash table ERROR! return : %d\n", ret_val);

            goto ERROR;
         }
         ret_val = find(add_todolist, dh, NULL);
         if(ret_val == NOT_FOUND){
            find_url = FALSE;
         }
         else if(ret_val){
            printf("disk hash find ERROR! return : %d\n", ret_val);

            lockf(disk_hash_lock_fd, F_ULOCK, 0);
            lockf(todolist_fd, F_ULOCK, 0);

            exit(ret_val);
         }
         else{
            printf("URL : %s EXIST!\n", add_todolist);
         }
         if(ret_val = close_table(dh)){
            printf("close disk hash ERROR! return : %d\n", ret_val);

            goto ERROR;
         }
         lockf(disk_hash_lock_fd, F_ULOCK, 0);
      }

      if(!todolist_buf_ptr){
         ftruncate(todolist_fd, 0);

         goto ERROR;
      }

      lseek(todolist_fd, SEEK_SET, 0);
      if(strlen(todolist_buf_ptr) != write(todolist_fd, todolist_buf_ptr, strlen(todolist_buf_ptr))){
         printf("to do list write ERROR! return : %2d, %s .\n", errno, strerror(errno));

         goto ERROR;
      }
      lockf(todolist_fd, F_ULOCK, 0);
   }

   lockf(disk_hash_lock_fd, F_LOCK, 0);
   if(ret_val = open_table(dh)){
      printf("open disk hash table ERROR! return : %d\n", ret_val);

      goto ERROR;
   }
   ret_val = add(add_todolist, host_url, strlen(host_url) + 1, TYPE_CHAR, dh);
   if(ret_val){
      printf("disk hash add ERROR! return : %d\n", ret_val);
      lockf(disk_hash_lock_fd, F_ULOCK, 0);

      exit(ret_val);
   }
   if(ret_val = close_table(dh)){
      printf("close disk hash ERROR! return : %d\n", ret_val);

      goto ERROR;
   }
   lockf(disk_hash_lock_fd, F_ULOCK, 0);


 ERROR:
   lockf(todolist_fd, F_ULOCK, 0);
   lockf(disk_hash_lock_fd, F_ULOCK, 0);


   return ret_val;
}

int check_dir_num(){
   DIR * dir = opendir(web_data_dir_name);
   if(!dir){
      printf("open dir %s ERROR! return : %s", web_data_dir_name, strerror(errno));

      return ERR_OPEN_DIR;
   }
   struct dirent * ptr;
   int count = 0;

   while((ptr = readdir(dir)) != NULL){
      count++;
   }

   closedir(dir);

 #ifdef _DEBUG
   printf("%s data total number : %d .\n", web_data_dir_name, count);
 #endif

//return REACH_DATA_LIMIT;
   return count > WEB_DATA_LIMIT ? REACH_DATA_LIMIT : SUCCESS;
}

int open_web_data_file(int index, int data_count, int *data_fd){
   int   data_len = strlen(web_data_dir_name) + 20;
   char  data_fname[data_len];
   char  *data_ptr = data_fname;

   strncpy(data_ptr, web_data_dir_name, data_len);
   data_ptr += strlen(data_ptr);

   strncpy(data_ptr, "/", 1);
   data_ptr++;

   sprintf(data_ptr, "%d", index);
   data_ptr += strlen(data_ptr);

   strncpy(data_ptr, "_", 1);
   data_ptr++;

   sprintf(data_ptr, "%d", data_count);

 #ifdef _DEBUG
   printf("data file name : %s .\n", data_fname);
 #endif

   *data_fd = open(data_fname, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0777);
   if(*data_fd < 0){
      printf("web data file open ERROR! return : %s .\n", strerror(errno));

      return ERR_OPEN_FILE;
   }

   return SUCCESS;
}

int setandsend_request(_OPENSSL *SSL, char *request){
   char  *req_ptr    = request;
   int   req_limit   = SOCKET_LEN;
   int   ret         = SUCCESS;

   memset(request, 0, SOCKET_LEN);

   strncpy(req_ptr, "GET ", req_limit);
   req_ptr     += sizeof("GET ") - 1;
   req_limit   -= sizeof("GET ") - 1;

   strncpy(req_ptr, add_todolist, req_limit);
   req_ptr     += strlen(add_todolist) - 1;
   req_limit   -= strlen(add_todolist) - 1;

   strncat(req_ptr, " HTTP/1.1\r\nHost: ", req_limit);
   req_ptr     += sizeof(" HTTP/1.1\r\nHost: ") - 1;
   req_limit   -= sizeof(" HTTP/1.1\r\nHost: ") - 1;

   strncpy(req_ptr, host_url, req_limit);
   req_ptr     += strlen(host_url);
   req_limit   -= strlen(host_url);

   strncat(req_ptr, "\r\n", req_limit);
   req_ptr     += sizeof("\r\n") - 1;
   req_limit   -= sizeof("\r\n") - 1;

   //strcat(req_ptr, "Connection: close\r\n");
   //strcat(req_ptr, "Content-Type: text/html; charset=UTF-8\r\n");
   //strcat(req_ptr, "Accept-Encoding: chunked\r\n");
   //strcat(req_ptr, "User-Agent: Mozilla/5.0\r\n");
   //strcat(req_ptr, "Transfer-Encoding: chunked\r\n");

   strncat(req_ptr, "\r\n", req_limit);
   req_ptr     += sizeof("\r\n") - 1;
   req_limit   -= sizeof("\r\n") - 1;

 #ifdef _DEBUG
   printf("\n********** Request: **********\n%s\n", request);
   printf(  "******************************\n\n");
 #endif

   ret = SSL_write(SSL->ssl, request, strlen(request));
   if(ret <= 0){
      int ssl_get_ret = SSL_get_error(SSL->ssl, ret);

      /* if(ssl_get_ret == SSL_ERROR_NONE){
         printf("openssl write ERROR! return : SSL_ERROR_NONE\n");
      }
      else if(ssl_get_ret == SSL_ERROR_ZERO_RETURN){
         printf("openssl write ERROR! return : SSL_ERROR_ZERO_RETURN\n");
      }
      else if(ssl_get_ret == SSL_ERROR_WANT_READ || ssl_get_ret == SSL_ERROR_WANT_WRITE){
         printf("openssl write ERROR! return : SSL_ERROR_WANT_READ || ssl_get_ret == SSL_ERROR_WANT_WRITE\n");
      }
      else{
         printf("openssl write ERROR! return : ELSE\n");
      } */
      switch(ret){
         case SSL_ERROR_NONE:
            printf("openssl write ERROR! return : SSL_ERROR_NONE\n");
            break;
         case SSL_ERROR_ZERO_RETURN:
            printf("openssl write ERROR! return : SSL_ERROR_ZERO_RETURN\n");
            break;
         case SSL_ERROR_WANT_WRITE:
            printf("openssl write ERROR! return : SSL_ERROR_WANT_READ\n");
            break;
         case SSL_ERROR_WANT_CONNECT:
            printf("openssl write ERROR! return : SSL_ERROR_WANT_CONNECT\n");
            break;
         case SSL_ERROR_WANT_ACCEPT:
            printf("openssl write ERROR! return : SSL_ERROR_WANT_ACCEPT\n");
            break;
         case SSL_ERROR_WANT_X509_LOOKUP:
            printf("openssl write ERROR! return : SSL_ERROR_WANT_X509_LOOKUP\n");
            break;
         case SSL_ERROR_SYSCALL:
            printf("openssl write ERROR! return : SSL_ERROR_SYSCALL\n");
            break;
         case SSL_ERROR_SSL:
            printf("openssl write ERROR! return : SSL_ERROR_SSL\n");
            break;
         default :
            printf("openssl write ERROR! return : ELSE\n");
            break;
      }
   }


   return SUCCESS;
}

int accept_response(_OPENSSL *SSL, char *response){
 #ifdef _DEBUG
   printf("\n********** Response: **********\n");
 #endif

   while(1){
      int ret = SUCCESS;
      memset(response, 0, SOCKET_LEN);

      ret = SSL_read(SSL->ssl, response, SOCKET_LEN);
      if(ret <= 0){
         int ssl_get_ret = SSL_get_error(SSL->ssl, ret);

         switch(ret){
            case SSL_ERROR_NONE:
               printf("openssl read ERROR! return : SSL_ERROR_NONE\n");
               break;
            case SSL_ERROR_ZERO_RETURN:
               printf("openssl read ERROR! return : SSL_ERROR_ZERO_RETURN\n");
               break;
            case SSL_ERROR_WANT_READ:
               printf("openssl read ERROR! return : SSL_ERROR_WANT_READ\n");
               break;
            case SSL_ERROR_WANT_CONNECT:
               printf("openssl read ERROR! return : SSL_ERROR_WANT_CONNECT\n");
               break;
            case SSL_ERROR_WANT_ACCEPT:
               printf("openssl read ERROR! return : SSL_ERROR_WANT_ACCEPT\n");
               break;
            case SSL_ERROR_WANT_X509_LOOKUP:
               printf("openssl read ERROR! return : SSL_ERROR_WANT_X509_LOOKUP\n");
               break;
            case SSL_ERROR_SYSCALL:
               printf("openssl read ERROR! return : SSL_ERROR_SYSCALL\n");
               break;
            case SSL_ERROR_SSL:
               printf("openssl read ERROR! return : SSL_ERROR_SSL\n");
               break;
            default :
               printf("openssl read ERROR! return : ELSE\n");
               break;
         }
      }

       #ifdef _DEBUG
         //printf("%s\n", response);
         break;
       #endif


   }

 #ifdef _DEBUG
   printf(  "******************************\n\n");
 #endif

   return SUCCESS;
}



//printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

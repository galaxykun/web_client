#include "web_client.h"

int main(int argc, char *argv[]){
   int ret_val = 0;

   if (argc < 3 || argc > 4){
      printf("parameter error!!\n");
      printf("./web_client “URL” “Output Directory” [“process count”]\n");

      return ERR_PARAMETER;
   }

   if(argc == 4){
      process_count = atoi(argv[3]);
   }

   child_pid = (pid_t*)malloc(process_count * sizeof(pid_t));
   if(!child_pid){
      ret_val = ERR_OUT_OF_MEM;
      printf("child pid malloc ERROR! return : %d\n", ret_val);

      goto ERROR;
   }

   host_url = (char*)malloc(strlen(argv[1]) + 1);
   if(!host_url){
      ret_val = ERR_OUT_OF_MEM;
      printf("URL malloc ERROR! return : %d\n", ret_val);

      goto ERROR;
   }

   ret_val = HOST_string_conversion(argv[1], host_url);
   if(ret_val){
      ret_val = ERR_URL;
      printf("URL ERROR! Please use https .\n", ret_val);

    #ifdef _DEBUG
      printf("HOST_string_conversion ERROR! return : %d\n", ret_val);
    #endif

      goto ERROR;
   }

 #ifdef _DEBUG
   printf("HOST URL : %s\n", host_url);
 #endif

   if(ret_val = to_do_list_url_string_conversion(argv[1], add_todolist)){
      printf("to_do_list_url_string_conversion ERROR! return : %s", ret_val);

      goto ERROR;
   }

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

   web_data_dir_num = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   if(web_data_dir_num == MAP_FAILED){
      printf("web_data_dir_num mmap ERROR!\n");
      ret_val = errno;

      goto ERROR;
   }
   *web_data_dir_num = 0;

   char *web_data_dir_ptr = web_data_dir_name;
   strncpy(web_data_dir_ptr, output_dir, strlen(output_dir));
   web_data_dir_ptr += strlen(output_dir);
   strncpy(web_data_dir_ptr, "/", 1);
   web_data_dir_ptr ++;
   sprintf(web_data_dir_ptr, "%d", (*web_data_dir_num));

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

   todolist_fd = open(todolist_fname, O_RDWR | O_CREAT | O_SYNC | O_TRUNC, 0777);
   if(todolist_fd < 0){
      ret_val = errno;
      printf("todolist file open ERROR!\n");

      goto ERROR;
   }
 
   ret_val = create_dir(DIR_TEMP);
   if(ret_val){
      printf("%s create ERROR!\n", DIR_TEMP);

      goto ERROR;
   }
 
   disk_hash_lock_fd = open(DISK_HASH_LOCK, O_RDWR | O_CREAT | O_TRUNC, 0777);
   if(disk_hash_lock_fd < 0){
      ret_val = errno;
      printf("disk_hash_lock_fd file open ERROR!\n");

      goto ERROR;
   }
 
   share_mem_lock_fd = open(SHARE_MEM_LOCK, O_RDWR | O_CREAT | O_TRUNC, 0777);
   if(share_mem_lock_fd < 0){
      ret_val = errno;
      printf("share_mem_lock_fd file open ERROR!\n");

      goto ERROR;
   }

   todolist_lock_fd = open(TODOLIST_LOCK, O_RDWR | O_CREAT | O_TRUNC, 0777);
   if(todolist_lock_fd < 0){
      ret_val = errno;
      printf("todolist_lock_fd file open ERROR!\n");

      goto ERROR;
   }

   data_dir_lock_fd = open(DATA_DIR_LOCK, O_RDWR | O_CREAT | O_TRUNC, 0777);
   if(data_dir_lock_fd < 0){
      ret_val = errno;
      printf("data_dir_lock_fd file open ERROR!\n");

      goto ERROR;
   }

   ret_val = disk_hash_find(add_todolist);
   if(ret_val == NOT_FOUND){
      ret_val = add_todolist_file(add_todolist);
      if(ret_val){
         printf("add_todolist_file ERROR! return : %d .\n", ret_val);

         goto ERROR;
      }
      ret_val = SUCCESS;
   }
   else if(ret_val){
      printf("main find disk hash ERROR! return : %d .\n", ret_val);

      goto ERROR;
   }
   else{
      printf("main find : %s .\n", argv[1]);

      goto ERROR;
   }

   child_state = (int*)mmap(NULL, process_count * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   if(child_state == MAP_FAILED){
      printf("child_state mmap ERROR!\n");
      ret_val = errno;

      goto ERROR;
   }
   for(int i = 0; i < process_count; i++){
      child_state[i] = STATE_RUN;
   }

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

   for(int i = 0; i < process_count; i++){
      child_pid[i] = fork();
      if(!child_pid[i]) {
         process_index = i;
         break;
      }

      //fork ERROR
      if(child_pid[i] == -1){
         i--;
      }
   }

   if(process_index != -1){
      //child do
    #ifdef _DEBUG
      printf("child_func run!\n");
    #endif

      child_func();

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
   if(todolist_lock_fd > 0){
      close(todolist_lock_fd);
      remove(TODOLIST_LOCK);
   }
   if(data_dir_lock_fd > 0){
      close(data_dir_lock_fd);
      remove(DATA_DIR_LOCK);
   }
   if(rmdir(DIR_TEMP)){
      printf("rmmove temp dir ERROR! return : %s!\n", strerror(errno));
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

   optr = strstr(original, "https://");
   if(!optr){
      return ERR_URL;
   }

   optr += strlen("https://");
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
   nptr[strlen(nptr)] = '\0';


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
       #ifdef _DEBUG
         printf("pid : %5d is end, status : %3d, process_index : %2d\n", child_pid[i], status, i);
       #endif

         if(!status){
            child_pid[i] = -1;
         }
         else if(!*child_catch_url[i]){
            child_pid[i] = fork();
         }
         else{
            lockf(disk_hash_lock_fd, F_LOCK, 0);
            int ret = 0;
            if(ret = open_table(&dh)){
               printf("open disk hash table ERROR! return : %d\n", ret);
               i--;
               lockf(disk_hash_lock_fd, F_ULOCK, 0);

               continue;
            }

            ret = del(child_catch_url[i], &dh);
            if(ret && ret != NOT_FOUND){
               printf("del disk hash ERROR! return : %d\n", ret);
               i--;
               lockf(disk_hash_lock_fd, F_ULOCK, 0);

               continue;
            }

            if(ret = close_table(&dh)){
               printf("close disk hash ERROR! return : %d\n", ret);
               i--;
               lockf(disk_hash_lock_fd, F_ULOCK, 0);

               continue;
            }
            lockf(disk_hash_lock_fd, F_ULOCK, 0);

            if(ret = add_todolist_file(child_catch_url[i])){
               printf("sub_quit_signal_handle add_todolist_file ERROR! return : %d\n", ret);
               i--;

               continue;
            }

            lockf(share_mem_lock_fd, F_LOCK, 0);
            child_state[i] = STATE_READY;
            lockf(share_mem_lock_fd, F_ULOCK, 0);

            child_pid[i] = fork();
         }

         if(child_pid[i] == 0){
            process_index = i;
            child_func();

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
         if(child_state[i] == STATE_RUN){
            end = FALSE;
            printf("child : %2d is not ready!\n", i);
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
         else{
            end = FALSE;

          #ifdef _DEBUG
            end = TRUE;
          #endif
         }
      }
      lockf(share_mem_lock_fd, F_ULOCK, 0);

      if(end){
         break;
      }
   }

 #ifdef _DEBUG
   printf("parent end! pid : %d .\n", getpid());
 #endif

   bool wait = TRUE;
   while(wait){
    #ifdef _DEBUG
      printf("wait child...\n");
    #endif

      sleep(PARENT_SLEEP_TIME);
      wait = FALSE;
      for(int i = 0; i < process_count; i++){
         if(child_pid[i] != -1)
            wait = TRUE;
      }
   }



   return SUCCESS;
}

int child_func(){
   // lockf(share_mem_lock_fd, F_LOCK, 0);
   // child_state[process_index] = STATE_RUN;
   // lockf(share_mem_lock_fd, F_ULOCK, 0);

   memset(add_todolist, 0, URL_LIMIT);
   int ret_val = SUCCESS;
   int data_file_count = 0;

   prctl(PR_SET_PDEATHSIG, SIGTERM);

   _OPENSSL SSL;
   ret_val = openSSL_connect(&SSL);
   if(ret_val){
      printf("child pid : %5d, openssl connect ERROR! return : %2d\n", getpid(), ret_val);

      exit(ret_val);
   }

    #ifdef _DEBUG
      int count = 0;
    #endif

   while(1){
    #ifdef _DEBUG
      if(count++ == 1){
         lockf(share_mem_lock_fd, F_LOCK, 0);
         child_state[process_index] = STATE_READY;
         lockf(share_mem_lock_fd, F_ULOCK, 0);
         break;
      }
      //getchar();
      //printf("GO~~~~~~~~!!!!!!!!!!!!!!!!!\n");
      // lockf(share_mem_lock_fd, F_LOCK, 0);
      // lockf(share_mem_lock_fd, F_ULOCK, 0);
    #endif

      lockf(share_mem_lock_fd, F_LOCK, 0);
      if(child_state[process_index] == STATE_END){
         lockf(share_mem_lock_fd, F_ULOCK, 0);

         break;
      }

      child_state[process_index] = STATE_RUN;
      lockf(share_mem_lock_fd, F_ULOCK, 0);

      memset(add_todolist, 0, URL_LIMIT);

      if(get_todolist_url()){
         lockf(share_mem_lock_fd, F_LOCK, 0);
         child_state[process_index] = STATE_READY;
         lockf(share_mem_lock_fd, F_ULOCK, 0);

       #ifdef _DEBUG
         if(count > 0) count--;
       #endif

         continue;
      }
      strncpy(child_catch_url[process_index], add_todolist, URL_LIMIT);
      child_catch_url[process_index][URL_LIMIT - 1] = 0;

    #ifdef _DEBUG
      printf("request URL : %s .\n", add_todolist);
    #endif

      lockf(data_dir_lock_fd, F_LOCK, 0);
      if(ret_val = check_dir_num()){
         if(ret_val == REACH_DATA_LIMIT){
            char *temp = strrchr(web_data_dir_name, '/');
            //for(; *temp != '/'; temp++);
            sprintf(++temp, "%d", ++(*web_data_dir_num));

            ret_val = SUCCESS;
            ret_val = create_dir(web_data_dir_name);
            if(ret_val){
               printf("%s create ERROR!\n", DIR_TEMP);
               sprintf(temp, "%d", --(*web_data_dir_num));
               ret_val = SUCCESS;

               lockf(share_mem_lock_fd, F_LOCK, 0);
               child_state[process_index] = STATE_READY;
               lockf(share_mem_lock_fd, F_ULOCK, 0);

               lockf(data_dir_lock_fd, F_ULOCK, 0);
               continue;
            }
         }
         else{
            printf("check dir ERROR! return : %d .\n", ret_val);

            lockf(share_mem_lock_fd, F_LOCK, 0);
            child_state[process_index] = STATE_READY;
            lockf(share_mem_lock_fd, F_ULOCK, 0);

            lockf(data_dir_lock_fd, F_ULOCK, 0);
            continue;
         }
      }
      lockf(data_dir_lock_fd, F_ULOCK, 0);

    #ifdef _DEBUG
      printf("web data dir name : %s\n", web_data_dir_name);
    #endif

      char request[SOCKET_LEN], response[SOCKET_LEN]; // 請求 與 回應訊息
      setandsend_request(&SSL, request);
      accept_response(&SSL, response, data_file_count++);

      // lockf(share_mem_lock_fd, F_LOCK, 0);
      // child_state[process_index] = STATE_READY;
      // lockf(share_mem_lock_fd, F_ULOCK, 0);

      sleep(CHILD_SLEEP_TIME);

    #ifdef _DEBUG
      printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    #endif
   }

 #ifdef _DEBUG
   printf("child end!\n");
 #endif


   return SUCCESS;
}

int get_todolist_url(){
   int ret_val = SUCCESS;
   char *todolist_buf      = NULL;
   char *todolist_buf_ptr  = NULL;

   lockf(todolist_lock_fd, F_LOCK, 0);
   struct stat info;
   if(stat(todolist_fname, &info)){
      printf("to do list file ERROR! retrun : %2d, %s .\n", errno, strerror(errno));
      lockf(todolist_lock_fd, F_ULOCK, 0);

      exit(errno);
   }

   if(!info.st_size){
    #ifdef _DEBUG
      //printf("to do list file size is 0!\n");
    #endif

      ret_val = TODOLIST_ZERO;

      goto ERROR;
   }
   else{
      todolist_buf = (char*)malloc(info.st_size + 1 * sizeof(char));
      if(!todolist_buf){
         printf("to do list buffer malloc ERROR! return : %2d, %s .\n", errno, strerror(errno));

         goto ERROR;
      }
      todolist_buf[info.st_size] = 0;

      lseek(todolist_fd, 0, SEEK_SET);
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
         strncpy(add_todolist, todolist_buf_ptr, strlen(todolist_buf_ptr) < (URL_LIMIT - 1) ? strlen(todolist_buf_ptr) : (URL_LIMIT - 1));
         todolist_buf_ptr = ++temp_chr;

         ret_val = disk_hash_find(add_todolist);
         if(ret_val == NOT_FOUND){
            find_url = FALSE;

         }
         else if(ret_val){
            printf("disk hash find ERROR! return : %d\n", ret_val);
            lockf(todolist_lock_fd, F_ULOCK, 0);

            exit(ret_val);
         }
         else{
            printf("URL : %s EXIST!\n", add_todolist);
         }

         if(todolist_buf_ptr - todolist_buf >= info.st_size){
            todolist_buf_ptr = NULL;

            find_url = FALSE;
         }
      }

      if(!todolist_buf_ptr){
         ftruncate(todolist_fd, 0);
      }
      else{
         lseek(todolist_fd, 0, SEEK_SET);
         if(strlen(todolist_buf_ptr) != write(todolist_fd, todolist_buf_ptr, strlen(todolist_buf_ptr))){
            printf("to do list write ERROR! return : %2d, %s .\n", errno, strerror(errno));

            goto ERROR;
         }
         ftruncate(todolist_fd, strlen(todolist_buf_ptr));
      }
   }

   if(*add_todolist != '/'){
      char *temp = add_todolist;
      for(; *temp != '/'; temp++);
      for(int i = 0; *(temp - 1) != '\0'; temp++, i++){
         add_todolist[i] = *temp;
      }
   }

   lockf(disk_hash_lock_fd, F_LOCK, 0);
   if(ret_val = open_table(&dh)){
      printf("open disk hash table ERROR! return : %d\n", ret_val);

      goto ERROR;
   }

   ret_val = add(add_todolist, host_url, strlen(host_url) + 1, TYPE_CHAR, &dh);
   if(ret_val){
      printf("disk hash add ERROR! return : %d\n", ret_val);
      lockf(disk_hash_lock_fd, F_ULOCK, 0);
      lockf(todolist_lock_fd, F_ULOCK, 0);

      exit(ret_val);
   }

   if(ret_val = close_table(&dh)){
      printf("close disk hash ERROR! return : %d\n", ret_val);

      goto ERROR;
   }


 ERROR:
   lockf(todolist_lock_fd, F_ULOCK, 0);
   lockf(disk_hash_lock_fd, F_ULOCK, 0);


   return ret_val;
}

int check_dir_num(){
   DIR * dir = opendir(web_data_dir_name);
   if(!dir){
      printf("open dir %s ERROR! return : %s", web_data_dir_name, strerror(errno));

      return ERR_OPEN_DIR;
   }
   struct dirent *ptr;
   int count = 0;

   while((ptr = readdir(dir)) != NULL){
      count++;
   }

   closedir(dir);

 #ifdef _DEBUG
   printf("%s data total number : %d .\n", web_data_dir_name, count);
 #endif


   return count > WEB_DATA_LIMIT ? REACH_DATA_LIMIT : SUCCESS;
}

int open_web_data_file(int data_count, int *data_fd){
   int   data_len = strlen(web_data_dir_name) + 20;
   char  data_fname[data_len];
   char  *data_ptr = data_fname;

   strncpy(data_ptr, web_data_dir_name, data_len);
   data_ptr += strlen(data_ptr);

   strncpy(data_ptr, "/", 1);
   data_ptr++;

   sprintf(data_ptr, "%d", getpid());
   data_ptr += strlen(data_ptr);

   strncpy(data_ptr, "_", 1);
   data_ptr++;

   sprintf(data_ptr, "%d.txt", data_count);

 #ifdef _DEBUG
   printf("data file name : %s .\n", data_fname);
 #endif

   *data_fd = open(data_fname, O_RDWR | O_CREAT | O_TRUNC, 0777);
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

   // strcat(req_ptr, "Connection: close\r\n");
   // strcat(req_ptr, "Content-Type: text/html; charset=UTF-8\r\n");
   // strcat(req_ptr, "Accept-Encoding: chunked\r\n");
   // strcat(req_ptr, "User-Agent: Mozilla/5.0\r\n");
   // strcat(req_ptr, "Transfer-Encoding: chunked\r\n");

   strncat(req_ptr, "\r\n", req_limit);
   req_ptr     += sizeof("\r\n") - 1;
   req_limit   -= sizeof("\r\n") - 1;

 #ifdef _DEBUG
   printf("\n********** pid : %2d Request: **********\n%s\n", process_index, request);
   printf(  "****************************************\n\n");
 #endif

   bool want_write = TRUE;
   while(want_write){
      want_write = FALSE;

      ret = SSL_write(SSL->ssl, request, strlen(request));
      if(ret <= 0){
         int ssl_get_ret = SSL_get_error(SSL->ssl, ret);

         switch(ssl_get_ret){
            case SSL_ERROR_NONE:
               printf("openssl write ERROR! return : SSL_ERROR_NONE\n");
               break;
            case SSL_ERROR_ZERO_RETURN:
               printf("openssl write ERROR! return : SSL_ERROR_ZERO_RETURN pid : %d .\n", getpid());
               openSSL_close(SSL);
               *(child_catch_url[process_index]) = 0;

               exit(ERR_OPENSSL);
               break;
            case SSL_ERROR_WANT_WRITE:
               printf("openssl write ERROR! return : SSL_ERROR_WANT_READ\n");
               want_write = TRUE;

               break;
            case SSL_ERROR_WANT_CONNECT:
               printf("openssl write ERROR! return : SSL_ERROR_WANT_CONNECT\n");
               want_write = TRUE;

               break;
            case SSL_ERROR_WANT_ACCEPT:
               printf("openssl write ERROR! return : SSL_ERROR_WANT_ACCEPT\n");
               want_write = TRUE;

               break;
            case SSL_ERROR_WANT_X509_LOOKUP:
               printf("openssl write ERROR! return : SSL_ERROR_WANT_X509_LOOKUP\n");
               want_write = TRUE;

               break;
            case SSL_ERROR_SYSCALL:
               printf("openssl write ERROR! return : SSL_ERROR_SYSCALL\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
            case SSL_ERROR_SSL:
               printf("openssl write ERROR! return : SSL_ERROR_SSL\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
            default :
               printf("openssl write ERROR! return : ELSE\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
         }
      }
   }


   return SUCCESS;
}

int accept_response(_OPENSSL *SSL, char *response, int data_file_count){
   int data_fd = 0;
   int ret = SUCCESS;
   int body_len = 0;

   if(ret = open_web_data_file(data_file_count, &data_fd)){
      printf("response open web data file ERROR! return : %d .\n", ret);

      exit(ret);
   }

 #ifdef _DEBUG
   printf("\n********** pid : %2d Response: **********\n", process_index);
 #endif

   bool want_read = TRUE;
   while(want_read){
      want_read = FALSE;
      memset(response, 0, SOCKET_LEN);

      ret = SSL_read(SSL->ssl, response, SOCKET_LEN);
      if(ret <= 0){
         int ssl_get_ret = SSL_get_error(SSL->ssl, ret);
         switch(ssl_get_ret){
            case SSL_ERROR_NONE:
               printf("first openssl read ERROR! return : SSL_ERROR_NONE\n");
               break;
            case SSL_ERROR_ZERO_RETURN:
               printf("first openssl read ERROR! return : SSL_ERROR_ZERO_RETURN pid : %d .\n", getpid());
               openSSL_close(SSL);
               *(child_catch_url[process_index]) = 0;

               exit(ERR_OPENSSL);
               break;
            case SSL_ERROR_WANT_READ:
               printf("first openssl read ERROR! return : SSL_ERROR_WANT_READ\n");
               want_read = TRUE;

               break;
            case SSL_ERROR_WANT_CONNECT:
               printf("first openssl read ERROR! return : SSL_ERROR_WANT_CONNECT\n");
               want_read = TRUE;

               break;
            case SSL_ERROR_WANT_ACCEPT:
               printf("first openssl read ERROR! return : SSL_ERROR_WANT_ACCEPT\n");
               want_read = TRUE;

               break;
            case SSL_ERROR_WANT_X509_LOOKUP:
               printf("first openssl read ERROR! return : SSL_ERROR_WANT_X509_LOOKUP\n");
               want_read = TRUE;

               break;
            case SSL_ERROR_SYSCALL:
               printf("first openssl read ERROR! return : SSL_ERROR_SYSCALL\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
            case SSL_ERROR_SSL:
               printf("first openssl read ERROR! return : SSL_ERROR_SSL\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
            default :
               printf("first openssl read ERROR! return : ELSE\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
         }
      }
   }

 #ifdef _DEBUG
   //printf("%s\n", response);
 #endif

   if(ret = response_head_handle(response, &body_len)){
      printf("response head handle ERROR! return : %d .\n", ret);

      exit(ret);
   }

 #ifdef _DEBUG
   printf(  "****************************************\n\n");
 #endif

   char write_temp[URL_LIMIT];
   add_todolist[strlen(add_todolist)] = '\n';
   sprintf(write_temp, "%s\n", add_todolist);
   if(strlen(write_temp) != write(data_fd, write_temp, strlen(write_temp))){
      printf("write_temp add to do list write ERROR! return : %2d, %s .\n", errno, strerror(errno));

      return ERR_WRITE;
   }
   add_todolist[strlen(add_todolist) - 1] = '\0';

   if(strlen(response) != write(data_fd, response, strlen(response))){
      printf("first add to do list write ERROR! return : %2d, %s .\n", errno, strerror(errno));

      return ERR_WRITE;
   }

   while(body_len){
      memset(response, 0, SOCKET_LEN);

      ret = SSL_read(SSL->ssl, response, SOCKET_LEN);
      if(ret <= 0){
         int ssl_get_ret = SSL_get_error(SSL->ssl, ret);
         switch(ssl_get_ret){
            case SSL_ERROR_NONE:
               printf("first openssl read ERROR! return : SSL_ERROR_NONE\n");
               break;
            case SSL_ERROR_ZERO_RETURN:
               printf("first openssl read ERROR! return : SSL_ERROR_ZERO_RETURN pid : %d .\n", getpid());
               openSSL_close(SSL);
               *(child_catch_url[process_index]) = 0;

               exit(ERR_OPENSSL);
               break;
            case SSL_ERROR_WANT_READ:
               printf("first openssl read ERROR! return : SSL_ERROR_WANT_READ\n");
               continue;

               break;
            case SSL_ERROR_WANT_CONNECT:
               printf("first openssl read ERROR! return : SSL_ERROR_WANT_CONNECT\n");
               continue;

               break;
            case SSL_ERROR_WANT_ACCEPT:
               printf("first openssl read ERROR! return : SSL_ERROR_WANT_ACCEPT\n");
               continue;

               break;
            case SSL_ERROR_WANT_X509_LOOKUP:
               printf("first openssl read ERROR! return : SSL_ERROR_WANT_X509_LOOKUP\n");
               continue;

               break;
            case SSL_ERROR_SYSCALL:
               printf("first openssl read ERROR! return : SSL_ERROR_SYSCALL\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
            case SSL_ERROR_SSL:
               printf("first openssl read ERROR! return : SSL_ERROR_SSL\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
            default :
               printf("first openssl read ERROR! return : ELSE\n");
               openSSL_close(SSL);

               exit(ERR_OPENSSL);
               break;
         }
      }

      if(strlen(response) != write(data_fd, response, strlen(response))){
         printf("temp add to do list write ERROR! return : %2d, %s .\n", errno, strerror(errno));

         return ERR_WRITE;
      }

      if(body_len != -1){
         body_len -= strlen(response);

       #ifdef _DEBUG
         if(body_len <= 0){
            printf("body len : %d\n", body_len);
            break;
         }
       #endif
      }
      else{
         char *temp;
         if(temp = strstr(response, "0\r\n\r\n")){
            if(temp == response || *(temp - 1) == '\n'){
             #ifdef _DEBUG
               printf("write ... body len : chunked\n");
             #endif

               break;
            }
         }

      }

       #ifdef _DEBUG
         //printf("%s\n", response);
         //break;
       #endif
   }

   response_body_handle(data_fd);

   if(close_header){
      *(child_catch_url[process_index]) = 0;

      exit(ERR_SSL_CONNCET);
   }


   return SUCCESS;
}

int response_head_handle(const char *response, int *body_len){
   if(!response){
      return ERR_PARAMETER;
   }

   char *resp_ptr    = (char*)response;
   int   resp_status = 0;

   resp_ptr    = strchr(resp_ptr, ' ');
   resp_status = atoi(++resp_ptr);

   printf("%s response status code : %d .\n", add_todolist, resp_status);

   if(resp_status >= 100 && resp_status < 200){
      *body_len = 0;
   }
   else if(resp_status >= 200 && resp_status < 300){
      *body_len = 0;
      if(resp_ptr = strstr(response, "Content-Length:")){
         *body_len = atoi((resp_ptr + sizeof("Content-Length:")));
      }
      else if(resp_ptr = strstr(response, "Transfer-Encoding: chunked")){
         *body_len = -1;
      }
   }
   else if(resp_status >= 300 && resp_status < 400){
      char temp_add[URL_LIMIT];

      resp_ptr = strstr(response, "Content-Length:");
      if(resp_ptr){
         *body_len = atoi((resp_ptr + sizeof("Content-Length:")));
      }
      else{
         *body_len = 0;
      }

      resp_ptr = strstr(response, "Location: ");
      if(resp_ptr){
         resp_ptr += sizeof("Location: ") - 1;

         bool same_host = TRUE;
         char *temp = strstr(resp_ptr, "https://");
         if(temp){
            resp_ptr = temp + sizeof("https://") - 1;
            for(int i = 0; *temp != '/' && i < strlen(host_url); i++, temp++){
               if(*temp != host_url[i]){
                  same_host = FALSE;

                  break;
               }
            }
         }

         if(same_host){
            for(int i = 0; i < URL_LIMIT - 1; i++, resp_ptr++){
               if(*resp_ptr != '\r' && *resp_ptr != '\n'){
                  temp_add[i] = *resp_ptr;
               }
               else{
                  temp_add[i]       = '\n';
                  temp_add[i + 1]   = '\0';
                  break;
               }
            }

            lockf(todolist_lock_fd, F_LOCK, 0);
            lseek(todolist_fd, 0, SEEK_END);
            if(strlen(temp_add) != write(todolist_fd, temp_add, strlen(temp_add))){
               printf("temp add to do list write ERROR! return : %2d, %s .\n", errno, strerror(errno));
               lockf(todolist_lock_fd, F_ULOCK, 0);

               return ERR_WRITE;
            }
            lockf(todolist_lock_fd, F_ULOCK, 0);
         }
      }
   }
   else if(resp_status >= 400 && resp_status < 500){
      *body_len = 0;
   }
   else{
      *body_len = 0;
   }

   if(*body_len > 0){
      resp_ptr =  strstr(response, "\r\n\r\n");
      resp_ptr += (sizeof("\r\n\r\n") - 1);
      *body_len -= strlen(resp_ptr);
   }

   if(strstr(response, "Connection: close")){
      close_header = TRUE;
   }

   return SUCCESS;
}

int response_body_handle(const int data_fd){
   int ret = 0;
   int  data_size = lseek(data_fd, 0, SEEK_END);
   char *data_buf = (char*)malloc(data_size + 1 * sizeof(char));
   if(!data_buf){
      printf("response read data ERROR! return : %s", strerror(errno));

      return ERR_OUT_OF_MEM;
   }
   data_buf[data_size] = 0;

   lseek(data_fd, 0, SEEK_SET);
   if(data_size != read(data_fd, data_buf, data_size)){
      printf("data read ERROR! return : %2d, %s .\n", errno, strerror(errno));

      return ERR_READ_FILE;
   }

   char *a_href_ptr = data_buf;
   while(a_href_ptr = strstr(a_href_ptr, "<a ")){
      if(!(a_href_ptr = strstr(a_href_ptr, "href="))){
         continue;
      }
      a_href_ptr +=  sizeof("href=") - 1;

      for(; *a_href_ptr == '\"' || *a_href_ptr == '\'' || *a_href_ptr == ' '; a_href_ptr++);

      char *temp = a_href_ptr;
      for(; *temp && *temp != '\"' && *temp != ' ' && *temp != '>' && *temp != '\''; temp++);

      char body_add[URL_LIMIT];
      memset(body_add, 0, URL_LIMIT);

      for(char *fptr = a_href_ptr, *sptr = body_add; fptr != temp && (fptr - a_href_ptr) < (URL_LIMIT - 1); fptr++, sptr++){
         if(*fptr == '\r'){
            fptr = strstr((fptr + 1), "\r\n");
            fptr += 2;
         }
         *sptr = *fptr;
      }
      //strncpy(body_add, a_href_ptr, (temp - a_href_ptr) < (URL_LIMIT - 1) ? (temp - a_href_ptr) : (URL_LIMIT - 1));


      bool same_host = TRUE;
      if(temp = strstr(body_add, "://")){
         if(*(temp - 1) != 's'){
            same_host = FALSE;
         }
         else{
            temp += sizeof("://") - 1;

            for(int i = 0; *temp != '/' && i < strlen(host_url); i++, temp++){
               if(*temp != host_url[i]){
                  same_host = FALSE;

                  break;
               }
            }
         }
      }
      else if(*body_add != '/' && *body_add != '#' && !strstr(body_add, "javascript:")){
         char temp_add[URL_LIMIT];
         temp_add[URL_LIMIT - 1] = 0;
         char *tail;

         temp = body_add;
         tail = strrchr(add_todolist, '/');
         if(!(*(tail + 1))){
            strcpy(temp_add, add_todolist);
         }
         else{
            char *i = add_todolist, *j = temp_add;
            for(; i != (tail + 1); i++, j++){
               *j = *i;
            }
            *j = 0;
         }
         strncat(temp_add, temp, URL_LIMIT - 1 - strlen(temp_add));
         temp = temp_add;
      }
      else if(*body_add == '/'){
         temp = body_add;
      }
      else{
         same_host = FALSE;
      }

    #ifdef _DEBUG
      printf("add_todolist : %s .\n", add_todolist);
      printf("body_add : %s .\n", body_add);
      printf("add to to do list URL : %s .\t same_host : %d\n", temp, same_host);
    #endif

      if(same_host){
         if(ret = add_todolist_file(temp)){
            printf("response_body_handle add_todolist_file ERROR!\n");

            exit(ret);
         }

         /* ret = disk_hash_find(temp);
         if(ret == NOT_FOUND){
            if(ret = add_todolist_file(temp)){
               printf("response_body_handle add_todolist_file ERROR!\n");

               exit(ret);
            }
         }
         else if(ret){
            printf("response_body_handle disk hash find ERROR! return : %d\n", ret);

            exit(ret);
         }
         else{
            printf("URL : %s EXIST!\n", add_todolist);
         } */
      }
   }

    #ifdef _DEBUG
      printf("<a is not find!!\n");
    #endif

   close(data_fd);

   return SUCCESS;
}

int add_todolist_file(char *add){
   int ret_val = URL_percent_encoding(add);
   if(ret_val){
      printf("URL_percent_encoding ERROR! return : %d\n", ret_val);

      return ret_val;
   }

   lockf(todolist_lock_fd, F_LOCK, 0);
   int len = strlen(add) < URL_LIMIT - 2 ? strlen(add) : URL_LIMIT - 2;
   add[len] = '\n';
   add[len + 1] = '\0';

   lseek(todolist_fd, 0, SEEK_END);
   if(len + 1 != write(todolist_fd, add, len + 1)){
      printf("write to do list file ERROR!\n");

      lockf(todolist_lock_fd, F_ULOCK, 0);
      return ERR_WRITE;
   }

   add[len] = '\0';
   lockf(todolist_lock_fd, F_ULOCK, 0);

   return SUCCESS;
}

int disk_hash_find(const char *find_key){
   int ret = 0;
   int find_ret = 0;

   lockf(disk_hash_lock_fd, F_LOCK, 0);
   if(ret = open_table(&dh)){
      printf("open disk hash table ERROR! return : %d\n", ret);
      lockf(disk_hash_lock_fd, F_ULOCK, 0);

      return ret;
   }
   find_ret = find(find_key, &dh, NULL);
   if(ret = close_table(&dh)){
      printf("close disk hash ERROR! return : %d\n", ret);
      lockf(disk_hash_lock_fd, F_ULOCK, 0);

      return ret;
   }
   lockf(disk_hash_lock_fd, F_ULOCK, 0);

   return find_ret;
}

int URL_percent_encoding(char original[]){
   if(!original){
      return ERR_PARAMETER;
   }

   char *optr = original;
   char new[URL_LIMIT];
   char *nptr = new;
   char arr[] = "0123456789ABCDEF";

   /******************************************
   * 0   ~  9     A  ~  Z     a  ~  z        *
   * 48  ~  57    65 ~  90    97 ~  122      *
   *                                         *
   * !   #  &  '  (  )  *  +  ,  -  .  /     *
   * 33  35 38 39 40 41 42 43 44 45 46 47    *
   *                                         *
   * :   ;  =  ?  @  [  ]  _  ~              *
   * 58  59 61 63 64 91 93 95 126            *
   *******************************************/

   for(int i = 0; *optr != 0 && i < (URL_LIMIT - 1); optr++, nptr++, i++){
      bool judge = FALSE;

      if(*optr == '%'){
         char temp[] = "abcdef";

         for(int i = 0; i < strlen(arr), !judge; i++){
            if(arr[i] == *(optr + 1)){
               judge = TRUE;
            }
         }
         for(int i = 0; i < strlen(temp), !judge; i++){
            if(temp[i] == *(optr + 1)){
               judge = TRUE;
            }
         }
      }

      if(*optr == 33 || *optr == 35 || *optr == 38 || *optr == 39 || *optr == 40 || *optr == 41 ||
         *optr == 42 || *optr == 43 || *optr == 44 || *optr == 45 || *optr == 46 || *optr == 47 ||
         *optr == 58 || *optr == 59 || *optr == 61 || *optr == 63 || *optr == 64 || *optr == 91 ||
         *optr == 93 || *optr == 95 || *optr == 126 || (*optr >= 48 && *optr <= 57) ||
         (*optr >= 65 && *optr <= 90) || (*optr >= 97 && *optr <= 122)){

         *nptr = *optr;
      }
      else if(judge){
         *nptr++  = *optr++;
         *nptr++  = *optr++;
         *nptr    = *optr;
      }
      else{
         *(nptr++) = '%';
         *(nptr++) = *optr < 0 ? arr[((*optr + 256) >> 4)] : arr[((*optr) >> 4)];
         *nptr = *optr < 0 ? arr[((*optr + 256) & 15)] : arr[((*optr) & 15)];
      }
   }
   *nptr = 0;
   strcpy(original, new);


   return SUCCESS;
}


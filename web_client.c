#include "web_client.h"


int main(int argc, char *argv[]){
   int ret_val = 0;
   char *host_url = NULL;
   char *add_todolist = NULL;
   int process_count = 5;
   char *web_data_dir_name = NULL;
   int web_data_dir = 0;
   int queue_fd = 0;
   _DISK_HASH dh;
   int temp_fd = 0;
   int *child_state = NULL;

   if (argc < 3 || argc > 4){
      printf("parameter error!!\n");
      printf("./web_client “URL” “Output Directory” [“process count”]\n");

      return SUCCESS;
   }

   if(argc == 4){
      process_count = atoi(argv[3]);
   }

   pid_t child_pid[process_count];

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

   add_todolist = (char*)malloc(strlen(argv[1]) + 1);
   if(!add_todolist){
      ret_val = ERR_OUT_OF_MEM;
      printf("add to to do list malloc ERROR!\n", ret_val);

      goto ERROR;
   }

   to_do_list_url_string_conversion(argv[1], add_todolist);

 #ifdef _DEBUG
   printf("add to to do list URL : %s\n", add_todolist);
 #endif

   char *output_dir = argv[2];
   if(output_dir[strlen(output_dir) - 1] == '/'){
      output_dir[strlen(output_dir) - 1] = '\0';
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
   sprintf(web_data_dir_ptr, "%d", web_data_dir);

 #ifdef _DEBUG
   printf("web data dir name : %s\n", web_data_dir_name);
 #endif

   ret_val = create_dir(web_data_dir_name);
   if(ret_val){
      printf("%s create ERROR!\n", web_data_dir_name);

      goto ERROR;
   }

   char *queue_filename = NULL;
   queue_filename = (char*)malloc(strlen(output_dir) + 1 + sizeof(QUEUE_FILE_NAME));
   if(!queue_filename){
      ret_val = ERR_OUT_OF_MEM;
      printf("queue_filename malloc ERROR!\n");

      goto ERROR;
   }

   strncpy(queue_filename, output_dir, strlen(output_dir) + 1);
   strcat(queue_filename, QUEUE_FILE_NAME);

 #ifdef _DEBUG
   printf("queue file name : %s\n", queue_filename);
 #endif

   queue_fd = open(queue_filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
   if(queue_fd < 0){
      ret_val = errno;
      printf("queue file open ERROR!");

      goto ERROR;
   }

   add_todolist[strlen(add_todolist)] = '\n';
   if(0 >= write(queue_fd, add_todolist, strlen(add_todolist))){
      printf("write add_todolist to queue ERROR!\n");
      ret_val = errno;

      goto ERROR;
   }

   /* ret_val = open_table(&dh);
   if(ret_val){
      printf("open disk hash ERROR!\n");

      goto ERROR;
   } */

   temp_fd = open(FILE_LOCK, O_RDWR | O_CREAT | O_TRUNC, 0777);
   if(temp_fd < 0){
      ret_val = errno;
      printf("temp_fd file open ERROR!");

      goto ERROR;
   }

   child_state = mmap(NULL, process_count * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
   if(child_state == MAP_FAILED){
      printf("mmap ERROR!\n");
      ret_val = errno;

      goto ERROR;
   }
   memset(child_state, STATE_READY, process_count * sizeof(int));

 #ifdef _DEBUG
   _OPENSSL SSL;
   ret_val = openSSL_connect(host_url, &SSL);

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

   //child do
   if(index != -1){

   }

   //parent do



 ERROR:
   if(ret_val)
      printf("return value : %d\n", ret_val);
   if(host_url)
      free(host_url);
   if(add_todolist)
      free(add_todolist);
   if(queue_filename)
      free(queue_filename);
   if(queue_fd > 0)
      close(queue_fd);
   if(temp_fd > 0){
      close(temp_fd);
      remove(FILE_LOCK);
   }
   if(web_data_dir_name)
      free(web_data_dir_name);
   if(child_state != MAP_FAILED)
      munmap(child_state, process_count * sizeof(int));

   /* if(index == -1)
      close_table(&dh); */


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

   optr = strstr(original, "://");
   if(!optr){
      strncpy(nptr, original, strlen(original) + 1);
   }
   else{
      optr += 3;
      for (; *optr != '/'; optr++);
      strncpy(nptr, optr, strlen(optr) + 1);
   }


   return SUCCESS;
}

int create_dir(const char* dir_name) {
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

int openSSL_connect(const char host[], _OPENSSL *SSL) {
   if(!host || !SSL){
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
   SSL->server = create_socket(host);
   if(SSL->server != 0) {
    #ifdef _DEBUG
      printf("Successfully made the TCP connection to: %s .\n", host);
    #endif
   }

   /* ---------------------------------------------------------- *
   * Attach the SSL session to the socket descriptor            *
   * ---------------------------------------------------------- */
   SSL_set_fd(SSL->ssl, SSL->server);

   /* ---------------------------------------------------------- *
   * Set to Support TLS SNI extension by Andric                 *
   * ---------------------------------------------------------- */
   SSL_ctrl(SSL->ssl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void*)host);

   /* ---------------------------------------------------------- *
   * Try to SSL-connect here, returns 1 for success             *
   * ---------------------------------------------------------- */
   if ((ret = SSL_connect(SSL->ssl)) != 1 ) {
      int err;
    #ifdef _DEBUG
      printf("Error: Could not build a SSL session to: %s .\n", host);
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
      printf("Successfully enabled SSL/TLS session to: %s .\n", host);
    #endif
   }


   return SUCCESS;
}

int create_socket(const char hostname[]) {
   if(!hostname){
      return ERR_PARAMETER;
   }

   int sockfd;
   char portnum[6] = "443";
   char *tmp_ptr = NULL;
   int  port;
   struct hostent *host;
   struct sockaddr_in dest_addr;

   /* ---------------------------------------------------------- *
   * if the hostname contains a colon :, we got a port number   *
   * ---------------------------------------------------------- */
   if (strchr(hostname, ':')) {
      tmp_ptr = strchr(hostname, ':');
      /* the last : starts the port number, if avail, i.e. 8443 */
      strncpy(portnum, tmp_ptr+1,  sizeof(portnum));
      *tmp_ptr = '\0';
   }

   port = atoi(portnum);

   if ((host = gethostbyname(hostname)) == NULL) {
      #ifdef _DEBUG
      printf("Error: Cannot resolve hostname %s .\n", hostname);
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
      printf("Error: Cannot connect to host %s [%s] on port %d.\n", hostname, tmp_ptr, port);
      #endif
   }


   return sockfd;
}

int openSSL_close(_OPENSSL *SSL) {
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
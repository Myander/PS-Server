#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#define BUFSIZE 16384
//#define SERVER_PATH "tpf_unix_sock.server"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
#define DATA "Hello from client"

int main(int argc, char *argv[]){

   int client_sock, rc, len;
   struct sockaddr_un server_sockaddr;
   struct sockaddr_un client_sockaddr;
   char *buf, *buf2;
   memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
   memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));

   if (argc != 2) {
           fprintf(stderr, "Usage: %s username\n",
                   argv[0]);
           exit(EXIT_FAILURE);
   }

   buf = malloc(BUFSIZE * sizeof(char));
   //buf2 = malloc(16384 * sizeof(char));

   /**************************************/
   /* Create a UNIX domain stream socket */
   /**************************************/
   client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
   if (client_sock == -1) {
       printf("SOCKET ERROR = \n");
       exit(1);
   }

   /***************************************/
   /* Set up the UNIX sockaddr structure  */
   /* by using AF_UNIX for the family and */
   /* giving it a filepath to bind to.    */
   /*                                     */
   /* Unlink the file so the bind will    */
   /* succeed, then bind to that file.    */
   /***************************************/
   client_sockaddr.sun_family = AF_UNIX;
   strcpy(client_sockaddr.sun_path, CLIENT_PATH);
   len = sizeof(client_sockaddr);

   unlink(CLIENT_PATH);
   rc = bind(client_sock, (struct sockaddr *) &client_sockaddr, len);
   if (rc == -1){
       printf("BIND ERROR: \n");
       close(client_sock);
       exit(1);
   }
   server_sockaddr.sun_family = AF_UNIX;
   strcpy(server_sockaddr.sun_path, SERVER_PATH);
   rc = connect(client_sock, (struct sockaddr *) &server_sockaddr, len);
   if(rc == -1){
       printf("CONNECT ERROR = \n");
       close(client_sock);
       exit(1);
   }

   /************************************/
   /* Copy the data to the buffer and  */
   /* send it to the server socket.    */
   /************************************/

   memset(buf, 0, BUFSIZE);
   strcpy(buf, argv[1]);
   printf("Sending data...\n");

   size_t total = 0;
   printf("%d\n", sizeof(buf));

   rc = send(client_sock, buf, BUFSIZE, 0);
   if (rc == -1) {
       printf("SEND ERROR = \n");
       close(client_sock);
       exit(1);
   }
   else {
       printf("Data sent: %s\n", buf);
   }

   /**************************************/
   /* Read the data sent from the server */
   /* and print it.                      */
   /**************************************/
   printf("Waiting to recieve data...\n");
   memset(buf, 0, BUFSIZE);
   rc = recv(client_sock, buf, BUFSIZE, 0);
                ;
   if (rc == -1) {
       printf("RECV ERROR = \n");
       close(client_sock);
       exit(1);
   }
   else {
       printf("DATA RECEIVED = %s\n", buf);
   }

   /******************************/
   /* Close the socket and exit. */
   /******************************/
   //free(buf2);
   free(buf);
   close(client_sock);

   return 0;
}

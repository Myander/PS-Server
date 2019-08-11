#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#define BUFSIZE 255
#define NTHREADS 10
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "client_path"
#define DATA "Hello from client"

int thread_count;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

struct workerArgs
{
    int connections;

};

void *client_handler(void *args);



int main(int argc, char *argv[]) {

        pthread_t thread1, thread2;
        int iret1, connections, i, j;
        struct workerArgs *wa;
        pthread_t thread_id[NTHREADS];

        connections = 100;
        wa = malloc(sizeof(struct workerArgs));
        wa->connections = connections;

        //printf("%d\n", argc);
   //if (argc != 4) {
           //fprintf(stderr, "Usage: %s uds.name [# clients] [# connections]\n",

           //exit(EXIT_FAILURE);
        //}
        //printf("%s %d %d\n",argv[1],atoi(argv[2]),atoi(argv[3]));
        for(i = 0; i < NTHREADS; i++) {
                pthread_create(&thread_id[i], NULL, client_handler, wa);
        }
        for(j = 0; j < NTHREADS; j++) {
                pthread_join(thread_id[j], NULL);
        }


        /*pthread_create(&thread1, NULL, client_handler, wa);
        pthread_create(&thread2, NULL, client_handler, wa);


        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);*/

	printf("connections: %d\n", thread_count);
        return 0;
}

void *client_handler(void *args) {

        int client_sock, rc, len, con, i;
        struct sockaddr_un server_sockaddr;
        struct sockaddr_un client_sockaddr;
        char *buf, *sockpath;
        pthread_t thread_id;

        memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
        memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
        struct workerArgs *wa;
	    wa = (struct workerArgs*) args;
        con = wa->connections;

        sockpath = malloc(256 * sizeof(char));
        memset(sockpath, 0, 256);
        thread_id = pthread_self();
        printf("id: %u\n", (unsigned int)thread_id);

        sprintf(sockpath,"%s%u", CLIENT_PATH, (unsigned int)thread_id);
        printf("sock: %s\n", sockpath);
        buf = malloc(BUFSIZE * sizeof(char));
        //buf2 = malloc(16384 * sizeof(char));

        /**************************************/
        /* Create a UNIX domain stream socket */
        /**************************************/
        for (i = 0; i < con; i++) {

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
        strcpy(client_sockaddr.sun_path, sockpath);
        len = sizeof(client_sockaddr);

        unlink(sockpath);
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
	    unlink(sockpath);	
            exit(1);
        }


        /************************************/
        /* Copy the data to the buffer and  */
        /* send it to the server socket.    */
        /************************************/

        memset(buf, 0, BUFSIZE);
        strcpy(buf, "myles");
        printf("Sending data...\n");

        size_t total = 0;


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
            //printf("DATA RECEIVED = \n%s\n", buf);
        }
	pthread_mutex_lock( &mutex1 );
   	thread_count++;
   	pthread_mutex_unlock( &mutex1 );
        /******************************/
        /* Close the socket and exit. */
        /******************************/
        close(client_sock);
        }
        
        unlink(sockpath);
        free(sockpath);
        free(buf);
        close(client_sock);

        return NULL;
}

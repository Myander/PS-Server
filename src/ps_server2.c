#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <pwd.h>
#include <fcntl.h>
#include <time.h>
#include <stddef.h>
#include <getopt.h>
#include <pthread.h>
#include "ps_server.h"
#define BUFSIZE 16384
# define __USE_MISC 1

struct workerArgs
{
    int socket;
    int filedesc;
};


int flag;
int ffind;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;



/*
 * Function: handle
 * Parameters:
 *     arg: signal being caught
 *
 * Description: signal handler for SIGINT
 * sets flag to 1 when SIGINT is caught.
 */
void *handle_request(void *client_sock);

void handle(int arg)
{
	if (arg == SIGINT) {
		flag = 1;
		printf("\nExiting gracefully\n");
	}
}

int main(int argc, char *argv[])
{
	char *fname;
	int server_sock, client_sock, rc, opt;
	unsigned int len;
	pthread_t thread1;
        int fd;
        int tcount = 0;
	//int bytes_rec = 0;
	struct sockaddr_un server_sockaddr;
	struct sockaddr_un client_sockaddr;
	//char *buf;
	int backlog = 10;
	//int ffind = 0;
	struct workerArgs *wa;

	memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
	memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
	//buf = malloc(BUFSIZE * sizeof(char));

	//struct passwd *pw;
	char *date;
	//struct node *listptr;
	//int fd;

	struct sigaction siga;

    	siga.sa_handler = &handle;
    	siga.sa_flags = SIGINT;
    	sigemptyset (&siga.sa_mask);
    	if (sigaction(SIGINT, &siga, 0) == -1) {
        	//perror(“Sigaction Error!“);
        	flag = 1;
    	}

	if (argc > 4 || argc == 1) { // check for correct number of arguments
		fprintf(stderr, "Usage: %s [-v filename] socketname\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	while ((opt = getopt(argc, argv, "v:")) != -1) { // parse arguments
		switch (opt) {
		case 'v':
			fname = optarg;
			ffind = 1;
			break;
		default:
			fprintf(stderr, "Usage: %s [-v filename] socketname\n",
				argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (ffind == 1) { // if -v file then create file and write date
		fd = open(fname, O_RDWR|O_CREAT|O_APPEND, 0666);
		time_t curtime;
		struct tm *loc_time;

		curtime = time(NULL);
		loc_time = localtime(&curtime);
		date = malloc(256 * sizeof(char));
		memset(date, 0, 256);
		strftime(date, 256, "PS Server logging started %a %b %d %R %Z %Y\n", loc_time);
		write(fd, date, strlen(date)+1);
		free(date);
	}

	if (optind >= argc) { // check for arguments after options
		fprintf(stderr, "Expected argument after options\n");
		exit(EXIT_FAILURE);
	}


	/**************************************/
	/* Create a UNIX domain stream socket */
	/**************************************/

	server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_sock == -1) {
		printf("SOCKET ERROR:\n");
		exit(1);
	}


	server_sockaddr.sun_family = AF_UNIX;
	strncpy(server_sockaddr.sun_path, argv[optind], strlen(argv[optind])+1);
	len = sizeof(server_sockaddr);

	unlink(argv[optind]);
	rc = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
	if (rc == -1) {
		printf("BIND ERROR:\n");
		close(server_sock);
		exit(1);
	}

	/*********************************/
	/* Listen for any client sockets */
	/*********************************/
	rc = listen(server_sock, backlog);
	if (rc == -1) {
		printf("LISTEN ERROR:\n");
		close(server_sock);
		exit(1);
	}
	printf("socket listening...\n");

	while (flag != 1) { // loop until SIGINT sent

		client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, &len);
		if (client_sock == -1) {
			if (flag == 1)
				break;
			printf("ACCEPT ERROR:\n");
			continue;
		}
		//printf("%d\n", client_sock);
		// pthread_cond_signal
		wa = malloc(sizeof(struct workerArgs));
        	wa->socket = client_sock;
                wa->filedesc = fd;
		pthread_create(&thread1, NULL, handle_request, wa);
		//handle_request(&client_sock);


	}


	if (ffind == 1)
		close(fd);
	if (flag != 1)
		close(client_sock);
	close(server_sock);
        return 0;
}

void *handle_request(void *args)
{
	int bytes_rec = 0;
	char *buf, *pwbuf;
	size_t bufsize;
	struct passwd pwd;
	struct passwd *result;
	char *token, *data, *line;
	struct node *listptr;
	int pcount, rc, socket, fd, s;
	struct workerArgs *wa;
        pthread_t thread_id;

	wa = (struct workerArgs*) args;
	socket = wa->socket;

	bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufsize == -1)
		bufsize = 16384;
	pwbuf = malloc(bufsize);

	pthread_detach(pthread_self());
	buf = malloc(BUFSIZE * sizeof(char));
	memset(buf, 0, BUFSIZE);
	bytes_rec = recv(socket, buf, BUFSIZE, 0);
	//printf("after recv\n");
	if (bytes_rec == -1) {
		printf("RECV ERROR:\n");
		close(socket);
		return NULL;
	} else { // strip newline, retrieve user info
                //printf("buf: %s\n", buf);
		token = strtok(buf, " \t\r\n\v\f");
                if (token == NULL) {
                        printf("user does not exist.\n");
                        close(socket);
                        return NULL;
                }
		//pw = getpwnam(token);
		s = getpwnam_r(token, &pwd, pwbuf, bufsize, &result);
	}

	if (result == NULL) {
		printf("user does not exist2.\n");
		close(socket);
		free(pwbuf);
                free(buf);
                return NULL;
	}
	/*if (pw == NULL) { // check if user exists
		printf("user does not exist2.\n");
		close(socket);
		free(buf);
		return NULL;
	} */

	int uid = (int)pwd.pw_uid;

	//retrieve process info about user and assemble data
        //pthread_mutex_lock( &mutex1 );
	listptr = search_proc(uid);
	if (listptr == NULL) {
                close(socket);
                free(buf);
		free(wa);
                return NULL;
        }
        //pthread_mutex_unlock( &mutex1 );
	data = client_data(listptr);
	pcount = process_count(listptr);
        thread_id = pthread_self();

	if (ffind == 0) // if no file specified print to standard out
		printf("USER %s [%d processes] %u\n", token, pcount, (unsigned int)thread_id);
	else { // write to file
                fd = wa->filedesc;
		line = malloc(256 * sizeof(char));
		memset(line, 0, 256);
		sprintf(line, "USER %s [%d processes]\n", token, pcount);
                pthread_mutex_lock( &mutex2 );
		write(fd, line, strlen(line)+1);
                pthread_mutex_unlock( &mutex2 );
                free(line);
	}

	free_list(listptr);
	memset(buf, 0, BUFSIZE);
	strncpy(buf, data, strlen(data)+1);
	free(data);
	// send user process data to client
	rc = send(socket, buf, BUFSIZE, 0);
	if (rc == -1) {
		printf("SEND ERROR:\n");
		return NULL;
	}
	close(socket);
	free(wa);
	free(buf);
	//pthread_detach(pthread_self());
	return NULL;
}

/*
 * Function: search_proc
 * Parameters:
 *    uid: user id to be searched for in /proc/pid/status
 * Returns: point to head of linked list containing process data
 * Description: searches proc directory for all directories
 * that have numeric names (pids) then calls parse_status
 * to find the process name and user id.
 *
 */
struct node *search_proc(int uid)
{
	char *pid;
	struct dirent *dp;
	DIR *dir;
	int len, i;
	int id = uid;
	int chcount = 0;
	struct node *head = NULL;
	struct node **listptr;

	listptr = &head;
	dir = opendir("/proc");
	if (dir == NULL) {
		fprintf(stderr, "Error: the directory does not exist\n");
		return head;
	}

	while ((dp = readdir(dir)) != NULL) {

		if (dp->d_type == 4) {
			for (i = 0; i < strlen(dp->d_name); i++) {
				if (dp->d_name[i] < '0' || dp->d_name[i] > '9')
					chcount++;

				if (dp->d_name[i+1] == '\0' && chcount == 0) {
					len = strlen(dp->d_name);
					pid = malloc((len+14) * sizeof(char));
					strncpy(pid, "/proc/", 7);
					strncat(pid, dp->d_name, len+1);
					strncat(pid, "/status", 8);
                                        pthread_mutex_lock( &mutex1 );
                                        parse_status(pid, listptr, id);
                                        pthread_mutex_unlock( &mutex1 );

					free(pid);

				}
			}
			chcount = 0;
		}
	}
	closedir(dir);
	return head;
}

/*
 * Function: parse_status
 * Parameters:
 *     filepath: file to be parsed
 *     head: pointer to head of linked list
 *     uid: user id to be compared
 *
 * Description: parses the status files of process directories
 * found in /proc/pid/status and compares the uids, if they
 * match then process name is added to linked list.
 */
void parse_status(char *filepath, struct node **head, int uid)
{
	int flag = 0;
	char *data;
	char *token, *name;
	int len, num, fd;
	int id = uid;
	fd = open(filepath, O_RDONLY);
	if (fd < 0)
		return;
	data = malloc(4096 * sizeof(char));
	memset(data, 0, 4096);
	read(fd, data, 4096);
	token = strtok(data, " \t\r\n\v\f");
        while (token != NULL) {
		if (strncmp(token, "Name:", 5) == 0) {
			flag = 1;
			token = strtok(NULL, " \t\r\n\v\f");
			continue;
		}
		if (strncmp(token, "Uid:", 4) == 0) {
			flag = 2;
			token = strtok(NULL, " \t\r\n\v\f");
			continue;
		}
		if (flag == 1) {
			len = strlen(token);
			name = malloc((len+1) * sizeof(char));
			strncpy(name, token, len+1);
			flag = 0;
		} else if (flag == 2) {
			num = atoi(token);
			if (id == num)
                                *head = alph_insert(name, *head);
                        free(name);
			flag = 0;
		}
		token = strtok(NULL, " \t\r\n\v\f");
	}
	close(fd);
	free(data);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include "ps_server.h"

/*
 * Function: create
 * Parameters:
 *	name: user name
 *      next: pointer to next node
 * Returns: pointer to head of link list
 * Description: create new link list node which points to next
 */
struct node *create(char *name, struct node *next)
{
	int len;
	struct node *new_node = malloc(sizeof(struct node));

	if (new_node == NULL) {
		printf("Error");
		exit(0);
	}
	len = strlen(name);
	new_node->name = malloc(len+1 * sizeof(char));
	strncpy(new_node->name, name, len+1);
	new_node->count = 1;
	new_node->next = next;
	return new_node;
}

/*
 * Function: alph_insert
 * Parameters:
 *      name: user name
 *      head: pointer to head node
 * Returns: pointer to head of link list
 * Description: alphabetic insert into linked list
 * calls create function and inserts node
 */

struct node *alph_insert(char *name, struct node *head)
{
	if (head == NULL) { // list empty
		head = create(name, head);
		return head;
	}

	if (head->next == NULL) { // one entry

		if (str_match(head->name, name) == 0) // if names match
			head->count++;
		else if (str_match(head->name, name) < 0) { // insert at end
			struct node *new_node = create(name, NULL);

			head->next = new_node;
		} else { // insert at front
			struct node *new_node = create(name, head);

			head = new_node;
		}
		return head;
	}

	struct node *prev = head;
	struct node *itr = head;

	while (itr->next != NULL) { // multiple entries in list

		if (str_match(name, head->name) == 0) {// names match
			head->count++;
			return head;
		} else if (str_match(name, head->name) < 0) { // insert at front
			struct node *new_node = create(name, head);

			head = new_node;
			return head;
		}
		if (str_match(name, itr->name) == 0) {
			itr->count++;
			return head;
		} else if (str_match(name, itr->name) < 0) {
			struct node *new_node = create(name, itr);

			prev->next = new_node;
			return head;
		}

		prev = itr;
		itr = itr->next;

	}

	if (str_match(name, itr->name) == 0)
		itr->count++;
	else if (str_match(name, itr->name) < 0) { // insert before last node
		struct node *new_node = create(name, itr);

		prev->next = new_node;
	} else { // insert at end of list
		struct node *new_node = create(name, NULL);

		itr->next = new_node;
	}
	return head;
}

/*
 * Function: print_list
 * Parameters:
 *     head: pointer to head of linked list
 *
 * Description: print contents of linked list
 */

void print_list(struct node *head)
{
	if (head == NULL) {
		printf("list is empty");
		return;
	}
	struct node *itr = head;

	while (itr->next != NULL) {
		printf("%d %s\n", itr->count, itr->name);
		itr = itr->next;
	}
	printf("%d %s\n", itr->count, itr->name);
}

/*
 * Function: free_list
 * Parameters:
 *     head: pointer to head of linked list
 *
 * Description: free all memory allocated in linked list
 */
void free_list(struct node *head)
{
	if (head == NULL) { // list empty
		fprintf(stderr, "list empty\n");
		exit(0);
	}
	if (head->next == NULL) { // only one node to free
		free(head->name);
		free(head);
		return;
	}

	struct node *itr = head;
	struct node *itr2;

	while (itr->next != NULL) { // free nodes in list
		itr2 = itr->next;
		free(itr->name);
		free(itr);
		itr = itr2;
	}
	free(itr->name);
	free(itr); // free last node

}

/*
 * Function: process_count
 * Parameters:
 *     head: pointer to head of linked list
 * Returns: process count found in linked list
 * Description: iterates through the linked list
 * adding up all the processes and returning count.
 */
int process_count(struct node *head)
{
	int process_count = 0;

	if (head == NULL) {
		fprintf(stderr, "list empty\n");
		exit(0);
	}

	if (head->next == NULL)  // only one node to free
		return head->count;

	struct node *itr = head;

	while (itr->next != NULL) {
		process_count += itr->count;
		itr = itr->next;

	}
	process_count += itr->count;
	return process_count;

}

/*
 * Function: client_data
 * Parameters:
 *     head: pointer to head of linked list
 * Returns: pointer to character array
 * Description: iterates through linked list
 * and fills a buffer with process data to be
 * sent back to the client.
 */

char *client_data(struct node *head)
{
	char *buf, *line;

	buf = malloc(16384 * sizeof(char));
	line = malloc(256 * sizeof(char));
	memset(buf, 0, 16384);
	memset(line, 0, 256);

	if (head == NULL) {
		fprintf(stderr, "list empty\n");
		exit(0);
	}
	if (head->next == NULL)  // only one node to free
		sprintf(buf, "%d %s\n", head->count, head->name);


	struct node *itr = head;

	while (itr->next != NULL) {
		sprintf(line, "%d %s\n", itr->count, itr->name);
		strncat(buf, line, strlen(line)+1);
		itr = itr->next;

	}
	sprintf(line, "%d %s\n", itr->count, itr->name);
	strncat(buf, line, strlen(line)+1);
	free(line);
	return buf;
}



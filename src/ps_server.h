#ifndef HEADER_FILE
#define HEADER_FILE

// link list stuct and functions
struct node {
	int count;
	char *name;
	struct node *next;
};

struct node *create(char *name, struct node *next);
struct node *alph_insert(char *name, struct node *head);
void print_list(struct node *head);
void free_list(struct node *head);
char *client_data(struct node *head);

int process_count(struct node *head);
void parse_status(char *filepath, struct node **head, int uid);
// search proc functions
struct node *search_proc(int uid);
int str_match(char *str1, char *str2);

#endif

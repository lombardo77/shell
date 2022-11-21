#ifndef JB_LISTS
#define JB_LISTS

typedef struct job{
    pid_t pid;
    char* name;
    int index;
}job;


 
typedef struct node{
    struct node* next;
    job* data;

}node;

node* add(node* head, node* data);
node* rm_node(node* head, int pid);
void print_list(node* head);
void free_list(node* head);
node* new_list();
#endif

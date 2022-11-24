#ifndef JB_LISTS
#define JB_LISTS

typedef struct job{
    pid_t pid;
    char* name;
    int index;
    int bg;
    int status;
}job;


 
typedef struct node{
    struct node* next;
    job* data;

}node;

node* newjobnode(int i, char* ARG, pid_t pid, int bg, int status);
node* add(node* head, node* data);
node* rm_node(node* head, int pid);
void print_list(node* head);
void free_list(node* head);
node* new_list();
int get_pid(node* head, int pid);
#endif

#ifndef JB_LISTS
#define JB_LISTS

typedef struct job{
    pid_t pid;
    char* name;
    char* path;
    int index;
    int bg;
    int status;
}job;


 
typedef struct node{
    struct node* next;
    job* data;

}node;

void freelno(node* head);
void fnaj(node* nodein);
void freejob(job* jobin);
void freenode(node* nodein);
int is_running(job* job_in);
job* createjob(int i, char* arg, pid_t, int bg, int status, char* path);
node* createnode(job* job_in);
node* getnode(node* head, job* job);
job* getjob(node* head, int pid);
node* add(node* head, node* data);
node* rm_node(node* head, int pid);
void print_list(node* head);
void freelnaj(node* head);
node* new_list();
int getpidbi(node* head, int pid);
#endif

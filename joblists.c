#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RUNNING 1
#define STOPPED -1
#define DONE 0

//jobs
typedef struct job {
    pid_t pid;
    char* name;
    int index;
    int bg;
    int status; // 1 if running, 0 if done, -1 if stopped
}job;

//nodes
typedef struct node {
    struct node* next;
    job* data;
}node;

//add to list
node* add(node* head, node* data)
{
    node* tmp = head;
    while (tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->next = data;
    return head;
}

//prints a list
void print_list(node* head) {
    node* tmp = head;
    while (tmp->next != NULL) {
        printf("[%d] ", tmp->data->index);
        printf("%s ", tmp->data->name);
        printf("pid: %d ", tmp->data->pid);
        printf(tmp->data->status == RUNNING ? 
                "status: Running ": 
                (tmp->data->status == STOPPED ? 
                 "status : Stopped ": "status: finished "));
        printf(tmp->data->bg == 1 ? "bg: True\n": "bg: False\n");
        tmp = tmp->next;
    }
        printf("[%d] ", tmp->data->index);
        printf("%s ", tmp->data->name);
        printf("pid: %d ", tmp->data->pid);
        printf(tmp->data->status == RUNNING ?
                "status: Running ": "status: Stopped ");
        printf(tmp->data->bg == 1 ? "bg: True\n": "bg: False\n");
}

//frees node but not job
void freenode(node* nodein)
{
    free(nodein);
}

// frees job and name
void freejob(job* jobin)
{
    free(jobin->name);
    free(jobin);
}

void fnaj(node* nodein)
{
    freejob(nodein->data);
    freenode(nodein);
}


//frees list (needs work)
void freelnaj(node* head)
{
    node* tmp = head;
    node* nxt = tmp->next;
    while (tmp->next != NULL) {
        fnaj(tmp);
        tmp = nxt;
        nxt = nxt->next;
    }
    fnaj(tmp);
}

//fress list (nodes only)
void freelno(node* head)
{
    node* tmp = head;
    node* nxt = tmp->next;
    while (tmp->next != NULL) {
        freenode(tmp);
        tmp = nxt;
        nxt = nxt->next;
    }
    freenode(tmp);
}

//removes by pid
node* rm_node(node* head, int pid) {
    node* tmp = head;
    node* prev = NULL;
    while(tmp != NULL) {
        if (tmp->data->pid == pid) {
            if (prev == NULL){
                freenode(tmp);
                return tmp->next;
            } else if (tmp->next == NULL) {
                freenode(tmp);
                tmp = NULL;
                prev->next = NULL;
                return head;
            } else {   
                prev->next = tmp->next;
                freenode(tmp);
                return head;
            }
        }
        prev = tmp;
        tmp = tmp->next;
    }
    return head;
}

//returns a new list head
node* new_list() 
{
    job* a = malloc(sizeof(job));
    a->pid = 0;
    char* name = malloc(5);
    strcpy(name, "head");
    a->name = name;
    a->index = 0;
    a->bg = -1;
    a->status = DONE;

    node* head = malloc(sizeof(node));
    head->next = NULL;
    head->data = a;
    return head;
}


//returns pid by index
int getpidbi(node* head, int index){
    node* tmp = head;
    while(tmp != NULL) {
        if (tmp->data->index == index)
            return tmp->data->pid;
        tmp = tmp->next;
    }
    return -1;
}

// returns jobs by pid
job* getjob(node* head, int pid){
    node* tmp = head;
    while (tmp != NULL) {
        if (tmp->data->pid == pid){
            return tmp->data;
        }
        tmp = tmp->next;
    }
    return NULL;
}

//returns node by job
node* getnode(node* head, job* job)
{
    node* tmp = head;
    while (tmp != NULL) {
        if (tmp->data->pid == job->pid){
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

// new job
job* createjob(int i, char* arg, pid_t pid, int bg, int status)
{   
    char* name = (char*)malloc(strlen(arg) + 1 );
    strcpy(name, arg);
    job* new_job = (job*)malloc(sizeof(job));
    new_job->pid = pid;
    new_job->name = name;
    new_job->index = i;
    new_job->bg = bg;
    new_job->status = status;
    return new_job;
}

//input job. output new node
node* createnode(job* job_in)
{
    node* new_node = malloc(sizeof(node));
    new_node->next = NULL;
    new_node->data = job_in;
}

//is running?
int is_running(job* job_in)
{
    return job_in->status == 1;
}

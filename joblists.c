#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RUNNING 1
#define STOPPED -1

//jobs
typedef struct job{
    pid_t pid;
    char* name;
    int index;
    int bg;
    int status; // 1 if running, 0 if done, -1 if stopped
}job;

//nodes
typedef struct node{
    struct node* next;
    job* data;
}node;

//add to list
node* add(node* head, node* data){
    node* tmp = head;
    while (tmp->next != NULL)
    {
        tmp = tmp->next;
    }
    tmp->next = data;
    return head;
}

//removes by pid
node* rm_node(node* head, int pid){
    node* tmp = head;
    node* prev = NULL;

    while(tmp != NULL) {
        if (tmp->data->pid == pid){
            if (prev == NULL){
                free(tmp->data->name);
                free(tmp->data);
                free(tmp);
                return tmp->next;
            }
            else if (tmp->next == NULL) {
                free(tmp->data->name);
                free(tmp->data);
                free(tmp);
                tmp = NULL;
                prev->next = NULL;
                return head;
            } else{   
                free(tmp->data->name);
                free(tmp->data);
                free(tmp);
                prev->next = tmp->next;
                return head;
            }
        }
        prev = tmp;
        tmp = tmp->next;
    }
}

//prints a list
void print_list(node* head){
    node* tmp = head;
    int i = 0;
    while (tmp->next != NULL)
    {
        printf("[%d] ", tmp->data->index);
        printf("%s ", tmp->data->name);
        printf("pid: %d ", tmp->data->pid);
        printf(tmp->data->status == RUNNING ? 
                "status: Running ": "status: Stopped ");
        printf(tmp->data->bg == 1 ? "bg: True\n": "bg: False\n");
        tmp = tmp->next;
        i++;
    }
        printf("[%d] ", tmp->data->index);
        printf("%s ", tmp->data->name);
        printf("pid: %d ", tmp->data->pid);
        printf(tmp->data->status == RUNNING ?
                "status: Running ": "status: Stopped ");
        printf(tmp->data->bg == 1 ? "bg: True\n": "bg: False\n");
}

//frees list (needs work)
void free_list(node* head)
{
    node* tmp = head;
    node* nxt = tmp->next;
    while(tmp->next != NULL)
    {
        free(tmp->data->name);
        free(tmp->data);
        free(tmp);
        tmp = nxt;
        nxt = nxt->next;
    }
    free(tmp->data->name);
    free(tmp->data);
    free(tmp);
}

//returns a new list head
node* new_list()
{
    job* a = malloc(sizeof(job));
    a->pid = 0;
    a->name = malloc(1);
    a->index = 0;
    a->bg = -1;
    node* head = malloc(sizeof(node));
    head->next = NULL;
    head->data = a;
    return head;
}


//returns pid by index
int getpidbi(node* head, int index){
    node* tmp = head;
    while(tmp != NULL)
    {
        if (tmp->data->index == index)
            return tmp->data->pid;
        tmp = tmp->next;
    }
    return -1;
}

// returns jobs by pid
job* getjob(node* head, int pid){
    node* tmp = head;
    while(tmp != NULL)
    {
        if (tmp->data->pid == pid){
            return tmp->data;
        }
        tmp = tmp->next;
    }
    return NULL;
}

//returns node by job
node* getnode(node* head, job* job){
    node* tmp = head;
    while(tmp != NULL)
    {
        if (tmp->data->pid == job->pid){
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}


// creates a new job and node, could be two functions. 
node* newjobnode(int i, char* arg, pid_t pid, int bg, int status){
    char* name = (char*)malloc(strlen(arg) + 1 );
    strcpy(name, arg);
    job* child_job = (job*)malloc(sizeof(job));
    child_job->pid = pid;
    child_job->name = name;
    child_job->index = i;
    child_job->bg = bg;
    child_job->status = status;
    node* new_job = malloc(sizeof(node));
    new_job->next = NULL;
    new_job->data = child_job;
    return new_job;
}

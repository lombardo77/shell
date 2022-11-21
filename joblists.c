#include <stdio.h>
#include <stdlib.h>


typedef struct job{
    pid_t pid;
    char* name;
    int index;
}job;

 
typedef struct node{
    struct node* next;
    job* data;
}node;


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

    while(tmp != NULL)
    {
        if (tmp->data->pid == pid)
        {
            if (prev == NULL)
                return tmp->next;
            else if (tmp->next == NULL)
            {
                free(tmp->data->name);
                free(tmp->data);
                tmp = NULL;
                prev->next = NULL;
                free(tmp);
                return head;
            }
            else
            {
                prev->next = tmp->next;
                free(tmp->data->name);
                free(tmp->data);
                free(tmp);
                return head;
            }
        }
        prev = tmp;
        tmp = tmp->next;
    }
}

void print_list(node* head){
    node* tmp = head;
    int i = 0;
    while (tmp->next != NULL)
    {
        printf("[%d] ", tmp->data->index);
        printf("%s ", tmp->data->name);
        printf("pid: %d \n", tmp->data->pid);
        tmp = tmp->next;
        i++;
    }
        printf("[%d] ", tmp->data->index);
        printf("%s ", tmp->data->name);
        printf("pid: %d \n", tmp->data->pid);
}

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

node* new_list()
{
    job* a = malloc(sizeof(job));
    a->pid = 0;
    a->name = malloc(1);
    a->index = 0;
    node* head = malloc(sizeof(node));
    head->next = NULL;
    head->data = a;
    return head;
}


int get_pid(node* head, int index){
    node* tmp = head;
    while(tmp != NULL)
    {
        if (tmp->data->index == index)
            return tmp->data->pid;
        tmp = tmp->next;
    }
    return -1;
}

//int main(){
//    job j0 = {90123, "A", 1};
//    node head = {NULL, &j0};
//
//    job j1 = {2981, "B", 2}; 
//    node n1 = {NULL, &j1}; 
//    add(&head, &n1);
//    
//    job j2 = {2231, "C", 3}; 
//    node n2 = {NULL, &j2}; 
//    add(&head, &n2);
//    
//    job j3 = {23434, "d", 4}; 
//    node n3 = {NULL, &j3}; 
//    add(&head, &n3);
//
//
//    print_list(&head);
//
//
//}

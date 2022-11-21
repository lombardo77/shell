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

void add(node* head, node* data){
    node* tmp = head;
    while (tmp->next != NULL)
    {
        tmp = tmp->next;
    }
    tmp->next = data;
}

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
                tmp = NULL;
            else
            {
            prev->next = tmp->next;
            return head;
            }
        }
        prev = tmp;
        tmp = tmp->next;
    }

}

void print_list(node* head){
    node* tmp = head;

    while (tmp->next != NULL)
    {
        printf("%d; ", tmp->data->pid);
        printf("%s; ", tmp->data->name);
        printf("%d\n", tmp->data->index);
        tmp = tmp->next;
    }
        printf("%d; ", tmp->data->pid);
        printf("%s; ", tmp->data->name);
        printf("%d\n", tmp->data->index);
}



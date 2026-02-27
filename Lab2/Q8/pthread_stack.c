#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int num_threads = 0;

typedef struct node
{
    int node_id; // a unique ID assigned to each node
    struct node *next;
} Node;

Node *top; // top of stack

/*Option 1: Mutex Lock*/
void push_mutex()
{
    Node *old_node;
    Node *new_node;
    new_node = malloc(sizeof(Node));
    // update top of the stack below
    old_node->next = new_node;
    // assign a unique ID to the new node
    new_node->node_id = old_node->node_id + 1;


}
int pop_mutex()
{
    Node *old_node;
    Node *new_node;
    // update top of the stack below
    return old_node->node_id;
}
/*Option 2: Compare-and-Swap (CAS)*/
void push_cas()
{
    Node *old_node;
    Node *new_node;
    new_node = malloc(sizeof(Node));
    // update top of the stack below
    // assign a unique ID to the new node
}
int pop_cas()
{
    Node *old_node;
    Node *new_node;
    // update top of the stack below
    return old_node->node_id;
}
/* the thread function */
void *thread_func(int opt)
{
    /* Assign each thread an id so that they are unique in range [0, num_thread -1
    ] */
    int my_id;
    if (opt == 0)
    {
        push_mutex();
        push_mutex();
        pop_mutex();
        pop_mutex();
        push_mutex();
    }
    else
    {
        push_cas();
        push_cas();
        pop_cas();
        pop_cas();
        push_cas();
    }
    printf("Thread %d: exit\n", my_id);
    pthread_exit(0);
}
int main(int argc, char *argv[])
{
    num_threads = atoi(argv[1]);
    /* Option 1: Mutex */
    pthread_t *workers;
    for (int i = 0; i < num_threads; i++)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(...);
    }
    for (int i = 0; i < num_threads; i++)
        pthread_join(...);
    // Print out all remaining nodes in Stack
    printf("Mutex: Remaining nodes \n");
    /*free up resources properly */
    /* Option 2: CAS */
    for (int i = 0; i < num_threads; i++)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(...);
    }
    for (int i = 0; i < num_threads; i++)
        pthread_join(...);
    // Print out all remaining nodes in Stack
    printf("CAS: Remaining nodes \n");
    /*free up resources properly */
}

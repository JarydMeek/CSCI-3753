#include <semaphore.h>
#include <pthread.h>

#ifndef STACK_H
#define STACK_H


#define STACK_SIZE 10                       // max elements in stack
#define MAX_NAME_LENGTH 255                 // max chars in a hostname (including null terminator)

typedef struct {
    char array[STACK_SIZE][MAX_NAME_LENGTH];// storage array for strings
    int top;                                // array index indicating where the top is
    sem_t space_avail;                      // Semaphore that will be initialized to the size of the array for blocking
    sem_t items_avail;                      // Semaphore that will be initialized to 0, and will signal items being available
    pthread_mutex_t mutex;                  // Mutex that will allow for mutual exclusion
    int finished;
    int producers;
    int exited;
} stack;

int  stack_init(stack *s, int producers);                  // init the stack
int  stack_push(stack *s, char element[]);     // place element on the top of the stack
int  stack_pop (stack *s, char *element);    // remove element from the top of the stack
int  stack_finished (stack *s);
void  stack_done (stack *s);
void stack_free(stack *s);                  // free the stack's resources

#endif
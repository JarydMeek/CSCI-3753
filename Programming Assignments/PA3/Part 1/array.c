#include "array.h"
#include <stdio.h>
#include <string.h>

int stack_init(stack *s, int producers) {                  // init the stack
    s->top = 0;
    s->finished = 0;
    s->producers = producers;
    s->exited = 0;
    //Initialize semaphore to manage size of stack
    sem_init(&(s -> space_avail), 0, STACK_SIZE);

    //Initialize semaphore to handle items being available
    sem_init(&(s -> items_avail), 0, 0);

    //Initialize mutex for mutual exclusion
    pthread_mutex_init (&(s -> mutex), NULL);

    return 0;
}

int stack_push(stack *s, char element[]) {     // place element on the top of the stack
    sem_wait(&(s -> space_avail));
    pthread_mutex_lock(&(s -> mutex));
        //Critical Section
        strcpy(s->array[s->top++], element);
    pthread_mutex_unlock(&(s -> mutex));
    sem_post(&(s -> items_avail));
    return 0;
}

int stack_pop(stack *s, char *element) {     // remove element from the top of the stack
    int ret = 0;
    sem_wait(&(s -> items_avail));
    pthread_mutex_lock(&(s -> mutex));
        //Critical Section
	if (s->top != 0) {	//check if array actually has something, if not skip popping (idea created with help from Emily Parker)
		strcpy(element, s->array[--s->top]);
	} else {
		ret = -1;
	}
    pthread_mutex_unlock(&(s -> mutex));
    sem_post(&(s -> space_avail));
    return ret;
}

int stack_finished(stack *s) {
	int temp;
	pthread_mutex_lock(&(s -> mutex));
	sem_getvalue(&(s->items_avail), &temp);
	int fin = s->finished;
	pthread_mutex_unlock(&(s -> mutex));
	if (fin == 1 && temp == 0) {
		sem_post(&(s -> items_avail));
		return 1;
	}else { 
		return 0;
	}
}

void stack_done(stack *s) {
	pthread_mutex_lock(&(s -> mutex));
	s->exited++;
	if (s->producers == s->exited) {
		s->finished = 1;
		sem_post(&(s -> items_avail));
	}
	pthread_mutex_unlock(&(s -> mutex));
}

void stack_free(stack *s) {                 // free the stack's resources
    pthread_mutex_destroy(&(s -> mutex));
}
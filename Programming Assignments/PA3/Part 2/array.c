#include "array.h"
#include <stdio.h>
#include <string.h>

int stack_init(stack *s, int producers) {                  // init the stack
    s->top = 0;                 //store the current number of items in stack
    s->finished = 0;            //act as a bool to tell if we are done producing
    s->producers = producers;   //store how many producer threads SHOULD exit
    s->exited = 0;              //store how many producer threads HAVE exited.

    //Initialize semaphore to wait threads when stack full
    sem_init(&(s -> space_avail), 0, STACK_SIZE);

    //Initialize semaphore to handle items being available
    sem_init(&(s -> items_avail), 0, 0);

    //Initialize mutex for mutual exclusion
    pthread_mutex_init (&(s -> mutex), NULL);

    return 0;
}

int stack_push(stack *s, char element[]) {      //place element on the top of the stack
    sem_wait(&(s -> space_avail));              //check if space available in stack, wait if there isn't
    pthread_mutex_lock(&(s -> mutex));          //lock for mutual exclusion
        //Critical Section
        strcpy(s->array[s->top++], element);    //copy to stack, increment top of stack
    pthread_mutex_unlock(&(s -> mutex));        //unlock mutual exlusion
    sem_post(&(s -> items_avail));              //signal to any consumer threads that there's stuff to consume
    return 0;                                   //return no error.
}

int stack_pop(stack *s, char *element) {        //remove element from the top of the stack
    int ret = 0;                                //storage to store return value
    sem_wait(&(s -> items_avail));              //see if there are items available, if there aren't wait
    pthread_mutex_lock(&(s -> mutex));          //lock for mutual exclusion
        //Critical Section
        //logic here is this, if there is nothing else to be produced, and there is a consumer thread waiting, that consumer will never exit
        //so in our consumer, we force the first thread that exits to unlock the next and so on, until all consumer threads exit
        //the issue is, when we notify the waiting thread because one exited, there's nothing on the stack to pop
        //so check that, and if there isn't anything to pop, return negative 1, so that the consumer knows nothing was popped
        //otherwise pop like normal since there's stuff on the stack
        //(VERY basic idea created with help from Emily Parker)
	if (s->top != 0) {	//check if array actually has something, if not skip popping 
		strcpy(element, s->array[--s->top]);
	} else {
		ret = -1;
	}
    pthread_mutex_unlock(&(s -> mutex));    //unlock mutual exclusion
    sem_post(&(s -> space_avail));          //alert producers that there's space
    return ret;                             //return stored value (-1 if nothing got popped, 0 if something did)
}

int stack_finished(stack *s) {              //runs check to see if consumers need to exit
	int temp;                               //storage value for checking the semaphore
	pthread_mutex_lock(&(s -> mutex));      //lock the stack so nothing can change
	sem_getvalue(&(s->items_avail), &temp); //get the number of items in the stack
	int fin = s->finished;                  //check if all the producers have exited
	pthread_mutex_unlock(&(s -> mutex));    //we've saved values, unlock the mutex so the stack can work again
	if (fin == 1 && temp == 0) {            //check if the stack is empty and all producers have exited
		sem_post(&(s -> items_avail));      //if so, start the chain reaction of consumers exiting
		return 1;                           
	}else { 
		return 0;                           
	}
}

void stack_done(stack *s) {                 //function that gets called when a thread exits, it's a glorified number counter and if statement
	pthread_mutex_lock(&(s -> mutex));      //lock for mutual excusion so two threads don't write to vars at same time
	s->exited++;                            //add one to exited 
	if (s->producers == s->exited) {        //see if the number of threads that have exited is the same as the number of threads that were created.
		s->finished = 1;                    //change the var to say all threads have exited
		sem_post(&(s -> items_avail));      //start the chain reaction of consumer threads exiting
	}
	pthread_mutex_unlock(&(s -> mutex));    //unlock so the stack can be used again
}

void stack_free(stack *s) {                 // free the stack's resources
    pthread_mutex_destroy(&(s -> mutex));   //we used unnamed semaphores so we only have to clear the mutexes memory
}

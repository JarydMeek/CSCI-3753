#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void *producer(void *passedStorage) {
    stack *storage = (stack *) passedStorage;
    char temp[10];
    for (int x = 1; x <= 15; x++) {
        sprintf(temp, "%d", x);
        
        stack_push(storage, temp);
	printf("Pushed -> %s\n", temp);
    }
    stack_done(storage);
}

void *consumer(void *passedStorage) {
    stack *storage = (stack *) passedStorage;
    char temp[10];
    while (1) {
	if (stack_finished(storage) == 0) {
		if (stack_pop(storage, temp) == 0) {
			printf("Popped -> %s\n", temp);
		}
	    } else { 
		sem_post(&(storage -> items_avail)); //signal to any waited consumers to continue, who will then check an if statement and exit.
		break;
		}
	}
}


int main() { 
    stack storage;

    int producers = 5;
    int consumers = 5;

    stack_init(&storage, producers);


    pthread_t ids[producers + consumers];

    for (int i = 0; i<producers; i++) {
	pthread_create(&ids[i], NULL, &producer, (void *) &storage);
	}
    
       for (int i = producers; i<consumers + producers; i++) {
	pthread_create(&ids[i], NULL, &consumer, (void *) &storage);
	}
    
    
   for (int i = 0; i<producers+consumers; i++) {
	pthread_join(ids[i], NULL);
	}

    stack_free(&storage);
}
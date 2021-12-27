#include"array.h"
#include"util.h"
#include"multi-lookup.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/time.h>

int main(int argc, char *argv[]) {
    //Timing Storage and start
    struct timeval start, stop;
	gettimeofday(&start, NULL);

    //Handle Args First

    //Check arguments, if less than 6 we're missing args
    if (argc < 6) {
        printf("Missing arguments\n");
        printf("Proper usage: multi-lookup <# requester> <# resolver> <requester log> <resolver log> [ <data file> ... ]\n");
        exit(-1);
    }
    if (argc > MAX_INPUT_FILES + 5) { //check arguments for maximum 
        fprintf(stderr, "Invalid arguments entered. (Too many files)\n");
        exit(-1);
    }
    //check if we can read number of threads
    int numRequester = atoi(argv[1]);
    if (numRequester < 1 || numRequester > MAX_REQUESTER_THREADS) {
        fprintf(stderr, "Invalid argument entered. (Invalid number of requester threads)\n"); 
        exit(-1);
    }
    int numResolver = atoi(argv[2]);
    if (numResolver < 1 || numResolver > MAX_RESOLVER_THREADS) {
        fprintf(stderr, "Invalid argument entered. (Invalid number of resolver threads)\n"); 
        exit(-1);
    }

    //Check output files, sees if it can open for writing, if it can't throw error, if file doesn't exist, create it.
    FILE *requesterLog = fopen(argv[3], "w+");
     if(requesterLog == NULL) {
        fprintf(stderr,"Requester log could not be opened.\n");
        exit(-1);
     }
    FILE *resolverLog = fopen(argv[4], "w+");
     if(resolverLog == NULL) {
        fprintf(stderr,"Resolver log could not be opened.\n");
        exit(-1);
     }


    //Declare thread-safe stack and initialize
    stack hostnames;
    stack_init(&hostnames, numRequester);
    

    //initialize requester arguments struct
    reqArgs toPassReq;
    toPassReq.storage = &hostnames;
    pthread_mutex_init(&toPassReq.getNewFile, NULL);
    toPassReq.numFiles = argc;
    toPassReq.currFile = 5;
    toPassReq.argv = argv;
    pthread_mutex_init(&toPassReq.reqLogLock, NULL);
    toPassReq.requesterLog = requesterLog;


    //initialize resolver arguments struct
    resArgs toPassRes;
    toPassRes.storage = &hostnames;
    pthread_mutex_init(&toPassRes.resLogLock, NULL);
    toPassRes.resolverLog = resolverLog;


    //array to store thread ids in
    pthread_t ids[numRequester + numResolver];

    //create the appropriate number of requester threads
    for (int i = 0; i<numRequester; i++) {
	    pthread_create(&ids[i], NULL, &requester, (void *) &toPassReq);
	}

    //create the appropriate numbert of resolver threads
    for (int i = numRequester; i<numRequester + numResolver; i++) {
	    pthread_create(&ids[i], NULL, &resolver, (void *) &toPassRes);
	}
    
    //join the threads so we don't exit before the threads complete
    for (int i = 0; i<numRequester+numResolver; i++) {
	    pthread_join(ids[i], NULL);
	}

    //plug the memory holes (clear memory for no leaks)
    stack_free(&hostnames);
    fclose(requesterLog);
    fclose(resolverLog);

    //Timing pt 2.
    gettimeofday(&stop, NULL);
    //calculate and print total time
    printf("%s: total time is %lf seconds\n", argv[0], (stop.tv_sec+(stop.tv_usec/1000000.0))-(start.tv_sec+(start.tv_usec/1000000.0)));
    
}


//Requester threads
void *requester(void *passedStruct) {
    reqArgs *args = (reqArgs *) passedStruct;       //convert the struct back to a struct
    stack *storage = args->storage;                 //create local pointer for ease of typing
    int numFilesServiced = 0;                       //local var to track how many files THIS thread serviced
    char *temp = malloc(MAX_NAME_LENGTH);           //Create memory for the hostname
    while(1) {                                      
        //Pick a file and open it.
        FILE *currentFile;
        pthread_mutex_lock(&args->getNewFile);      //lock to make sure two threads don't open the same file
        if (args->currFile < args->numFiles) {      //check to see if all files have been read
            currentFile = fopen(args->argv[args->currFile], "r");   //open file
            args-> currFile++;                      //log that this file has been opened
        } else {
            pthread_mutex_unlock(&args->getNewFile);//unlock the mutex if all files have been read (prevents deadlock)
            break;                                  //break out of loop, causing thread to exit.
        }
        pthread_mutex_unlock(&args->getNewFile);    //let other threads get a file :)
        
        if(currentFile == NULL) {                   //if file doesn't actually exist or can't be opened
            fprintf(stderr,"invalid file: %s\n", args->argv[args->currFile]);

        } else {                                    //file can be opened, read it.
            numFilesServiced++;
            while (fgets(temp, MAX_NAME_LENGTH, currentFile) != NULL) {
                temp[strcspn(temp, "\n")] = 0;              //this line removes newlines from the temp buffer since they were causing problems.     
                //The logic for above line came from here https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input
                stack_push(storage, temp);                  //push to thread safe buffer
                char *result = malloc(strlen(temp) + 2);    //create storage for hostname plus 2 for newline and null terminator
                strcpy(result, temp);                       //copy the read string to the new space
                strcat(result, "\n");                       //add the newline
                pthread_mutex_lock(&args -> reqLogLock);    //lock the log
                fwrite(result, sizeof(char), strlen(result), args->requesterLog); //write to log
                pthread_mutex_unlock(&args -> reqLogLock);  //unlock log
                free(result);                               //free memory so no memory leaks
                //printf("Pushed -> %s\n", temp);           //debugging line
            }
            fclose(currentFile);            //close the current file before loop to stop memory leaks
        }
        
    }
    free(temp);                         //free the memory of our temp storage for no leaks
    
    printf("thread %lx serviced %d files\n", pthread_self(), numFilesServiced); //print to stdout
    stack_done(storage);                //track how many producers have finished so that we know when consumers can exit.
    return NULL;                        //I know it's a void, this just stops warnings (and does force an exit)
}
//resolver threads
void *resolver(void *passedStruct) {
    resArgs *args = (resArgs *) passedStruct;   //convert the void pointer back to a struct
    stack *storage = args->storage;             //store stack in pointer (not needed tbh)
    char temp[MAX_NAME_LENGTH];                 //temp storage for hostname
    char *output = malloc(MAX_IP_LENGTH);       //temp storage for ip address
    int numResolved = 0;                        //local var to track number of hostnames resolved (or attempted)
    
    while (1) {
        if (stack_finished(storage) == 0) {     //see if it's time to exit
            if (stack_pop(storage, temp) == 0) {//pop and see if there's nothing to pop
                numResolved++;                  //we popped successfully, track that.
                if (dnslookup(temp, output, MAX_IP_LENGTH) == -1) { //preform the lookup
                    strcpy(output,"NOT_RESOLVED");                  //if error, add error to where the ip should be
                } 
                //printf("Popped -> %s\n", temp);                   //debugging nice line
                char *result = malloc(strlen(temp) + strlen(output) + 4); //create storage for our log string plus 4 for the comma, space, newline and null terminator
                strcpy(result, temp);                       //copy the hostname to the log string
                strcat(result, ", ");                       //add comma and space
                strcat(result, output);                     //add the ip address
                strcat(result, "\n");                       //add newline
                pthread_mutex_lock(&args -> resLogLock);    //lock log
                fwrite(result, sizeof(char), strlen(result), args -> resolverLog);  //write to log
                pthread_mutex_unlock(&args -> resLogLock);  //unlock log
                free(result);                               //free log string storage
            }
        } else { 
            sem_post(&(storage -> items_avail)); //if it's time to exit signal to any waited consumers to continue, who will then check an if statement and exit.
            break; //exit loop and therefore proceed exiting thread
        }
	}
    
    free(output);       //free ip address space
    printf("thread %lx serviced %d hostnames\n", pthread_self(), numResolved); //print to stdout
    return NULL;        //exit
}
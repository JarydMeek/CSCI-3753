#ifndef MULTILOOKUP_H
#define MULTILOOKUP_H


//Define Variables
#define MAX_INPUT_FILES 100
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MAX_NAME_LENGTH 255
#define MAX_IP_LENGTH INET6_ADDRSTRLEN


//Struct that gets passed to all requesters
typedef struct {
    stack *storage; //The main thread safe buffer
    pthread_mutex_t getNewFile; //mutex for getting new file to read
    int numFiles; //int to hold number of arguments passed to the program
    int currFile; //int to hold which file we are on (starts at 5 for the other args)
    char **argv; //pass through the arguments, so that we can get file names
    pthread_mutex_t reqLogLock; //mutex to lock the requesterLog file when writing
    FILE *requesterLog; //holds the requesterLog file for logging to.
} reqArgs;

//Struct that gets passed to all resolvers
typedef struct {
    stack *storage; //The main thread safe buffer
    pthread_mutex_t resLogLock; //mutex to lock the resolverLog file when writing
    FILE *resolverLog; //longs the resolverLog file for logging to.
} resArgs;

int main(int argc, char *argv[]);                  // main driver
void *requester(void *passedStruct);
void *resolver(void *passedStruct);

#endif

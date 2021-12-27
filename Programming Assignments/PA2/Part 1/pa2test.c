#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc == 2) {
        int currentFile = open(argv[1], O_RDWR);
        if (currentFile != -1) {
            //We have been passed a file, and we can open it for Read/Write
            //MENU
            while (1 == 1){
                char *input = malloc(100);
                printf("Option (r for read, w for write, s for seek): ");
                int menuStatus = scanf(" %s", input);

                char inputChar = input[0];

                //ctrl d
                if (menuStatus == EOF) {
                    free(input);
                    close(currentFile);
                    return 0;
                }
                //if r is entered
                else if (inputChar == 'r') {
                    printf("Enter the number of bytes you want to read: "); //prompt user
                    int bytes;
                    if (scanf("%d", &bytes) == EOF) {
                        return 1;
                    }
                    char *buffer = calloc(bytes+1, sizeof(char)); //allocate space for string to print
                    read(currentFile, buffer, bytes); //read from file
                    printf("%s\n", buffer); //print
                    free(buffer);
                }
                //if w is entered
                else if (inputChar == 'w') {
                    printf("Enter the string you want to write: ");
                    fflush(stdout);
                    char *buffer = malloc(101); //allocate space for string
                    int charsToWrite = read(0, buffer, 100);
		            buffer[charsToWrite-1] = '\0';
                    write(currentFile, buffer, charsToWrite); //read from file
                    free(buffer);
                }
                //if s is entered
                else if (inputChar == 's') {
                    printf("Enter an offset value: ");
                    int offset;
                    scanf("%d", &offset);
                    printf("Enter a value for whence (0 for SEEK_SET, 1 for SEEK_CUR, 2 for SEEK_END): ");
                    int whence;
                    scanf("%d", &whence);
                    if (whence == 0){
                        lseek(currentFile, offset, SEEK_SET);
                    } else if (whence == 1){
                        lseek(currentFile, offset, SEEK_CUR);
                    } else if (whence == 2){
                        lseek(currentFile, offset, SEEK_END);
                    }
                    
                }


                free(input);
            }

        close(currentFile);

        } else {
            printf("Could not open file.\n");
            return -1;
        }
    } else {
        printf("Invalid arguments, Please enter a file name.");
        return -1;
    }
}
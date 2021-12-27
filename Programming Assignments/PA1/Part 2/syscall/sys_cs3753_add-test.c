#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
        printf("calling 'cs3753_add' system call\n");
	int number1 = 123;	
	int number2 = 456;
	int result = 0;
	
	printf("Number 1: %d\n", number1);
	printf("Number 2: %d\n", number2);
	printf("Expected result: %d\n", number1+number2);
        if (syscall(412, number1, number2, &result) == 0) {
		printf("Result from syscall addition: %d\n", result);
                printf("\n\nsyscall(412) returned 0: Success\n");
        } else {
                printf("\n\nsyscall(412) returned -1: Failure\n");
        }
}
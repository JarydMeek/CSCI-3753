#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main() {
        printf("calling 'helloworld' system call\n");
        if (syscall(411) == 0) {
                printf("syscall(411) returned 0: Success\n");
        } else {
                printf("syscall(411) returned -1: Failure\n");
        }
}

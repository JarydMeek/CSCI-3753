//place file in /arch/x86/kernel/

#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>

asmlinkage long sys_cs3753_add(int number1, int number2, int* result)
{
  printk(KERN_ALERT "Number 1: %d\n", number1);
  printk(KERN_ALERT "Number 2: %d\n", number2);
  int sum = number1 + number2;
  copy_to_user(result, &sum, sizeof(sum));
  printk(KERN_ALERT "Sum: %d\n", sum);  
  return 0;
}


#include<linux/init.h>
#include<linux/module.h>

#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/uaccess.h>

#define BUFFER_SIZE 1024 // buffer size of one kibibit


MODULE_AUTHOR("Jaryd Meek");
MODULE_LICENSE("GPL");

/* Define device_buffer and other global data structures you will need here */

char *device_buffer; // Buffer, will be made to be 1 kibibit in size
int opened = 0; //track the number of times the file has been opened
int closed = 0; //track the number of times the file has been closed


ssize_t pa2_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
    /* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
    /* length is the length of the userspace buffer*/
    /* offset will be set to current position of the opened file after read*/
    /* copy_to_user function: source is device_buffer and destination is the userspace buffer *buffer */
    int numErr;
    int lengthRead;
    //overflow checking
    if (*offset >= BUFFER_SIZE) {
        printk(KERN_ALERT "ERR: offset greater than buffer size.");
        return -1;
    }
    
    numErr = copy_to_user(buffer,*offset + device_buffer, length); //copy read data to user space and return the number that couldn't be copied.
    lengthRead = length - numErr;
    *offset += lengthRead; //change the offset so we're looking in the right place, offset by the number of bytes read (amount asked to read - amount that couldn't be read)
    printk(KERN_ALERT "%d bytes have been read from buffer.\n", lengthRead);
    return lengthRead;
}



ssize_t pa2_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
    /* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
    /* length is the length of the userspace buffer*/
    /* current position of the opened file*/
    /* copy_from_user function: destination is device_buffer and source is the userspace buffer *buffer */
    int numErr;
    int lengthWritten;
    //overflow checking
    //if we're already over buffer or if we will be over buffer after writing.
    if (*offset >= BUFFER_SIZE) {
        printk(KERN_ALERT "ERR: offset greater than buffer size.\n");
        return -1;
    }
    if (*offset + length >= BUFFER_SIZE || *offset + length < 0) {
        printk(KERN_ALERT "ERR: attempted to write outside buffer (either above or below).\n");
        return -1;
    }
    numErr = copy_from_user(device_buffer + *offset, buffer, length); //copy write data from user space and return the number that couldn't be copied.
    lengthWritten = (length - numErr);
    *offset += lengthWritten; //change the offset so we're looking in the right place, offset by the number of bytes written (amount asked to write - amount that couldn't be written)
    printk(KERN_ALERT "%d bytes have been written to buffer.\n", lengthWritten);
    return lengthWritten;
}


int pa2_char_driver_open (struct inode *pinode, struct file *pfile)
{
    /* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
    opened++; //incriment counter
    printk(KERN_ALERT "Device is open, and has been opened %d times.\n", opened); //print open message and number of times opened
    return 0;
}

int pa2_char_driver_close (struct inode *pinode, struct file *pfile)
{
    /* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
    closed++; //incriment counter
    printk(KERN_ALERT "Device is closed, and has been closed %d times.\n", closed); //print close message and number of times closed
    return 0;
}

loff_t pa2_char_driver_seek (struct file *pfile, loff_t offset, int whence) {
    
    /* Update open file position according to the values of offset and whence */
    loff_t newOffset;
    
    if (whence == 0) {//set
        newOffset = offset;
    } else if (whence == 1) {//curr
        newOffset = pfile->f_pos + offset;
    } else if (whence == 2) { //end
        newOffset = BUFFER_SIZE - offset;
    } else {
        return -1;
    }
    

    //overflow checking, we don't want to go over the length of the buffer 😉
    if (newOffset >= BUFFER_SIZE || newOffset < 0){
        printk(KERN_ALERT "ERR: Attempting to offset outside of buffer.\n");
    return -1;
    }

    pfile -> f_pos = newOffset; //update offset on file object
    return newOffset;
}

struct file_operations pa2_char_driver_file_operations = {
    .owner   = THIS_MODULE,
    .open    = pa2_char_driver_open,       // int my_open  (struct inode *, struct file *);
    .release = pa2_char_driver_close,      // int my_close (struct inode *, struct file *);
    .read    = pa2_char_driver_read,       // ssize_t my_read  (struct file *, char __user *, size_t, loff_t *);
    .write   = pa2_char_driver_write,      // ssize_t my_write (struct file *, const char __user *, size_t, loff_t *);
    .llseek  = pa2_char_driver_seek        // loff_t  my_seek  (struct file *, loff_t, int);
};

static int pa2_char_driver_init(void)
{
    /* print to the log file that the init function is called.*/
    /* register the device */
    
    register_chrdev(240, "pa2_char_driver", &pa2_char_driver_file_operations);
    device_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    printk(KERN_ALERT "PA2 Character Driver has been initialized.\n");
    return 0;
}

static void pa2_char_driver_exit(void)
{
    /* print to the log file that the exit function is called.*/
    /* unregister  the device using the register_chrdev() function. */
    unregister_chrdev(240, "pa2_char_driver");
    kfree(device_buffer);
    printk(KERN_ALERT "PA2 Character Driver has exited.\n");
}

/* add module_init and module_exit to point to the corresponding init and exit function*/


module_init(pa2_char_driver_init);
module_exit(pa2_char_driver_exit);
        


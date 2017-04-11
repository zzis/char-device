#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#define MAJOR_NUMBER 61

#define DEVICE_SIZE 4 * 1024 * 1024
 
/* forward declaration */
static int onebyte_open(struct inode *inode, struct file *filep);
static int onebyte_release(struct inode *inode, struct file *filep);
static ssize_t onebyte_read(struct file *filep, char *buf, size_t count, loff_t *f_pos);
static ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos);
static void onebyte_exit(void);
/* definition of file_operation structure */
struct file_operations onebyte_fops = {
     read:     onebyte_read,
     write:    onebyte_write,
     open:     onebyte_open,
     release: onebyte_release
};
static char *onebyte_data = NULL;


static int onebyte_open(struct inode *inode, struct file *filep)
{
     return 0; // always successful
}

static int onebyte_release(struct inode *inode, struct file *filep)
{
     return 0; // always successful
}

static ssize_t onebyte_read(struct file *filep, char *buf, size_t count, loff_t *f_pos)
{    /*please complete the function on your own*/

    // if(copy_to_user(buf, onebyte_data, 1))
    // {
    //     count = -EFAULT;
    // }
    // *f_pos += 1;
    // if(*f_pos > 1)
    // {
    //     return 0;
    // }
    // return 1;

    if(*f_pos >= DEVICE_SIZE){
        return 0;
    }
    
    if(count + *f_pos > DEVICE_SIZE){
        count = DEVICE_SIZE - *f_pos;
    }

    if(copy_to_user(buf, onebyte_data + *f_pos, count)){
        return -EFAULT;
    }

    *f_pos = *f_pos + count;
    return count;
}

static ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos)
{
     /*please complete the function on your own*/
    //  if(*f_pos == 0) {
    //      if(copy_from_user(onebyte_data, buf, 1))
    //      {
    //          return -EFAULT;
    //      }
    //      *f_pos += 1;
    //      return 1;
    //  } else {
    //      //here should print error mesg
    //      // ENOSPC      28  is defined in error.h means /* No space left on device */
    //     return -ENOSPC;
    //  }

    if(*f_pos >= DEVICE_SIZE){
        return -ENOSPC;
    }

    if(count + *f_pos > DEVICE_SIZE){
        count = DEVICE_SIZE - *f_pos;
    }

    if(copy_from_user(onebyte_data + *f_pos, buf, count)){
        return -EFAULT;
    }

    *f_pos += count;
    return count;

}

static int onebyte_init(void)
{
     int result;
     // register the device
     result = register_chrdev(MAJOR_NUMBER, "onebyte", &onebyte_fops);
     if (result < 0) {
         return result;
     }    
     // allocate one byte of memory for storage
     // kmalloc is just like malloc, the second parameter is
// the type of memory to be allocated.
     // To release the memory allocated by kmalloc, use kfree.
    //  onebyte_data = kmalloc(sizeof(char), GFP_KERNEL);
    onebyte_data = kmalloc(DEVICE_SIZE, GFP_KERNEL);
     if (!onebyte_data) {
          onebyte_exit();
          // cannot allocate memory
          // return no memory error, negative signify a failure
         return -ENOMEM;
     }
     // initialize the value to be X
    //  *onebyte_data = 'X';
     printk(KERN_ALERT "This is a onebyte device module\n");
     return 0;
}

static void onebyte_exit(void)
{
     // if the pointer is pointing to something
     if (onebyte_data) {
          // free the memory and assign the pointer to NULL
          kfree(onebyte_data);
          onebyte_data = NULL;
     }
     // unregister the device
     unregister_chrdev(MAJOR_NUMBER, "onebyte");
     printk(KERN_ALERT "Onebyte device module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(onebyte_init);
module_exit(onebyte_exit);
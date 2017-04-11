#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/file.h>
#include <linux/ioctl.h>
#define MAJOR_NUMBER 61

#define DEVICE_SIZE 4 * 1024 * 1024
#define SCULL_IOC_MAGIC 'k'
#define SCULL_HELLO _IO(SCULL_IOC_MAGIC, 1)
#define SET_DEV_MSG _IOW(SCULL_IOC_MAGIC, 2, char*)
#define GET_DEV_MSG _IOR(SCULL_IOC_MAGIC, 3, char*)
#define SCULL_IOC_MAXNR 3

#define DEV_MSG_SIZE 60
 
/* forward declaration */
static int onebyte_open(struct inode *inode, struct file *filep);
static int onebyte_release(struct inode *inode, struct file *filep);
static ssize_t onebyte_read(struct file *filep, char *buf, size_t count, loff_t *f_pos);
static ssize_t onebyte_write(struct file *filep, const char *buf, size_t count, loff_t *f_pos);
static void onebyte_exit(void);
// static int onebyte_llseek(unsigned int fd, unsigned long offset_high, unsigned long offset_low, loff_t *result, unsigned int whence);
static loff_t onebyte_llseek(struct file *filp, loff_t offset, int whence);
long onebyte_ioctl(struct file *filp, unsigned int cmd, unsigned long arg); 
/* definition of file_operation structure */
struct file_operations onebyte_fops = {
     llseek :   onebyte_llseek,
     read :     onebyte_read,
     write :    onebyte_write,
     open :     onebyte_open,
     release : onebyte_release,
     unlocked_ioctl : onebyte_ioctl,
};
static char *onebyte_data = NULL;
static char *dev_msg = NULL;


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
    //print the size written
    // printk(KERN_ALERT "The writen size is %d", *f_pos);

    return count;

}

static loff_t onebyte_llseek(struct file *filp, loff_t offset, int whence)  
{  
    loff_t newpos;  
  
    switch(whence) {  
      case 0: /* SEEK_SET */  
        newpos = offset;  
        break;  
  
      case 1: /* SEEK_CUR */  
        newpos = filp->f_pos + offset;  
        break;  
  
      case 2: /* SEEK_END */  
        newpos = DEVICE_SIZE -1 + offset;  
        break;  
  
      default: /* can't happen */  
        return -EINVAL;  
    }  
    if ((newpos<0) || (newpos>DEVICE_SIZE))  
        return -EINVAL;  
  
    filp->f_pos = newpos;  
    return newpos;  
}

long onebyte_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    int err = 0;
    int retval = 0;

    if(_IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
    if(_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;

    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
    if (err) 
        return -EFAULT;

    switch(cmd){
        case SCULL_HELLO:
            printk(KERN_WARNING "hello from ioctl\n");
            break;
        case SET_DEV_MSG:
            if(copy_from_user(dev_msg, (char *)arg, DEV_MSG_SIZE)){
                return -EFAULT;
            }
            printk(KERN_ALERT "set dev msg: %s", dev_msg);
            break;
        case GET_DEV_MSG:
            if(copy_to_user((char *)arg, dev_msg, DEV_MSG_SIZE)){
                return -EFAULT;
            }
            printk(KERN_ALERT "get dev msg: %s", dev_msg);
            break;
        default:
            return -ENOTTY;
    }
    return retval;
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

     dev_msg = kmalloc(DEV_MSG_SIZE, GFP_KERNEL);
     if(!dev_msg){
         onebyte_exit();
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
     if(dev_msg){
         kfree(dev_msg);
         dev_msg = NULL;
     }
     // unregister the device
     unregister_chrdev(MAJOR_NUMBER, "onebyte");
     printk(KERN_ALERT "Onebyte device module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(onebyte_init);
module_exit(onebyte_exit);

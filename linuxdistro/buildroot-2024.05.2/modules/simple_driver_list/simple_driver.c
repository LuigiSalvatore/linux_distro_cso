#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h>

#define DEVICE_NAME "simple_driver"
#define CLASS_NAME  "simple_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sérgio Johann Filho");
MODULE_DESCRIPTION("A generic Linux char driver.");
MODULE_VERSION("0.1.0");

static int majorNumber;
static int number_opens = 0;
static struct class *charClass = NULL;
static struct device *charDevice = NULL;


#define MSG_SIZE	128

struct message_s {
	struct list_head link;
	char message[MSG_SIZE];
	short size;
};

struct list_head list;

int list_add_entry(char *data)
{
	struct message_s *new_node = kmalloc((sizeof(struct message_s)), GFP_KERNEL);
	
	if (!new_node) {
		printk(KERN_INFO "Memory allocation failed, this should never fail due to GFP_KERNEL flag\n");
		
		return 1;
	}
	strcpy(new_node->message, data);
	new_node->size = strlen(data);
	list_add_tail(&(new_node->link), &list);
	
	return 0;
}

void list_show(void)
{
	struct message_s *entry = NULL;
	int i = 0;
	
	list_for_each_entry(entry, &list, link) {
		printk(KERN_INFO "Message #%d: %s\n", i++, entry->message);
	}
}

int list_delete_head(void)
{
	struct message_s *entry = NULL;
	
	if (list_empty(&list)) {
		printk(KERN_INFO "Empty list.\n");
		
		return 1;
	}
	
	entry = list_first_entry(&list, struct message_s, link);
	
	list_del(&entry->link);
	kfree(entry);
		
	return 0;
}

int list_delete_entry(char *data)
{
	struct message_s *entry = NULL;
	
	list_for_each_entry(entry, &list, link) {
		if (strcmp(entry->message, data) == 0) {
			list_del(&(entry->link));
			kfree(entry);
			
			return 0;
		}
	}
	
	printk(KERN_INFO "Could not find data.");
	
	return 1;
}


static int	dev_open(struct inode *, struct file *);
static int	dev_release(struct inode *, struct file *);
static ssize_t	dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t	dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

static int simple_init(void)
{
	printk(KERN_INFO "Simple Driver: Initializing the LKM\n");

	// Try to dynamically allocate a major number for the device -- more difficult but worth it
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber < 0) {
		printk(KERN_ALERT "Simple Driver failed to register a major number\n");
		return majorNumber;
	}
	
	printk(KERN_INFO "Simple Driver: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	charClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charClass)) {		// Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Simple Driver: failed to register device class\n");
		return PTR_ERR(charClass);	// Correct way to return an error on a pointer
	}
	
	printk(KERN_INFO "Simple Driver: device class registered correctly\n");

	// Register the device driver
	charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(charDevice)) {		// Clean up if there is an error
		class_destroy(charClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Simple Driver: failed to create the device\n");
		return PTR_ERR(charDevice);
	}
	
	printk(KERN_INFO "Simple Driver: device class created.\n");
	
	INIT_LIST_HEAD(&list);
		
	return 0;
}

static void simple_exit(void)
{
	device_destroy(charClass, MKDEV(majorNumber, 0));
	class_unregister(charClass);
	class_destroy(charClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);
	printk(KERN_INFO "Simple Driver: goodbye.\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
	number_opens++;
	printk(KERN_INFO "Simple Driver: device has been opened %d time(s)\n", number_opens);
	printk("Process id: %d, name: %s\n", (int) task_pid_nr(current), current->comm);

	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
	int error = 0;
	struct message_s *entry = list_first_entry(&list, struct message_s, link);
   
	if (list_empty(&list)) {
		printk(KERN_INFO "Simple Driver: no data.\n");
		
		return 0;
	}	
	
	// copy_to_user has the format ( * to, *from, size) and returns 0 on success
	error = copy_to_user(buffer, entry->message, entry->size);

	if (!error) {				// if true then have success
		printk(KERN_INFO "Simple Driver: sent %d characters to the user\n", entry->size);
		list_delete_head();
		
		return 0;
	} else {
		printk(KERN_INFO "Simple Driver: failed to send %d characters to the user\n", error);
		
		return -EFAULT;			// Failed -- return a bad address message (i.e. -14)
	}
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	if (len < MSG_SIZE) {
		list_add_entry(buffer);
		list_show();

		printk(KERN_INFO "Simple Driver: received %zu characters from the user\n", len);
		
		return len;
	} else {
		printk(KERN_INFO "Simple Driver: too many characters to deal with (%d)\n", len);
		
		return 0;
	}
}

static int dev_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_INFO "Simple Driver: device successfully closed\n");

	return 0;
}

module_init(simple_init);
module_exit(simple_exit);

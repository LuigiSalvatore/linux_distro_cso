#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sérgio Johann Filho");
MODULE_DESCRIPTION("A very simple kernel module.");
MODULE_VERSION("0.0.1");
static int mymodule_init(void)
{
printk(KERN_EMERG "KERN_EMERG: Used for emergency messages, usually those that precede a crash.");
printk(KERN_ALERT "KERN_ALERT: A situation requiring immediate action.");
printk(KERN_CRIT "KERN_CRIT: Critical conditions, often related to serious hardware or software failures.");
printk(KERN_ERR "KERN_ERR: Used to report error conditions; device drivers often use KERN_ERR to report hardware difficulties.");
printk(KERN_WARNING "KERN_WARNING: Warnings about problematic situations that do not, in themselves, create serious problems with the system.");
printk(KERN_NOTICE "KERN_NOTICE: Situations that are normal, but still worthy of note. A number of security-related conditions are reported at this level.");
printk(KERN_INFO "KERN_INFO: Informational messages. Many drivers print information about the hardware they find at startup time at this level."); 
printk(KERN_DEBUG "KERN_DEBUG: Used for debugging messages.");
return 0;
}
static void mymodule_exit(void)
{
printk(KERN_INFO "mymodule unloaded.\n");
}
module_init(mymodule_init);
module_exit(mymodule_exit);
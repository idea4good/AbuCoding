#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define DRIVER_AUTHOR "Your Name <your.email@example.com>"
#define DRIVER_DESC   "A simple module to print a variable's address"

// Initialization function
static int __init hello_init(void) {
    static int my_var = 42;
    printk(KERN_INFO "Hello, World!\n");
    printk(KERN_INFO "Address of my_var: %p\n", (void *)&my_var);
    return 0;  // Non-zero return means that the module couldn't be loaded.
}

// Exit function
static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye, World!\n");
}

// Macros for registering module entry and exit points
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("0.1");

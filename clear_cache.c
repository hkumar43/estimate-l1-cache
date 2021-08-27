#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include<linux/slab.h>

static int __init start_clear(void){

	asm volatile ( "WBINVD\n\t" );
	return 0;

}

static void __exit end_clear(void){
	printk(KERN_INFO "CACHE CLEARED");
}

module_init(start_clear);

module_exit(end_clear);



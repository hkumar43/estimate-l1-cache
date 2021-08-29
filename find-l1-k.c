#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>
#include <linux/sched.h>
#include <linux/slab.h>


//Set using the data observed
#define HIT_LATENCY_BOUND 5 

#define LOG_LINE_WIDTH 10

static int __init start(void){
	unsigned high0,low0,high1,low1;
	uint64_t start,end;
        unsigned long flags;

	//This is the large array that will be used to estimate the cache  hit and miss.
	//Assuming the largest cache tested will not exceed n integers' size.
	uint64_t N = 100000;
	uint16_t islands = 4;
	uint64_t dist = 8192;
	uint64_t n = 32;
	uint64_t  *array;

        //printk(KERN_INFO "Creating test array\n");

        array = kmalloc(N*sizeof(uint64_t),GFP_KERNEL);	

	
	int i = 0;
        
	//Fire the instruction for  clearing the caches.
        asm volatile ( "WBINVD\n\t"); 
	
	printk(KERN_INFO "GRAPH_DATA STARTS");

	//Getting the instruction cache with the required instructions
        asm volatile (
				"CPUID\n\t"
				"RDTSC\n\t"
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t": "=r" (high0), "=r" (low0):: "%rax","%rbx","%rcx","%rdx"
				);

	asm volatile (
				"RDTSCP\n\t"
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t" "CPUID\n\t": "=r" (high1), "=r" (low1):: "%rax","%rbx","%rcx","%rdx");
	start = ( ((uint64_t)high0 << 32) | low0 );
	end = ( ((uint64_t)high1 << 32) | low1 );

	asm volatile (
				"CPUID\n\t"
				"RDTSC\n\t"
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t": "=r" (high0), "=r" (low0):: "%rax","%rbx","%rcx","%rdx"
				);

	asm volatile (
				"RDTSCP\n\t"
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t" "CPUID\n\t": "=r" (high1), "=r" (low1):: "%rax","%rbx","%rcx","%rdx");
	start = ( ((uint64_t)high0 << 32) | low0 );
	end = ( ((uint64_t)high1 << 32) | low1 );
        /******************************/
      
        //Find the overhead of other instructions
	asm volatile (
				"CPUID\n\t"
				"RDTSC\n\t"
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t": "=r" (high0), "=r" (low0):: "%rax","%rbx","%rcx","%rdx"
				);

	asm volatile (
				"RDTSCP\n\t"
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t" "CPUID\n\t": "=r" (high1), "=r" (low1):: "%rax","%rbx","%rcx","%rdx");
	start = ( ((uint64_t)high0 << 32) | low0 );
	end = ( ((uint64_t)high1 << 32) | low1 );

        int overhead = end - start;

       	
       
        // Code for determining Block Size.
	
	for(i=0;i<islands;i++){
		int j=0;
		for(;j<n;j++){
			uint16_t index = i*dist+j;
			preempt_disable();
			raw_local_irq_save(flags);
		
		    	
			asm volatile (
					"CPUID\n\t"
					"RDTSC\n\t"
					"mov %%edx, %0\n\t"
					"mov %%eax, %1\n\t": "=r" (high0), "=r" (low0):: "%rax","%rbx","%rcx","%rdx"
					);

			volatile uint64_t a = array[index];

			

			asm volatile (
					"RDTSCP\n\t"
					"mov %%edx, %0\n\t"
					"mov %%eax, %1\n\t" "CPUID\n\t": "=r" (high1), "=r" (low1):: "%rax","%rbx","%rcx","%rdx");
			start = ( ((uint64_t)high0 << 32) | low0 );
			end = ( ((uint64_t)high1 << 32) | low1 );

			raw_local_irq_restore(flags);
			preempt_enable();

			int elapsed =  end - start - overhead;


			printk(KERN_INFO "GRAPH_DATA %d",elapsed);

		}


		printk(KERN_INFO "GRAPH_DATA ISLAND_COMPLETED");

	}

        printk(KERN_INFO "GRAPH_DATA BLOCK_DATA_DONE");

	


        
	//Free all memory
	kfree(array);

	return 0;
}


static void __exit end(void){
	printk(KERN_INFO "GRAPH_DATA PROGRAM_COMPLETED\n");
}

module_init(start);

module_exit(end);



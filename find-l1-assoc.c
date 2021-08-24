#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>
#include <linux/sched.h>
#include <linux/slab.h>



static int __init start(void){
	unsigned high0,low0,high1,low1;
	uint64_t start,end;
        unsigned long flags;

	//This is the large array that will be used to estimate the cache  hit and miss.
	//Assuming the largest cache tested will not exceed n integers' size.
	uint64_t N = 100000;
	uint16_t islands = 5;
	uint64_t dist = 8192;
	uint64_t n = 32;
	uint64_t  *array;
        int *latency;
	char *types; //Will be storing for H for Hit and M for Miss.

        array = kmalloc(N*sizeof(uint64_t),GFP_KERNEL);	
        latency = kmalloc(N*sizeof(int),GFP_KERNEL); // Data type is int because it can be negative.
        types = kmalloc(N*sizeof(char),GFP_KERNEL);

	int i = 0;
	for(;i<N;i++)
		array[i] = i;
        
	printk(KERN_INFO "GRAPH_DATA STARTS");

	//Getting the  required instructions in the instruction cache.
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

       	
       
        //Code for finding the associativity.

	for(i=0;i<6400;i += 8){
                        preempt_disable();
			raw_local_irq_save(flags);
		
		    	
			asm volatile (
					"CPUID\n\t"
					"RDTSC\n\t"
					"mov %%edx, %0\n\t"
					"mov %%eax, %1\n\t": "=r" (high0), "=r" (low0):: "%rax","%rbx","%rcx","%rdx"
					);

			//The volatile keyword is for not doing optimizations with this instruction.
		
			volatile uint64_t a = array[i];

			

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

	printk(KERN_INFO "GRAPH_DATA ASSOC_DATA_DONE");
       
        printk(KERN_INFO "%x %x",array,array+1);	
	
	//Free all memory
	kfree(array);
        kfree(latency);
	kfree(types);

	return 0;
}


static void __exit end(void){
	printk(KERN_INFO "GRAPH_DATA PROGRAM_COMPLETED\n");
}

module_init(start);

module_exit(end);



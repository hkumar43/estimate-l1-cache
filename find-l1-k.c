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
	uint64_t n = 200;
	uint64_t  *array;

        //printk(KERN_INFO "Creating test array\n");

        array = kmalloc(N*sizeof(uint64_t),GFP_KERNEL);	

	
        asm volatile ( "WBINVD\n\t"); //Clear all cache 
	
	int i = 0;
	volatile int b;
	for(;i<N;i++)              //Read all elements once so that they are in L2, and the most recent ones in L1.
		b = array[i];
        
	
	printk(KERN_INFO "PROGRAM_STARTS");

	//Getting the instruction cache filled with the required instructions
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
      
        //Find the overhead of instructions used to capture the latencies.
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
        /*********************************/
       	
       
        // Code for printing the latencies for continuous reads from an array - adjacent memory locations .
	
		int j=0;
		for(;j<n;j++){
			preempt_disable();  //So that the processor can't be preempted by another process
			raw_local_irq_save(flags);  //Disbale hard interrupts on the CPU.
		
		    	
			asm volatile (
					"CPUID\n\t"
					"RDTSC\n\t"
					"mov %%edx, %0\n\t"
					"mov %%eax, %1\n\t": "=r" (high0), "=r" (low0):: "%rax","%rbx","%rcx","%rdx"
					);

			volatile uint64_t a = array[j];   //We use volatile so that the compiler does not remove the statement.

			

			asm volatile (
					"RDTSCP\n\t"
					"mov %%edx, %0\n\t"
					"mov %%eax, %1\n\t" "CPUID\n\t": "=r" (high1), "=r" (low1):: "%rax","%rbx","%rcx","%rdx");
			start = ( ((uint64_t)high0 << 32) | low0 );
			end = ( ((uint64_t)high1 << 32) | low1 );

			raw_local_irq_restore(flags);
			preempt_enable();

			int elapsed =  end - start - overhead;


			printk(KERN_INFO "%d",elapsed);

		}

	//Free all memory
	kfree(array);

	return 0;
}


static void __exit end(void){
	printk(KERN_INFO "PROGRAM_COMPLETED\n");
}

module_init(start);

module_exit(end);

MODULE_LICENSE("GPL");

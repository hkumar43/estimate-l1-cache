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
	uint64_t n = 32;
	uint64_t  *array;
        int *latency;
	char *types; //Will be storing for H for Hit and M for Miss.

        printk(KERN_INFO "Creating test array\n");

        array = kmalloc(n*sizeof(uint64_t),GFP_KERNEL);	
        latency = kmalloc(n*sizeof(int),GFP_KERNEL); // Data type is int because it can be negative.
        types = kmalloc(n*sizeof(char),GFP_KERNEL);

        uint64_t amem = array;
        uint64_t rem = amem >> 5;
	printk(KERN_INFO "Array pointer size : %x",array);	 
	printk(KERN_INFO "Remainder from 32B : %x", rem);
	int i = 0;
	for(;i<n;i++)
		array[i] = i;

      
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

       	
        
	for(i=0;i<n;i++){

		preempt_disable();
		raw_local_irq_save(flags);
	
		
		asm volatile (
				"CPUID\n\t"
				"RDTSC\n\t"
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t": "=r" (high0), "=r" (low0):: "%rax","%rbx","%rcx","%rdx"
				);

		//code whose latency we have to measure.
		//if(array[i] < 0) 
			
			//Just some code which will never be executed. My intension is to just read the data and execute a compare instruction. I am doing a simple thing like a = array[i], becasue due to the compiler, this code may completely be removed and not be executed.
		//	array[i] = -array[i] ;
		
                //The volatile keyword is for not doing optimizations with this instruction.
		volatile int a = array[i];

		//Find the maximum and the minimum latencies.
                

		asm volatile (
				"RDTSCP\n\t"
				"mov %%edx, %0\n\t"
				"mov %%eax, %1\n\t" "CPUID\n\t": "=r" (high1), "=r" (low1):: "%rax","%rbx","%rcx","%rdx");
		start = ( ((uint64_t)high0 << 32) | low0 );
		end = ( ((uint64_t)high1 << 32) | low1 );

		raw_local_irq_restore(flags);
		preempt_enable();

		int elapsed =  end - start - overhead;

		latency[i] = elapsed;

		printk(KERN_INFO "%d",elapsed);

                types[i] = elapsed > HIT_LATENCY_BOUND ? 'H' : 'M'; 


	}
        
	char type = 'M';
	uint32_t running_count = 0;
	for( i=0;i<n;i++){
		char prev_type = type;
	        if(latency[i] < HIT_LATENCY_BOUND){
                    type = 'H';
		    if(prev_type != type){
			    printk(KERN_INFO "%uM ",running_count);
			    running_count = 1;
		    }
		    else running_count++;
		}
	        else{
		    type = 'M';	   
		    if(prev_type != type){
			    printk(KERN_INFO "%uH ",running_count);
			    running_count = 1;
		    }
		    else running_count++;
 
		}


	}

        printk(KERN_INFO "%u%c",running_count,type);	


	//Free all memory
	kfree(array);
        kfree(latency);
	kfree(types);

	return 0;
}


static void __exit end(void){
	printk(KERN_INFO "PROGRAM COMPLETED\n");
}

module_init(start);

module_exit(end);



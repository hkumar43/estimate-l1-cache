#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>
#include <linux/sched.h>
#include <linux/slab.h>

#define NON_EVICT_LATENCY_BOUND 30 

static int __init start(void){
	unsigned high0,low0,high1,low1;
	uint64_t start,end;
        unsigned long flags;

	//This is the large array that will be used to estimate the cache  hit and miss.
	//Assuming the largest cache tested will not exceed n integers' size.
	uint64_t N = 200000;
	uint64_t  *array;
	char *types; //Will be storing for H for Hit and M for Miss.

        array = kmalloc(N*sizeof(uint64_t),GFP_KERNEL);	
        //latency = kmalloc(N*sizeof(int),GFP_KERNEL); // Data type is int because it can be negative.
        //types = kmalloc(N*sizeof(char),GFP_KERNEL);

	int i = 0;
	//for(;i<N;i++)
	//	array[i] = i;
        
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
	//for(k=1;k<=128;k*=2){
        //printk(KERN_INFO "Skipping %d sets",k);


	int size = 1024; //in cache clocks each of size 64B
	int sets = 1024;
        for(;sets >= 1; sets /= 2){	
	        int assoc = size/sets ;
                int incr = 8*sets; 
		printk(KERN_INFO "Check for assoc : %d", assoc);
                
		asm volatile ( "WBINVD\n\t" );
                
		for(i=0;i<incr*(assoc+4);i += incr){


                        
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

                        //latency[i] = elapsed;
                        //if(elapsed > 50) printk(KERN_INFO "GRAPH_DATA HIGH");
		       	printk(KERN_INFO "GRAPH_DATA %d : %d",i ,elapsed);
			

                }

	}
	//}

	//printk(KERN_INFO "GRAPH_DATA ASSOC_DATA_DONE");
       
        //printk(KERN_INFO "%x %x",array,array+1);	
       /* 
	int j;	
	char type = 'M';
		uint32_t running_count = 0;
		int blocks_count = 0;

		for( j=0;j<=32000;j+=k*8){
			char prev_type = type;
			if(latency[j] < NON_EVICT_LATENCY_BOUND){
			    type = 'H';
			    if(prev_type != type){
				    printk(KERN_INFO "%uM ",running_count);
				    running_count = 1;
			    }
			    else running_count++;
			}
			else{
			    type = 'M';	   
			    //printk(KERN_INFO "Miss - %d",latency[j]);
			    if(prev_type != type){
				    printk(KERN_INFO "%uH ",running_count);
				    running_count = 1;
			    }
			    else running_count++;
	 
			}

                        //if(latency[j] < 100) 
			//    blocks_count++;

                        //printk(KERN_INFO "%d :  %d",latency[j],blocks_count);


		 }

       		 printk(KERN_INFO "%u%c",running_count,type);	
        }*/

	//Free all memory
	kfree(array);

	return 0;
}


static void __exit end(void){
	printk(KERN_INFO "GRAPH_DATA PROGRAM_COMPLETED\n");
}

module_init(start);

module_exit(end);



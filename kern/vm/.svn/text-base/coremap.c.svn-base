#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/limits.h>
#include <vfs.h>
#include <vnode.h>
#include <coremap.h>
#include <curthread.h>
#include <thread.h>
#include <lib.h>
#include <vm.h>
#include <machine/spl.h>


static struct cm_entry * coremap;
static unsigned int cm_start;
static unsigned int cm_end;
static unsigned int freepaddr;


void cm_bootstrap(){
	coremap_ready = 0;
	paddr_t firstpaddr;
	paddr_t lastpaddr;
	ram_getsize(&firstpaddr, &lastpaddr);

	assert((firstpaddr & PAGE_FRAME) == firstpaddr);
	assert((lastpaddr & PAGE_FRAME) == lastpaddr);

	//total number of pages available on ram
	unsigned int num_pages = (lastpaddr - firstpaddr)/PAGE_SIZE; 
	//total memory of coremap array
	unsigned int cm_size = num_pages * sizeof(struct cm_entry);
	cm_size = ROUNDUP(cm_size, PAGE_SIZE);

	//allocate coremap
	coremap = (struct cm_entry * ) PADDR_TO_KVADDR (firstpaddr);
	//determine the start of free addresses
	freepaddr = firstpaddr + cm_size;
	assert(freepaddr < lastpaddr);

	//cm_start = freepaddr/PAGE_SIZE;
	//cm_end	= lastpaddr /PAGE_SIZE - cm_start;
	
	//determine size of total pages usable on coremap
	cm_end	= (lastpaddr-freepaddr) /PAGE_SIZE;
	//coremap = (struct cm_entry **) kmalloc (sizeof(struct cm_entry*) * cm_size);
	
	if (coremap == NULL){
		panic("core map not allocated\n");
	}

	int i;

	for(i=0; i < cm_end ; i++){
		//coremap[i]= (struct cm_entry*) kmalloc (sizeof(struct cm_entry));
		//if(i< cm_end){
			coremap[i].p_addr = freepaddr + (i* PAGE_SIZE);
			coremap[i].v_addr = (vaddr_t) PADDR_TO_KVADDR (coremap[i].p_addr);
			coremap[i].free = 1;
			//coremap[i].fixed = 0;
			coremap[i].length = 0;
		//}
		// else {
		// 	//kernel
		// 	coremap[i].free = 0;
		// 	coremap[i].kernel = 1;
		// }
	}

	coremap_ready = 1;
}

paddr_t cm_getppages (int npages){
	assert(coremap_ready);

	int i, block_start;
	int free_count = 0; 
	//loop through coremap to look for free entries
	for(i=0 ; i< cm_end ; i++){
		if(coremap[i].free){
			free_count++;
			if(free_count ==1){
				block_start = i; //indicates the start of the memory block
			}
			//FOUND ALL REQUIRED ENTRIES
			if(free_count == npages){
				coremap[block_start].length = npages;
				int j;
				//loop through block to occupy each entry
				for(j=block_start; j <= i ; j++){
					coremap[j].free = 0;

				}

				return coremap[block_start].p_addr;
			}
		}
		else{
			free_count = 0;
		}

	}
	return 0;

}

void cm_freekpages(vaddr_t addr){
	//convert va to pa
	paddr_t p_address = KVADDR_TO_PADDR(addr);
	int cm_index = (p_address - freepaddr)/PAGE_SIZE;

	int spl = splhigh();
	assert(cm_index < cm_end);
	assert(coremap[cm_index].length > 0);
			//coremap[i].fixed = 0;

	int i;
	//ALTERNATIVE: i = 0
	for(i = cm_index; i < coremap[cm_index].length ; i++){
		coremap[i].free = 1;
	}
	coremap[cm_index].length = 0;
	splx(spl);

}

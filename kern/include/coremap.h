#ifndef _MIPS_COREMAP_H_
#define _MIPS_COREMAP_H_

#include <machine/vm.h>

//structure for coremap
struct cm_entry {
	paddr_t p_addr;
	vaddr_t v_addr;
	//int fixed; //indicates that entry belongs to kernel
	int free; //1 = free , 0 = used
	int length;
};

//static struct cm_entry * coremap;
void cm_bootstrap();
int coremap_ready;
paddr_t cm_getppages (int npages);
void cm_freekpages(vaddr_t addr);


#endif

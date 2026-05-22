/*
 * Copyright (C) 2026 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Caitoa release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

//#ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "../include/mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = pvma->vm_id;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    pvma = pvma->vm_next;
    vmait = pvma->vm_id;
  }

  return pvma;
}

int __mm_swap_page(struct pcb_t *caller, addr_t vicfpn , addr_t swpfpn)
{
    __swap_cp_page(caller->krnl->mram, vicfpn, caller->krnl->active_mswp, swpfpn);
    return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, addr_t size, addr_t alignedsz)
{
  struct vm_rg_struct * newrg;
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);

  newrg = malloc(sizeof(struct vm_rg_struct));
  newrg->rg_start = cur_vma->sbrk;
  newrg->rg_end = newrg->rg_start + size;
  cur_vma->sbrk = newrg->rg_end;
  return newrg;
}

int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, addr_t vmastart, addr_t vmaend)
{
  if (vmastart >= vmaend)
  {
    return -1;
  }

  struct vm_area_struct *vma = caller->krnl->mm->mmap;
  if (vma == NULL)
  {
    return -1;
  }

  struct vm_area_struct *cur_area = get_vma_by_num(caller->krnl->mm, vmaid);
  if (cur_area == NULL)
  {
    return -1;
  }

  while (vma != NULL)
  {
    /* Sử dụng OVERLAP macro để kiểm tra sự chồng chéo giữa 2 vùng nhớ */
    if (vma != cur_area && OVERLAP(vma->vm_start, vma->vm_end, vmastart, vmaend))
    {
      return -1;
    }
    vma = vma->vm_next;
  }

  return 0;
}

int inc_vma_limit(struct pcb_t *caller, int vmaid, addr_t inc_sz)
{
  struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));

  /* TOTO with new address scheme, the size need tobe aligned 
   *      the raw inc_sz maybe not fit pagesize
   */ 
  addr_t inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
  int incnumpage =  inc_amt / PAGING_PAGESZ;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->krnl->mm, vmaid);
  if (cur_vma == NULL) {
    free(newrg);
    return -1;
  }

  addr_t old_end = cur_vma->vm_end;
  addr_t new_end = old_end + inc_amt;

  if (validate_overlap_vm_area(caller, vmaid, cur_vma->vm_start, new_end) < 0) {
    free(newrg);
    return -1; /*Overlap and failed allocation */
  }

  cur_vma->vm_end = new_end;

  /* The obtained vm area (only)
   * now will be alloc real ram region */
  if (vm_map_ram(caller, cur_vma->vm_start, cur_vma->vm_end, 
                   old_end, incnumpage , newrg) < 0) {
    cur_vma->vm_end = old_end; // Phục hồi nếu map thất bại
    free(newrg);
    return -1; /* Map the memory to MEMRAM */
  }

  /* Cập nhật vùng không gian mới vào danh sách vùng trống (freerg_list) */
  enlist_vm_rg_node(&cur_vma->vm_freerg_list, newrg);

  return 0;
}

// #endif

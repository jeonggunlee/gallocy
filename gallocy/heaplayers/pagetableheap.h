#ifndef _PAGETABLEHEAP_H_
#define _PAGETABLEHEAP_H_

#include "pagetable.h"


namespace HL {

template <class Super>
class PageTableHeap : public Super {
public:

  PageTableHeap() {
    sum = 0;
  }

  inline void * malloc (size_t sz) {
    void *ptr;
    ptr = Super::malloc (sz);
    if (ptr) {
      pt.insert_page_table_entry(ptr, sz);
      sum += sz;
    }
    return ptr;
  }

  inline void free (void *ptr) {
    Super::free(ptr);
  }

  inline void __reset() {
    Super::__reset();
  }

private:
  long int sum;

};

}

#endif

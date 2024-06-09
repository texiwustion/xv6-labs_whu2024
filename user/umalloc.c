#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define NALLOC 4096
#ifdef NULL
#undef NULL
#endif
#define NULL 0

typedef long Align;

union header {
  struct {
    union header *next;
    unsigned size;
  } s;
  Align x;
};
typedef union header Header;

static Header base;
static Header *freep = NULL;

static Header *morecore(unsigned nblocks);
void free(void *ptr);

void *malloc(unsigned nbytes) {
  Header *currp;
  Header *prevp;
  unsigned nunits;

  nunits = ((nbytes + sizeof(Header) - 1) / sizeof(Header)) + 1;

  if (freep == NULL) {
    base.s.next = &base;
    base.s.size = 0;
    freep = &base;
  }

  prevp = freep;
  currp = prevp->s.next;

  for (; ; prevp = currp, currp = currp->s.next) {
    if (currp->s.size >= nunits) {
      if (currp->s.size == nunits) {
        prevp->s.next = currp->s.next;
      } else {
        currp->s.size -= nunits;
        currp += currp->s.size;
        currp->s.size = nunits;
      }
      freep = prevp;
      return (void *)(currp + 1);
    }
    if (currp == freep) {
      if ((currp = morecore(nunits)) == NULL) {
        return NULL;
      }
    }
  }
}

static Header *morecore(unsigned nunits) {
  void *freemem;
  Header *insertp;

  if (nunits < NALLOC) {
    nunits = NALLOC;
  }

  freemem = sbrk(nunits * sizeof(Header));
  if (freemem == (void *) -1) {
    return NULL;
  }

  insertp = (Header *) freemem;
  insertp->s.size = nunits;
  free((void *)(insertp + 1));
  return freep;
}

void free(void *ptr) {
  Header *insertp, *currp;
  insertp = ((Header *)ptr) - 1;

  currp = freep;
  //for (currp = freep; !((currp < insertp) && (insertp < currp->s.next)); currp = currp->s.next) {
  //  if ((currp >= currp->s.next) && ((currp < insertp) || (insertp < currp->s.next))) {
  //    break;
  //  }
  //}
  //
  if ((insertp + insertp->s.size) == currp->s.next) {
    insertp->s.size += currp->s.next->s.size;
    insertp->s.next = currp->s.next->s.next;
  } else {
    insertp->s.next = currp->s.next;
  }
  currp->s.next = insertp;

  //if ((currp + currp->s.size) == insertp) {
  //  currp->s.size += insertp->s.size;
  //  currp->s.next = insertp->s.next;
  //} else {
  //  currp->s.next = insertp;
  //}

  //freep = currp;
}

#define FRAGMENT_THRESHOLD 512

void get_memory_fragments() {
  unsigned fragment_count = 0;
  unsigned total_fragments_size = 0;

  if (freep == NULL) {
    printf("No free memory blocks available.\n");
    return;
  }

  Header *p = freep;

  do {
    if (p->s.size <= FRAGMENT_THRESHOLD / sizeof(Header)) {
      fragment_count++;
      total_fragments_size += p->s.size * sizeof(Header);
    }
    p = p->s.next;
  } while (p != freep);

  printf("Number of fragments <= %dB: %d\n", FRAGMENT_THRESHOLD, fragment_count - 1);
  printf("Total fragments size <= %dB: %dB\n", FRAGMENT_THRESHOLD, total_fragments_size);
}

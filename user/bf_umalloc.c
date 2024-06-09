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

static Header *morecore(unsigned nu);
void bf_free(void *ap);

void *bf_malloc(unsigned nbytes) {
  Header *p, *prevp;
  Header *best_fit = NULL; 
  Header *best_fit_prev = NULL; 
  unsigned nunits;

  nunits = ((nbytes + sizeof(Header) - 1) / sizeof(Header)) + 1;

  if (freep == NULL) {
    base.s.next = freep = &base;
    base.s.size = 0;
  }

  prevp = freep;
  for (p = prevp->s.next; ; prevp = p, p = p->s.next) {
    if (p->s.size >= nunits) {
      if (best_fit == NULL || p->s.size < best_fit->s.size) {
        best_fit = p;
        best_fit_prev = prevp;
      }
    }
    if (p == freep) {
      if (best_fit == NULL) {
        if ((p = morecore(nunits)) == NULL) {
          return NULL;
        }
      } else {
        p = best_fit;
        prevp = best_fit_prev;
        break;
      }
    }
  }

  if (p->s.size == nunits) {
    prevp->s.next = p->s.next;
  } else {
    p->s.size -= nunits;
    p += p->s.size;
    p->s.size = nunits;
  }

  return (void *)(p + 1);
}

static Header *morecore(unsigned nu) {
  char *cp;
  Header *up;

  if (nu < NALLOC)
    nu = NALLOC;
  cp = sbrk(nu * sizeof(Header));
  if (cp == (char *) -1)
    return NULL;
  up = (Header *) cp;
  up->s.size = nu;
  bf_free((void *)(up + 1));
  return freep;
}

void bf_free(void *ap) {
  Header *bp, *p;

  bp = (Header *)ap - 1;
  p = freep;
  if (bp + bp->s.size == p->s.next) {
    bp->s.size += p->s.next->s.size;
    bp->s.next = p->s.next->s.next;
  } else {
    bp->s.next = p->s.next;
  }
  p->s.next = bp;
}

#define FRAGMENT_THRESHOLD 512

void bf_get_memory_fragments() {
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

  printf("Number of fragments <= %dB: %d\n", FRAGMENT_THRESHOLD, fragment_count);
  printf("Total fragments size <= %dB: %dB\n", FRAGMENT_THRESHOLD, total_fragments_size);
}

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define NALLOC 4096  // ????????,?? sbrk ???

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
void free(void *ap);

void *malloc(unsigned nbytes) {
  Header *p, *prevp;
  Header *best_fit = NULL; // ??????????
  Header *best_fit_prev = NULL; // ???????????????
  unsigned nunits;

  nunits = ((nbytes + sizeof(Header) - 1) / sizeof(Header)) + 1;

  if (freep == NULL) {
    base.s.next = freep = &base;
    base.s.size = 0;
  }

  prevp = freep;
  for (p = prevp->s.next; ; prevp = p, p = p->s.next) {
    if (p->s.size >= nunits) {
      // ??????????,????????
      if (best_fit == NULL || p->s.size < best_fit->s.size) {
        best_fit = p;
        best_fit_prev = prevp;
      }
    }
    if (p == freep) {
      // ????????,??????????
      if (best_fit == NULL) {
        if ((p = morecore(nunits)) == NULL) {
          return NULL;
        }
      } else {
        // ????????,?????
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

  freep = prevp;
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
  free((void *)(up + 1));
  return freep;
}

void free(void *ap) {
  Header *bp, *p;

  bp = (Header *)ap - 1;

  for (p = freep; !(bp > p && bp < p->s.next); p = p->s.next)
    if (p >= p->s.next && (bp > p || bp < p->s.next))
      break;

  if (bp + bp->s.size == p->s.next) {
    bp->s.size += p->s.next->s.size;
    bp->s.next = p->s.next->s.next;
  } else {
    bp->s.next = p->s.next;
  }
  if (p + p->s.size == bp) {
    p->s.size += bp->s.size;
    p->s.next = bp->s.next;
  } else {
    p->s.next = bp;
  }
  freep = p;
}

// ??????, 65536 * 8B = 4=512KB
#define FRAGMENT_THRESHOLD 1024

void get_memory_fragments() {
  unsigned fragment_count = 0; // ??????
  unsigned total_fragments_size = 0; // ???????

  if (freep == NULL) {
    printf("No free memory blocks available.\n");
    return;
  }

  Header *p = freep;

  // ??????,????????????????
  do {
    if (p->s.size <= FRAGMENT_THRESHOLD / sizeof(Header)) {
      fragment_count++;
      total_fragments_size += p->s.size * sizeof(Header);
    }
    p = p->s.next;
  } while (p != freep);

  // ??????
  printf("Number of fragments <= %d B: %d\n", FRAGMENT_THRESHOLD, fragment_count);
  printf("Total fragments size <= %d B: %d B\n", FRAGMENT_THRESHOLD, total_fragments_size);
}



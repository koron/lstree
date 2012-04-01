/*
 * xmalloc : eXtended malloc
 *
 * by Kauzyoshi Michishita.
 *
 */
#ifndef LINT
static char _rcs_id[] = "$Id: xmalloc.c,v 2.1 1999/09/10 01:24:11 mit Exp $" ;
#endif

#include <stdio.h>
#include "xmalloc.h"

#define ROUND_MIN_SIZE   32
#define ROUND_MAX_SIZE 4096
#define ROUND_SHIFT       8 /* ROUND_MIN_SIZE to ROUND_MAX_SIZE */

struct header_t {
  int magic ;
#define MAGIC_NUMBER  0x88888888

  unsigned short alloc_size ;	/* real allocate size */
  unsigned short request_size ;
  struct header_t *link ;
} ;

typedef struct header_t header_t ;

#define H_MAGIC(ptr)      ((ptr)->magic)
#define H_ALLOC_SIZE(ptr) ((ptr)->alloc_size)
#define H_REQ_SIZE(ptr)   ((ptr)->request_size)
#define H_LINK(ptr)       ((ptr)->link)

#define max(a, b) ((a)>(b) ? (a) : (b))
#define min(a, b) ((a)<(b) ? (a) : (b))
#define in_range(p) ((unsigned char *)(p) >= range_block_low   \
		     && (unsigned char *)(p) <= range_block_hi)

static unsigned char *range_block_low = (unsigned char *)0xffffffff ;
static unsigned char *range_block_hi  = (unsigned char *)0x00000000 ;
static header_t *free_memory_block[8] = { NULL, NULL, NULL, NULL,
					  NULL, NULL, NULL, NULL } ;
static unsigned int cur_free_count[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 } ;
static unsigned int cur_alloc_count[8]   = { 0, 0, 0, 0, 0, 0, 0, 0 } ;
static unsigned int total_free_count[8]  = { 0, 0, 0, 0, 0, 0, 0, 0 } ;
static unsigned int total_alloc_count[8] = { 0, 0, 0, 0, 0, 0, 0, 0 }  ;
static unsigned int total_alloc_other = 0 ;

char *
xmalloc(size)
     int size ;
{
  char *rptr ;
  int xsize = size + sizeof(header_t) ;

  if (size == 0)
    {
      rptr = NULL ;
    }

  if (xsize > ROUND_MAX_SIZE)
    {
      total_alloc_other += size ;
      rptr = (char *)malloc(size) ;
    }
  else
    {
      header_t *ptr ;
      int i ;
      int rup_size = ROUND_MIN_SIZE ;

      for(i=0 ; i<ROUND_SHIFT ; i++)
	{
	  if (xsize <= rup_size)
	    {
	      break ;
	    }
	  rup_size <<= 1 ;
	}

      if (free_memory_block[i])
	{
	  /* has fit size in free list */
#if 0
	  printf("( %08x, %08x )\n", range_block_low, range_block_hi) ;
#endif
	  ptr = free_memory_block[i] ;
	  free_memory_block[i] = H_LINK(ptr) ;
	  cur_free_count[i]-- ;
	  cur_alloc_count[i]++ ;
	}
      else
	{
	  /* doesn't have fit size */
	  ptr = (header_t *)malloc(rup_size) ;
	  cur_alloc_count[i]++ ;

	  /* set block range */
	  range_block_low = min(range_block_low, (unsigned char *)ptr) ;
	  range_block_hi  = max(range_block_hi , (unsigned char *)(ptr + sizeof(header_t))) ;
#if 0
	  printf("( %08x, %08x )\n", range_block_low, range_block_hi) ;
#endif
	}
      total_alloc_count[i]++ ;

      H_MAGIC(ptr)      = MAGIC_NUMBER ;
      H_ALLOC_SIZE(ptr) = rup_size ;
      H_REQ_SIZE(ptr)   = size ;
      H_LINK(ptr)       = NULL ;

      rptr = (char *)ptr + sizeof(header_t) ;
    }

  if (rptr == NULL)
    {
      fprintf(stderr, "Abort Memory exhaust.\n") ;
      exit(2) ;
    }
  return rptr ;
}

void
xfree(p)
     char *p ;
{
  header_t *ptr = (header_t *)(p - sizeof(header_t)) ;

  if (in_range(p) && H_MAGIC(ptr) == MAGIC_NUMBER)
    {
      int idx ;

#if 0
      printf("( %08x, %08x )\n", range_block_low, range_block_hi) ;
#endif
      switch(H_ALLOC_SIZE(ptr))
	{
	case   32 : idx = 0 ; break ;
	case   64 : idx = 1 ; break ;
	case  128 : idx = 2 ; break ;
	case  256 : idx = 3 ; break ;
	case  512 : idx = 4 ; break ;
	case 1024 : idx = 5 ; break ;
	case 2048 : idx = 6 ; break ;
	case 4096 : idx = 7 ; break ;
	default :
	  fprintf(stderr, "xfree : memory allocation error.\n") ;
	  abort() ;
	}

      H_LINK(ptr) = free_memory_block[idx] ;
      free_memory_block[idx] = ptr ;

      cur_free_count[idx]++ ;
      cur_alloc_count[idx]-- ;
      total_free_count[idx]++ ;
      return ;
    }
  else
    {
      free(p) ;
    }
  return ;
}

char *
xcalloc(num, size)
     int num ;
     int size ;
{
  int sz = size * num ;
  char *ptr = xmalloc(sz) ;
  header_t *xptr = (header_t *)(ptr - sizeof(header_t)) ;

  if (H_MAGIC(xptr) == MAGIC_NUMBER)
    {
      int remain = H_ALLOC_SIZE(xptr) - sizeof(header_t) - sz ;

      memset(ptr, 0, sz) ;
      if (remain > 0)
	{
	  memset(ptr + sz, 0xff, remain) ;
	}
      return ptr ;
    }
  return (char *)calloc(num, size) ;
}

char *
xrealloc(p, size)
     char *p ;
     int size ;
{
  header_t *ptr = (header_t *)(p - sizeof(header_t)) ;

  if (in_range(p) && H_MAGIC(ptr) == MAGIC_NUMBER)
    {
      char *newptr ;

      if (size <= (H_ALLOC_SIZE(ptr) - sizeof(header_t)))
	{
	  H_REQ_SIZE(ptr) = size ;
	  return p ;
	}
      else
	{
	  newptr = xmalloc(size) ;
	  xfree(p) ;
	  return newptr ;
	}
    }
  return (char *)realloc(p, size) ;
}

void
xmalloc_information()
{
  int i ;
  int blk = ROUND_MIN_SIZE ;
  unsigned int req_total = 0 ;
  unsigned int cur_total = 0 ;

  printf("blocked memory allocation information\n") ;
  printf("          current      total\n") ;
  printf("block : alloc  free  alloc  free\n") ;
  for(i=0 ; i<ROUND_SHIFT ; i++)
    {
      req_total += total_alloc_count[i] * blk ;
      cur_total += cur_alloc_count[i] * blk ;

      printf("%5d : %5d %5d  %5d %5d\n",
	     blk,
	     cur_alloc_count[i],
	     cur_free_count[i],
	     total_alloc_count[i],
	     total_free_count[i]) ;
      blk <<= 1 ;
    }
  printf("request size : %d\n", req_total) ;
  printf("current using size : %d\n", cur_total) ;
}

#ifdef DEBUG
main()
{
#define COUNT 100
  int i ;
  char *p[COUNT] ;

#define S 1

  {
    char *ptr = xmalloc(16) ;
    ptr = xrealloc(ptr, 32) ;
    ptr = xrealloc(ptr, 33) ;
  }

#if 1
  for(i=S ; i<COUNT ; i++)
    {
      char *ptr ;

      printf("%d : \n", i) ;
      ptr = p[i] = xcalloc(1, i) ;
      ptr -= sizeof(header_t) ;
      printf("%08x real=%d req=%d\n",
	     ptr, H_ALLOC_SIZE(ptr), H_REQ_SIZE(ptr)) ;
    }
  for(i=S ; i<COUNT ; i++)
    xfree(p[i]) ;
  xmalloc_information() ;
#endif
}
#endif

/*
 * ends
 */

/*
 * xmalloc.h
 * by Kazuyoshi Michishita.
 */

#ifndef LINT
static char _rcs_id_xmalloc_h[] = "$Id: xmalloc.h,v 2.1 1999/09/10 01:24:11 mit Exp $" ;
#endif

extern char *xmalloc(/* int */) ;
extern char *xcalloc(/* int, int */) ;
extern void  xfree(/* char * */) ;
extern void  xmalloc_information(/* void */) ;

/*
 * ends here
 */

/*
 * lstat : in UNIX
 */

#ifdef _WIN32

#ifndef lint
static char _rcs_id[] = "@(#) $Id: sjis_pathcpy.c,v 2.1 1999/09/10 01:24:10 mit Exp $" ;
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lstree.h"

char *
sjis_pathcpy(dst, src)
     char *dst ;
     char *src ;
{
  int wdrv = 0 ;
  char *wpath = NULL ;
  int pathlen ;
  char *ch ;
  int lastchar ;

  if (strlen(src) >= 2 && src[1] == ':')
    {
      wdrv = src[0] ;
      wpath = src + 2 ;
    }
  else
    {
      wdrv = 0 ;
      wpath = src ;
    }

  pathlen = strlen(wpath) ;
  switch(pathlen)
    {
    case 0:
      wpath = "." ;
      pathlen = 1 ;
      break ;
    case 1:
      pathlen = 1 ;
      break ;
    default:
      lastchar = 0 ;
      ch = wpath ;
      while(*ch)
	{
	  lastchar = *ch++ ;
	  if (is_sjis1(lastchar) && (*ch == '\\'))
	    ch++ ;
	}
      if (lastchar == '/' || lastchar == '\\')
	{
	  pathlen-- ;
	}
      break ;
    }

  if (wdrv == 0)
    {
      dst[0] = '\0' ;
    }
  else
    {
      sprintf(dst, "%c:", wdrv) ;
    }
  strncat(dst, wpath, pathlen) ;

  return dst ;
}
#endif

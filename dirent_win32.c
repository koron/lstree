/*
 * dirent.c : UN*X like get dirctory entry information
 * for MicroSoft Visual C++(V5.0)
 *
 * by Kazuyoshi Michishita
 */

#ifdef _WIN32
#ifndef lint
static char _rcs_id[] = "$Id: dirent_win32.c,v 2.1 1999/09/10 01:24:07 mit Exp $" ;
#endif

#include <stdio.h>
#include <sys/stat.h>

#include <windows.h>
/* #include <mapiwin.h> */

#define byte1(ch) ((ch) & 0x0ff)
#define is_sjis1(ch) ((((byte1(ch) >= 0x081) && (byte1(ch) <= 0x09f)) \
                       || ((byte1(ch) >= 0x0e0) && (byte1(ch) <= 0x0fc))) ? 1 : 0)

#include "dirent_win32.h"

DIR *
opendir(const char *path)
{
  DIR *ptr = (DIR *)calloc(sizeof(DIR), 1) ;

  /* allocate directory entry */
  if (ptr == NULL)
    {
      return NULL ;
    }
  ptr->dd_status = DD_STATUS_NONE ;
  strcpy(ptr->dd_dirname, path) ;

  return ptr ;
}

void
closedir(DIR *ptr)
{
  FindClose(ptr->dd_handle) ;
  free(ptr) ;
}

struct dirent *
readdir(DIR *dirp)
{
  HANDLE h ;
  static struct dirent dent ;
  WIN32_FIND_DATA find_data ;
  char pathname[FILENAME_MAX] ;

  if (dirp->dd_status == DD_STATUS_ERROR
      || dirp->dd_status == DD_STATUS_EOF)
    return NULL ;

  /* get directory info */
  if (dirp->dd_status == DD_STATUS_NONE)
    {
      int lastchar ;
      char *s ;

      if (_fullpath(pathname, dirp->dd_dirname, FILENAME_MAX) == NULL)
	{
	  dirp->dd_status = DD_STATUS_EOF ;
	  return NULL ;
	}

      s = pathname ;
      while(*s)
	{
	  lastchar = *s++ ;
	  if (is_sjis1(lastchar) && (*s == '\\'))
	    s++ ;
	}

      if (lastchar != '\\' && lastchar != '/')
	strcat(pathname, "\\") ;
      strcat(pathname, "*") ;
 
      h = FindFirstFile(pathname, &find_data) ;
      if (h == (HANDLE)-1)
	{
	  dirp->dd_status = DD_STATUS_ERROR ;
	  return NULL ;
	}
      dirp->dd_handle = h ;
      dirp->dd_status++ ;
    }
  else
    {
      if (FindNextFile(dirp->dd_handle, &find_data) == 0)
	{
	  dirp->dd_status = DD_STATUS_ERROR ;
	  return NULL ;
	}
      dirp->dd_status++ ;
    }

  /* make directory entry info */
  strcpy(dent.d_name, find_data.cFileName) ;
  dent.d_namelen = strlen(find_data.cFileName) ;

  if (find_data.cAlternateFileName[0] == '\0')
    {
      /* LongFileName same as Short(DOS)FileName */
      strcpy(dent.d_dosname, find_data.cFileName) ;
    }
  else
    {
      /* Has short(DOS) filename */
      strcpy(dent.d_dosname, find_data.cAlternateFileName) ;
    }
  return &dent ;
}

#endif /* _WIN32 */
/*
 * ends here
 */

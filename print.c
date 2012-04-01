/*
 * Print node info
 */

/*
  Netscape Internal image
    <IMG ALIGN=absbottom BORDER=0 SRC="internal-gopher-menu">
    <IMG ALIGN=absbottom BORDER=0 SRC="internal-gopher-index">
    <IMG ALIGN=absbottom BORDER=0 SRC="internal-gopher-binary">
    <IMG ALIGN=absbottom BORDER=0 SRC="internal-gopher-image">
    <IMG ALIGN=absbottom BORDER=0 SRC="internal-gopher-sound">
    <IMG ALIGN=absbottom BORDER=0 SRC="internal-gopher-movie">
    <IMG ALIGN=absbottom BORDER=0 SRC="internal-gopher-unknown">
*/

#ifndef LINT
static char _rcs_id[] = "$Id: print.c,v 2.1 1999/09/10 01:24:10 mit Exp $" ;
#endif

#if !defined(_WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <termios.h>
#include <time.h>

#ifdef sun
extern char *sys_errlist[] ;
#endif

#endif

#if defined(_WIN32)
#include <sys/stat.h>
#include <sys/types.h>

#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <windows.h>
/* #include <mapiwin.h> */
#include "dirent_win32.h"
#include "getopt.h"
#endif

#include "xmalloc.h"
#include "lstree.h"

/* this */
extern int   print_nodeinfo(/* nodeinfo_t *node */) ;
extern void  print_node_count(/* void */) ;
extern void  print_node_type(/* nodeinfo_t *node */) ;
extern char *is_match_filename(/* char *name */) ;
extern void  print_tree(/* nodeinfo_t *node */) ;
extern int   print_dir_indent(/* nodeinfo_t *node */) ;
extern char *make_URL(/* nodeinfo_t *node */) ;
extern void  HTML_A(/* nodeinfo_t *href */) ;
/* other */
extern int   do_list_tree(/* nodeinfo_t *parent */) ;
extern int   wild_match(/* char *wild, char target */) ;
extern int   is_rootpath(/* char *path */) ;

int
print_nodeinfo(node)
     nodeinfo_t *node ;
{
  nodeinfo_t *ptr = node ;

  while(ptr)
    {
      print_node_type(ptr) ;
      ptr = NODE_LINK(ptr) ;
    }

  return 0 ;
}

static int node_dir = 0 ;
static int node_reg = 0 ;
static int node_lnk = 0 ;
static int node_cdev = 0 ;
static int node_bdev = 0 ;
static int node_fifo = 0 ;
static int node_sock = 0 ;

void
print_node_count()
{
  printf("directory     : %5d\n", node_dir) ;
  printf("regular file  : %5d\n", node_reg) ;
  printf("symbolic link : %5d\n", node_lnk) ;
#ifdef UNIX
#ifdef S_IFCHR
  printf("character dev : %5d\n", node_cdev) ;
#endif
#ifdef S_IFBLK
  printf("block devicek : %5d\n", node_bdev) ;
#endif
#ifdef S_IFIFO
  printf("fifo          : %5d\n", node_fifo) ;
#endif
#ifdef S_IFSOCK
  printf("socket        : %5d\n", node_sock) ;
#endif
#endif
}

void
print_node_type(node)
     nodeinfo_t *node ;
{
  if (IS_DIR(node))
    {
      node_dir++ ;
      print_tree(node) ;
      do_list_tree(node) ;
    }
  else if (IS_REG(node))
    {
      node_reg++ ;
      if (! opt_dironly)
	{
	  if (is_match_filename(NODE_NAME(node)))
	    {
	      print_tree(node) ;
	    }
	}
    }
#if defined(UNIX)
  else
    {
      if (IS_LNK(node))
	node_lnk++ ;
#ifdef S_IFCHR
      else if (IS_CHR(node))
	node_cdev++ ;
#endif
#ifdef S_IFBLK
      else if (IS_BLK(node))
	node_bdev++ ;
#endif
#ifdef S_IFIFO
      else if (IS_FIFO(node))
	node_fifo++ ;
#endif
#ifdef S_IFSOCK
      else if (IS_SOCK(node))
	node_sock++ ;
#endif
      if (! opt_dironly)
	{
	  if (is_match_filename(NODE_NAME(node)))
	    {
	      print_tree(node) ;
	    }
	}
    }
#elif defined(_WIN32)
  /* Future : support Short-Cut */
#endif /* UNIX */
}

char *
is_match_filename(name)
     char *name ;
{
  int i ;

  if (opt_count_fnmatch)
    {
      for(i=0 ; i<opt_count_fnmatch; i++)
	{
	  if (wild_match(opt_fnmatch[i], name))
	    {
	      return name ;
	    }
	}
      return NULL ;
    }
  return name ;
}

char *monthname[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"} ;

void
print_tree(node)
     nodeinfo_t *node ;
{
  int left ;
  struct tm *tp ;

  left = print_dir_indent(NODE_PARENT(node)) ;

  if (opt_html)
    {
      HTML_A(node) ;
    }

  if (NODE_PARENT(node))
    {
      if (opt_html)
	{
	  fputs("  ", stdout) ;
	  left -= 2 ;
	}
      else
	{
	  fputs("+-- ", stdout) ;
	  left -= 4 ;
	}
    }

  /* print filename */
#ifdef _WIN32
  switch(opt_withdosshort)
    {
    case 'S': /* only short */
      printf("%-12s ", node->dosname) ;
      left -= 12 ;
      break ;
    case 's': /* with short */
      if (strcmp(NODE_SHORT_NAME(node), NODE_NAME(node)) != 0)
        {
          printf("%-12s ", NODE_SHORT_NAME(node)) ;
          left -= (12 + 1) ;
        }
      fputs(NODE_NAME(node), stdout) ;
      left -= strlen(NODE_NAME(node)) ;
      break ;
    default :
      fputs(NODE_NAME(node), stdout) ;
      left -= strlen(NODE_NAME(node)) ;
      break ;
    }
  if (IS_DIR(node))
    {
      fputs(DIR_SEPARATER, stdout) ;
      left-- ;
    }
#else
  fputs(NODE_NAME(node), stdout) ;
  if (IS_DIR(node) && !is_rootpath(NODE_NAME(node)))
    {
      fputs(DIR_SEPARATER, stdout) ;
      left-- ;
    }

  left -= strlen(NODE_NAME(node)) ;
#endif

  if (NODE_SYMLINK(node))
    {
      fputs("@ -> ", stdout) ;
      fputs(NODE_SYMLINK(node), stdout) ;
      left -= (5 + strlen(NODE_SYMLINK(node))) ;
    }

  if (opt_html)
    {
      fputs("</A>", stdout) ;
    }

  if (opt_longprint)
    {
#define SPACES (11 + 13) /* filesize(10), time(12) */
      {
        int i ;
        for(i=left ; i>SPACES ; i--)
	  {
	    putchar(' ') ;
	  }
      }

#ifdef __FreeBSD__
      printf(" %10qd", NODE_SIZE(node)) ;
#else
      printf(" %10ld", NODE_SIZE(node)) ;
#endif

      tp = localtime(&NODE_MTIME(node)) ;
#define SIXMONTHS ((365 / 2) * 3600 * 24)
      if ((node->sb.st_mtime + SIXMONTHS) < time(NULL))
        {
          printf(" %s %2d %5d",
                 monthname[tp->tm_mon],
                 tp->tm_mday,
                 tp->tm_year + 1900) ;
        }
      else
        {
          printf(" %s %2d %2d:%02d",
                 monthname[tp->tm_mon],
                 tp->tm_mday,
                 tp->tm_hour,
                 tp->tm_min) ;
        }
      
    }

  putchar('\n') ;
}

int
print_dir_indent(node)
     nodeinfo_t *node ;
{
  int left ;

  if (node == NULL)
    return opt_ncolumns ;
  if (NODE_PARENT(node) == NULL)
    {
      fputs("  ", stdout) ;
      return opt_ncolumns - 2 ;
    }

  left = print_dir_indent(NODE_PARENT(node)) ;
  if (node->flag & NODE_LAST)
    {
      fputs("    ", stdout) ;
    }
  else
    {
      fputs("|   ", stdout) ;
    }
  return left - 4 ;
}

char *
make_URL(node)
     nodeinfo_t *node ;
{
  char *pathname ;

  if (node == NULL)
    return NULL ;

  pathname = make_URL(NODE_PARENT(node)) ;
  if (pathname == NULL)
    {
      pathname = xstrdup("") ;
    }
  else
    {
      char *npath = xmalloc(strlen(pathname) + strlen(NODE_NAME(node)) + 2) ;

      if (strlen(pathname) == 0)
	{
	  npath[0] = '\0' ;
	}
      else
	{
	  strcpy(npath, pathname) ;
	  strcat(npath, "/") ;
	}
      strcat(npath, NODE_NAME(node)) ;
      xfree(pathname) ;
      pathname = npath ;
#if 0
fputs("----", stdout) ;
fputs(pathname, stdout) ;
fputs("----\n", stdout) ;
#endif
    }
  return pathname ;
}

#define HTML_NS_DIR "internal-gopher-menu"
#define HTML_NS_GEN "internal-gopher-unknown"

void
HTML_A(node)
     nodeinfo_t *node ;
{
  char *pathname = make_URL(node) ;

  fputs("<A HREF=\"", stdout) ;
  fputs(pathname, stdout) ;
  fputs("\">", stdout) ;

  fputs("<IMG SRC=\"", stdout) ;
  fputs((IS_DIR(node) ? HTML_NS_DIR : HTML_NS_GEN), stdout) ;
  fputs("\" ", stdout) ;
  fputs("ALIGN=\"absbottom\" BORDER=0 ALT=\"", stdout) ;
  fputs(pathname, stdout) ;
  fputs("\">", stdout) ;

  xfree(pathname) ;
}

/*
 * ends here
 */

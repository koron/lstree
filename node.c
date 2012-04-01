/*
 * lstree : list tree directory
 * Created by Kazuyoshi Michishita
 */

#ifndef LINT
static char _rcs_id[] = "$Id: node.c,v 2.1 1999/09/10 01:24:09 mit Exp $" ;
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

/* */
extern char *make_pathname(/* nodeinfo_t *node */) ;
extern int   do_list_tree(/* nodeinfo_t *parent */) ;
extern int   get_depth(/* nodeinfo_t *node */) ;
static void  dump_stack(/* nodeinfo_t *node */) ;
extern nodeinfo_t *alloc_nodeinfo(/* nodeinfo_t *parent, char *node_name */) ;
extern void  free_nodeinfo(/* nodeinfo_t *node */) ;
extern void  sort_node(/* nodestack_t *nodestack, int (*func)() */) ;
extern int   sort_by_name(/* struct nodeinfo_t **a, struct nodeinfo_t **b */) ;
extern int   sort_by_mtime(/* struct nodeinfo_t **a, struct nodeinfo_t **b */) ;
extern int   sort_by_size(/* struct nodeinfo_t **a, struct nodeinfo_t **b */) ;


int
is_rootpath(pathname)
     char *pathname ;
{
#ifdef UNIX
  return strcmp(pathname, "/") == 0 ;
#elif _WIN32
  return strcmp(pathname+1, ":/") == 0 || strcmp(pathname+1, ":\\") == 0 ;
#endif
}

char *
make_pathname(node)
     nodeinfo_t *node ;
{
  char *pathname ;

  if (node == NULL)
    return NULL ;

  pathname = make_pathname(NODE_PARENT(node)) ;
  if (pathname == NULL)
    {
      pathname = xstrdup(NODE_NAME(node)) ;
    }
  else
    {
      char *npath = xmalloc(strlen(pathname) + strlen(NODE_NAME(node)) + 2) ;

      strcpy(npath, pathname) ;
      if (!is_rootpath(pathname))
	strcat(npath, DIR_SEPARATER) ;
      strcat(npath, NODE_NAME(node)) ;
      xfree(pathname) ;
      pathname = npath ;
    }
  return pathname ;
}

/*
 * directory tree walk
 */
int
do_list_tree(parent)
     nodeinfo_t *parent ;
{
  DIR *dirptr ;
  struct dirent *dp ;
  nodestack_t *nodestack ;
  char *pathname ;

  if (opt_maxdepth > 0 && get_depth(parent) > opt_maxdepth)
    {
      return 0 ;
    }

  pathname = make_pathname(parent) ;
  dirptr = opendir(pathname) ;
  if (dirptr != NULL)
    {
      /* make node list */
      nodestack = alloc_nodestack() ;
      while((dp = readdir(dirptr)) != NULL)
	{
	  nodeinfo_t *node ;

	  if (strcmp(".", dp->d_name) == 0
	      || strcmp("..", dp->d_name) == 0)
	    {
	      continue ;
	    }

	  node = alloc_nodeinfo(parent, dp->d_name) ;
	  if (node == NULL)
	    {
	      fprintf(stderr, "node error %s\n", dp->d_name) ;
	      continue ;
	    }
	  NODESTACK_COUNT(nodestack)++ ;
	  /* add link */
	  NODE_LINK(node) = NODESTACK_LINK(nodestack) ;
	  NODESTACK_LINK(nodestack) = node ;
	}
      closedir(dirptr) ;

      if (opt_verbose > 3)
	dump_stack(NODESTACK_LINK(nodestack)) ;

      /* sort node name */
      if (NODESTACK_COUNT(nodestack) > 0)
	{
	  sort_node(nodestack, sort_function) ;
	}
      /* print file/directory/... */
      print_nodeinfo(NODESTACK_LINK(nodestack)) ;
      /* free resource */
      free_nodeinfo(NODESTACK_LINK(nodestack)) ;
      free_nodestack(nodestack) ;
    }
  else
    {
      fprintf(stderr, "error opendir %s\n", pathname) ;
    }
  xfree(pathname) ;
  return 0 ;
}

int
get_depth(node)
     nodeinfo_t *node ;
{
  int depth = 0 ;

  while(node)
    {
      node = NODE_PARENT(node) ;
      depth++ ;
    }
  return depth ;
}

static void
dump_stack(node)
     nodeinfo_t *node ;
{
  printf("-----------------------------------------------\n") ;
  while(node)
    {
      printf("%s -> %s\n", NODE_NAME(node), NODE_NAME(NODE_PARENT(node))) ;
      fflush(stdout) ;
      node = NODE_LINK(node) ;
    }
}

/*
 * Manage node information
 */
nodeinfo_t *
alloc_nodeinfo(parent, node_name)
     nodeinfo_t *parent ;
     char *node_name ;
{
  nodeinfo_t *node = (nodeinfo_t *)xcalloc(1, sizeof(nodeinfo_t)) ;
  char *parent_path = (char *)make_pathname(parent) ;
  char pathname[1024] ;

  if (parent_path == NULL)
    sprintf(pathname, "%s", node_name) ;
  else
    {
      if (is_rootpath(parent_path))
	sprintf(pathname, "%s%s", parent_path, node_name) ;
      else
	sprintf(pathname, "%s%s%s", parent_path, DIR_SEPARATER, node_name) ;

      xfree(parent_path) ;
    }

  if (lstat(pathname, &NODE_STAT(node)) < 0)
    {
      free_nodeinfo(node) ;
      return NULL ;
    }

#if defined(UNIX)
  if (IS_LNK(node))
    {
      char symlink_buf[PATH_MAX] ;
      int len = readlink(pathname, symlink_buf, PATH_MAX) ;

      symlink_buf[len] = '\0' ;
      NODE_SYMLINK(node) = xstrdup(symlink_buf) ;
    }
#elif defined(_WIN32)
  /* Future : support Short-Cut */
#endif /* UNIX */

  NODE_PARENT(node) = parent ;
  NODE_NAME(node) = xstrdup(node_name) ;
  return node ;
}

void
free_nodeinfo(node)
     nodeinfo_t *node ;
{
  nodeinfo_t *ptr = node ;
  
  while(ptr)
    {
      nodeinfo_t *tmp = ptr ;

      ptr = NODE_LINK(ptr) ;
#ifdef _WIN32
      if (NODE_SHORT_NAME(tmp))
	xfree(NODE_SHORT_NAME(tmp)) ;
#endif
      if (NODE_NAME(tmp))
	xfree(NODE_NAME(tmp)) ;
      if (NODE_SYMLINK(tmp))
	xfree(NODE_SYMLINK(tmp)) ;
      xfree(tmp) ;
    }
}

/*
 * Sort Function
 *
 */
void
sort_node(nodestack, func)
     nodestack_t *nodestack ;
     int (*func)() ;
{
  int i ;
  nodeinfo_t *ptr ;
  nodeinfo_t **ptr_list ;

  ptr_list = (nodeinfo_t **)xmalloc(NODESTACK_COUNT(nodestack)
				    * sizeof(struct nodeinfo_t *)) ;
  ptr = NODESTACK_LINK(nodestack) ;
  for(i=0 ; i<NODESTACK_COUNT(nodestack) ; i++)
    {
      ptr_list[i] = ptr ;
      ptr = NODE_LINK(ptr) ;
    }
  qsort(ptr_list, NODESTACK_COUNT(nodestack), sizeof(struct nodeinfo_t *), func) ;
  for(i=0 ; i<NODESTACK_COUNT(nodestack)-1 ; i++)
    {
      NODE_LINK(ptr_list[i]) = ptr_list[i+1] ;
    }
  NODE_LINK(ptr_list[NODESTACK_COUNT(nodestack)-1]) = NULL ;
  NODE_FLAG_SET(ptr_list[NODESTACK_COUNT(nodestack)-1], NODE_LAST) ;
  NODESTACK_LINK(nodestack) = ptr_list[0] ;
  xfree(ptr_list) ;
}

/*
 * sort sub functions
 */
int
sort_by_name(a, b)
     struct nodeinfo_t **a ;
     struct nodeinfo_t **b ;
{
  if (opt_revsort)
    return strcmp(NODE_NAME(*b), NODE_NAME(*a)) ;
  else
    return strcmp(NODE_NAME(*a), NODE_NAME(*b)) ;
}

int
sort_by_mtime(a, b)
     struct nodeinfo_t **a ;
     struct nodeinfo_t **b ;
{
  if (opt_revsort)
    return NODE_MTIME(*a) - NODE_MTIME(*b) ;
  else
    return NODE_MTIME(*b) - NODE_MTIME(*a) ;
}

int
sort_by_size(a, b)
     struct nodeinfo_t **a ;
     struct nodeinfo_t **b ;
{
  if (opt_revsort)
    return NODE_SIZE(*a) - NODE_SIZE(*b) ;
  else
    return NODE_SIZE(*b) - NODE_SIZE(*a) ;
}

/*
 * ends
 */

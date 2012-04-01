/*
 * lstree : list tree directory
 * Created by Kazuyoshi Michishita
 */

/* #ifndef lint */
static char _rcs_id[] = "@(#) $Id: lstree.c,v 2.1 1999/09/10 01:24:09 mit Exp $" ;
/* #endif */

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
extern void usage() ;
extern void add_fnmatch() ;
extern void pattern_file() ;
extern void fnmatch_information() ;

/*
 * Global
 */
char *program_name ;

int opt_verbose = 0 ;
int opt_longprint = 0 ;
int opt_revsort = 0 ;
int opt_dironly = 0 ;
int opt_ncolumns = 79 ;
int opt_maxdepth = 0 ;
int opt_html = 0 ;
#ifdef _WIN32
int opt_withdosshort = 0 ;
#endif
int opt_count_fnmatch = 0 ;
char **opt_fnmatch ;

/* sort order is node-name */
int (*sort_function)() = sort_by_name ;

void
usage(progname)
     char *progname ;
{
  fprintf(stderr, "usage : %s [options] directory ...\n", progname) ;
  fprintf(stderr, "  options\n") ;
  fprintf(stderr, "   -d : print only directory\n") ;
  fprintf(stderr, "   -l : long print with sort by name\n") ;
  fprintf(stderr, "   -t : long print with sort by time\n") ;
  fprintf(stderr, "   -z : long print with sort by size\n") ;
  fprintf(stderr, "   -r : reverse sort\n") ;
  fprintf(stderr, "   -f pattern  : match pattern for filename\n") ;
  fprintf(stderr, "   -F filename : pattern file\n") ;
  fprintf(stderr, "   -w width : default 80\n") ;
  fprintf(stderr, "   -D depth : maximum depth(>=1)\n") ;
  fprintf(stderr, "   -H : output HTML\n") ;
#ifdef _WIN32
  fprintf(stderr, "   -S : only short filename(8+3)\n") ;
  fprintf(stderr, "   -s : with short filename(8+3)\n") ;
#endif
  fprintf(stderr, "   -v : this message\n") ;
  fprintf(stderr, "   -V level : Verbose(debug) mode(level 1-9)\n") ;
  fprintf(stderr, "  environment variables\n") ;
  fprintf(stderr, "   COLUMNS : number of columns for screen/printer\n") ;
  fprintf(stderr, "%s\n", _rcs_id) ;
  exit(2) ;
}

int
main(argc, argv)
     int argc ;
     char *argv[] ;
{
  int i ;
  int ch ;
  char *dummy ;
  char *dummy_argv[] = {".", NULL} ;

  program_name = argv[0] ;
#ifdef UNIX
  /* get terminal window size */
  if (isatty(fileno(stdout)))
    {
      char *ptr ;
      struct winsize winsz ;

      if (ioctl(fileno(stdout), TIOCGWINSZ, &winsz) >= 0
          && winsz.ws_col > 0)
        opt_ncolumns = winsz.ws_col ;
      else
        {
          ptr = (char *)getenv("COLUMNS") ;
          if (ptr)
            {
              opt_ncolumns = atoi(ptr) ;
            }
        }
    }
#else
  if (getenv("COLUMNS"))
    {
      opt_ncolumns = atoi(getenv("COLUMNS")) ;
    }
#endif /* UNIX */

  while((ch = getopt(argc, argv, OPT_STRING)) != EOF)
    {
      switch(ch)
        {
        case 'l':
          opt_longprint = 1 ;
          break ;
        case 'd':
          opt_dironly = 1 ;
          break ;
        case 'w':
          opt_ncolumns = strtol(optarg, &dummy, 0) ;
          break ;
        case 't':
          opt_longprint = 1 ;
	  sort_function = sort_by_mtime ;
          break ;
        case 'z':
          opt_longprint = 1 ;
	  sort_function = sort_by_size ;
          break ;
        case 'r':
          opt_revsort = 1 ;
          break ;
	case 'f':
	  add_fnmatch(optarg) ;
	  break ;
	case 'F':
	  pattern_file(optarg) ;
	  break ;
        case 'D':
          opt_maxdepth = atoi(optarg) ;
	  if (opt_maxdepth < 1)
	    {
	      usage(program_name) ;
	    }
          break ;
#ifdef _WIN32
        case 'S':
          opt_withdosshort = 'S' ;
          break ;
        case 's':
          opt_withdosshort = 's' ;
          break ;
#endif
	case 'H':
	  opt_html = 1 ;
	  break ;
	case 'V':
	  opt_verbose = atoi(optarg) ;
	  break ;
        case 'v': /* fall through to default */
        case '?': /* fall through to default */
        default:
          usage(program_name) ;
          break ;
        }
    }

  /* for verbose */
  if (opt_verbose > 0)
    {
      if (opt_html)
	printf("<!--\n") ;
      printf("lstree : Version %s\n", _rcs_id) ;
      printf("verbose level : %d\n", opt_verbose) ;
      printf("directory depth : %d\n", opt_maxdepth) ;
      printf("\n") ;
      for(i=0 ; i<argc ; i++)
	{
	  printf("arg[%d] : %s\n", i, argv[i]) ;
	}
      if (opt_html)
	printf("-->\n") ;
      printf("\n") ;
    }

  /* replace args */
  argc -= optind ;
  argv += optind ;

  if (argc == 0)
    {
      argc = 1 ;
      argv = dummy_argv ;
    }

  for(i=0 ; i<argc ; i++)
    {
      nodeinfo_t *node ;

#ifdef _WIN32
      {
	char *wptr ;

	wptr = xmalloc(strlen(argv[i]) + 4) ;
	sjis_pathcpy(wptr, argv[i]) ;
	argv[i] = wptr ;
      }
#endif /* WIN32 */

      node = alloc_nodeinfo(NULL, argv[i]) ;

      if (opt_html)
	{
	  if (argv != dummy_argv)
	    printf("<BASE HREF=\"%s%s\">\n",
		   argv[i], is_rootpath(argv[i]) ? "" : "/") ;
	  else
	    {
	      char *wd = getcwd(NULL, 0) ;

	      if (wd != NULL)
		{
		  printf("<BASE HREF=\"%s/%s/\">\n", wd, argv[i]) ;
		}
	      free(wd) ;
	    }
	  fputs("<PRE>\n", stdout) ;
	}

      print_nodeinfo(node) ;
      free_nodeinfo(node) ;
      node = NULL ; /* garbage for reuse */

      if (opt_html)
	{
	  fputs("</PRE>\n", stdout) ;
	}
    }

#ifdef _WIN32
  for(i=0 ; i<argc ; i++)
    xfree(argv[i]) ;
#endif /* WIN32 */

  switch(opt_verbose)
    {
    /* all cases fall through */
    case 9 :
    case 8 :
    case 7 :
    case 6 :
      printf("\n") ;
      opt_html && printf("<!--\n") ;
      xmalloc_information() ;
      opt_html && printf("-->\n") ;
    case 5 :
    case 4 :
    case 3 :
    case 2 :
      printf("\n") ;
      opt_html && printf("<!--\n") ;
      fnmatch_information() ;
      opt_html && printf("-->\n") ;
    case 1 :
      printf("\n") ;
      opt_html && printf("<PRE>\n") ;
      print_node_count() ;
      opt_html && printf("</PRE>\n") ;
    default : /* default or level 0 is no information */
      break ;
    }

  return 0 ;
}

void
add_fnmatch(pattern)
     char *pattern ;
{
  int i ;
  char **p = (char **)xmalloc(sizeof(char *) * (opt_count_fnmatch + 1)) ;

  if (strlen(pattern) > 0)
    {
      for(i=0 ; i<opt_count_fnmatch ; i++)
	p[i] = opt_fnmatch[i] ;
      if (opt_count_fnmatch > 0)
	xfree(opt_fnmatch) ;
      opt_fnmatch = p ;
      opt_fnmatch[opt_count_fnmatch++] = pattern ;
    }
}

void
fnmatch_information()
{
  int i ;

  for(i=0 ; i<opt_count_fnmatch ; i++)
    {
      printf("matching pattern : \"%s\"\n", opt_fnmatch[i]) ;
    }
}

void
pattern_file(filename)
     char *filename ;
{
  FILE *fp = fopen(filename, "r") ;
  int plen ;
  char lbuf[256] ;

  if (fp == NULL)
    {
      usage(program_name) ;
    }
  while(fgets(lbuf, 256, fp) != NULL)
    {
      plen = strlen(lbuf) ;
      if (plen > 0)
	{
	  if (lbuf[plen - 1] == '\n')
	    {
	      lbuf[plen - 1] = '\0' ;
	    }
	  plen = 0 ;
	}

      if (plen > 0)
	{
	  add_fnmatch(strdup(lbuf)) ;
	}
    }
  fclose(fp) ;
}
/*
 * ends
 */

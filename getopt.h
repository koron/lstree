/*
 * pd_getopt.h : UNIX like getopt() header
 *
 * This is Public Domain, No original source header file.
 * written by Kazuyoshi Michishita
 */

extern char *optarg ;
extern int optind ;

#ifdef __STDC__
extern int getopt(int argc,
                  char *argv[],
		  char *optstring) ;
#else
extern int getopt() ;
#endif

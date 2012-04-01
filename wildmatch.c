/*
 * wildmatch : wildcard pattern matching, like a UNIX
 * Copyleft(not Copyright) by Kazuyoshi Michishita.
 * $Id: wildmatch.c,v 2.1 1999/09/10 01:24:11 mit Exp $
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define true  1
#define false 0

#define nul '\0'

#ifndef LINT
static char _rcs_id[] = "$Id: wildmatch.c,v 2.1 1999/09/10 01:24:11 mit Exp $" ;
#endif

extern int wild_match_charclass(/* char *wildcard, char *target */) ;
static int wild_match_charclass1(/* char **wildcardp, int tc */) ;


#ifdef DEBUG
#define WILD_MATCH(w, t) debug_wild_match((w), (t))

/*
 * Lisp style debugging
 */
int level = 0 ;

int
debug_wild_match(wildcard, target)
     char *wildcard ;
     char *target ;
{
  int ret ;
  int i ;


  for(i=0 ; i<level ; i++)
    printf("| ") ;
  printf("+-- wild_match(\"%s\", \"%s\")\n", wildcard, target) ;

  level++ ;
  ret = wild_match(wildcard, target) ;
  level-- ;

  for(i=0 ; i<level ; i++)
    printf("| ") ;
  printf("+-- %s\n", ret ? "true" : "false") ;

  return ret ;
}
#endif

int
wild_match(wildcard, target)
     char *wildcard ;
     char *target ;
{
#ifdef DEBUG
#  define wild_match(w, t) debug_wild_match(w, t)
#endif

  int wc = *wildcard ;	/* Wildcard Char */
  int tc = *target ;	/* Target Char */

  if (wc == nul)
    if (tc == nul)
      return true ;
    else 
      return false ;
  else if (wc == '\\')
    {
      wc = *(++wildcard) ;
      assert(wc != nul) ;
      if (wc == tc)
	return wild_match(wildcard+1, target+1) ;
    }
  else if (wc == '?')
    {
      if (tc == nul)
	return false ;
      return wild_match(wildcard+1, target+1) ;
    }
  else if (wc == '*')
    {
      if (tc == nul)
	return wild_match(wildcard+1, target) ;
      return wild_match(wildcard, target+1) || wild_match(wildcard+1, target) ;
    }
  else if (wc == '[')
    {
      return wild_match_charclass(wildcard+1, target) ;
    }
  else if (wc == tc)
    return wild_match(wildcard+1, target+1) ;

  return false ;
}

int
wild_match_charclass(wildcard, target)
     char *wildcard ;
     char *target ;
{
  int status ;
  int rev = false ;
  int cc = *wildcard ;
  int tc = *target ;

  if (cc == '^')
    {
      rev = true ;
      wildcard++ ;
    }

  status = rev ^ wild_match_charclass1(&wildcard, tc) ;

  while(*wildcard != ']' && *wildcard != nul)
    {
      wildcard++ ;
    }

  return status && wild_match(wildcard+1, target+1) ;
}

static int
wild_match_charclass1(wildcardp, tc)
     char **wildcardp ;
     int tc ;
{
  int cc = *(*wildcardp)++ ;

  while(cc != ']' && cc != nul)
    {
      if (cc == tc)
	return true ;

      cc = *(*wildcardp)++ ;
      switch(cc)
	{
	default :
	  break ;
	case '\\' :
	  cc = *(*wildcardp)++ ;
	  break ;
	case '-' :
	  {
	    int next = *(*wildcardp)++ ;

	    switch(next)
	      {
	      case ']' :
	      case nul :
		return false ;
		break ;
	      default :
		if (cc <= tc && next >= tc)
		  return true ;
	      }
	    cc = *(*wildcardp)++ ;
	  }
	}
    }
  return false ;
}

#ifdef DEBUG
#define TEST(w, t, mustbe) do { \
  if (WILD_MATCH((w), (t)) == mustbe) \
    printf("OK\n") ; \
  else \
    printf("NG\n") ; \
  printf("\n") ; \
  } while(0) ;

main()
{
  TEST("", "0", false) ;

  TEST("", "0", false) ;
  TEST("0", "", false) ;
  TEST("0", "01", false) ;

  TEST("", "", true) ;
  TEST("0", "0", true) ;
  TEST("01", "01", true) ;

  TEST("?", "", false) ;
  TEST("?", "0", true) ;
  TEST("0?", "0", false) ;
  TEST("0?", "01", true) ;
  TEST("0????5", "012345", true) ;

  TEST("\\\\", "\\", true) ;
  TEST("\\*", "*", true) ;
  TEST("\\\\*", "*", false) ;

  TEST("*", "", true) ;
  TEST("*", "*", true) ;
  TEST("*", "0", true) ;
  TEST("*", "01234", true) ;

  TEST("0*", "", false) ;
  TEST("0*", "0", true) ;
  TEST("0*", "01234", true) ;
  TEST("1*", "01234", false) ;

  TEST("0*4", "01234", true) ;
  TEST("0*45", "01234", false) ;
  TEST("0*45", "012345", true) ;
  TEST("0*4*", "0123456789", true) ;

  TEST("0*1", "01", true) ;
  TEST("0**1", "01", true) ;
  TEST("0*1*2", "012", true) ;

  TEST("[0]", "0", true) ;
  TEST("[01234]", "0", true) ;
  TEST("[01234", "0", false) ;
  TEST("0[01234]2", "012", true) ;
  TEST("0[0-4]", "02", true) ;
  TEST("0[0-4]5", "025", true) ;
  TEST("[^0]", "1", true) ;
  TEST("[^0", "1", false) ;
  TEST("[^0]", "0", false) ;
  TEST("[^0-4]", "5", true) ;
  TEST("[0-3][4-7][7-9]", "158", true) ;
  TEST("[0-3][^4-7][7-9]", "138", false) ;

  TEST("*.c", "a.c", true) ;
  TEST("d*.c", "d.c", true) ;
  TEST("d*1.c", "d01.c", true) ;
  

#if 0 /* DO NOT REMOVE */
  TEST("-*-*-*-*-*-*-12-*-*-*-m-*-*-*",
       "-adobe-courier-bold-o-normal--12-120-75-75-m-70-iso8859-1",
       true) ;
  TEST("-*-*-*-*-*-*-12-*-*-*-m-*-*-*",
       "-adobe-courier-bold-o-normal--12-120-75-75-X-70-iso8859-1",
       true) ;
#endif
}
#endif

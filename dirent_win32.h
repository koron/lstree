/*
 * dirent.h : UN*X like directory entry
 * for MicroSoft Visual C++(V5.0)
 *
 * by Kazuyoshi Michishita
 */
#ifdef _WIN32

#ifndef lint
static char _dirent_h[] = "$Id: dirent_win32.h,v 2.1 1999/09/10 01:24:07 mit Exp $" ;
#endif

#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif
#ifndef MAX_PATHNAME
#  define MAX_PATHNAME (PATH_MAX + 1)
#endif

struct _dirdesc
{
  HANDLE dd_handle ;
  int    dd_status ; // status with count
#define DD_STATUS_NONE  ( 0)
#define DD_STATUS_EOF   (-1)
#define DD_STATUS_ERROR (-2)
  char   dd_dirname[MAX_PATHNAME] ;
} ;
typedef struct _dirdesc DIR ;

struct dirent
{
  int  d_namelen ;
  char d_dosname[14] ;
  char d_longfilename[MAX_PATHNAME] ;
#define d_name d_longfilename
} ;

extern DIR *opendir(const char *path) ;
extern void closedir(DIR *dirp) ;
extern struct dirent *readdir(DIR *dirp) ;

#endif /* _WIN32 */
/*
 * ends here
 */

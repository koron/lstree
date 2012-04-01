/*
 * lstree.h
 *
 */

#ifndef lint
static char _rcs_lstree_h[] = "$Id: lstree.h,v 2.1 1999/09/10 01:24:09 mit Exp $" ;
#endif

#ifdef UNIX
#define OPT_STRING "dltvw:V:zrHD:f:F:"
#define DIR_SEPARATER "/"
#endif

#ifdef WIN32
#define OPT_STRING "dltvw:sSV:zrHD:f:F:"
#define DIR_SEPARATER "\\"
#define getcwd _getcwd
#define lstat stat

#define KANJI_SJIS
#endif

#ifdef KANJI_SJIS
#define byte1(ch) ((ch) & 0x0ff)
#define is_sjis1(ch) ((((byte1(ch) >= 0x081) && (byte1(ch) <= 0x09f)) \
                       || ((byte1(ch) >= 0x0e0) && (byte1(ch) <= 0x0fc))) ? 1 : 0)
#endif

/*
 * lstree uses MAX_PATHNAME_LEN
 */
#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif
#ifndef MAX_PATHNAME
#  define MAX_PATHNAME_LEN (PATH_MAX + 1)
#endif

#define bzero(ptr, len) memset((ptr), 0, (len))

/* getopt() */
extern char *optarg ;
extern int optind ;

/*
 * Command Option flag, value.
 */
extern int opt_verbose ;
extern int opt_longprint ;
extern int opt_dironly ;
extern int opt_revsort ;
extern int opt_ncolumns ;
extern int opt_maxdepth ;
extern int opt_html ;
#ifdef _WIN32
extern int opt_withdosshort ;
#endif
extern int opt_count_fnmatch ;
extern char **opt_fnmatch ;

extern int (*sort_function)(/* nodeinfo_t *, nodeinfo_t * */) ;

struct nodeinfo_t
{
  int    flag ;
#define NODE_LAST (1 << 0)
#define NODE_INFO (1 << 1)
  char   *name ;
  char   *symlink ;
#ifdef _WIN32
  char   *dosname ;
  char   drive ;	/* 'A', 'B', ... or 'a', 'b', ... */
  char   cur_drive ;
#endif
  struct stat sb ;
  struct nodeinfo_t *parent ;
  struct nodeinfo_t *link ;
} ;

typedef struct nodeinfo_t nodeinfo_t ;

#define NODE_FLAG_SET(ptr, val) (((ptr)->flag) |= (val))
#define NODE_FLAG_CLR(ptr, val) (((ptr)->flag) &= ~(val))
#define NODE_NAME(ptr)    ((ptr)->name)
#define NODE_SYMLINK(ptr) ((ptr)->symlink)
#ifdef _WIN32
#define NODE_SHORT_NAME(ptr)  ((ptr)->dosname)
#define HAS_SHORT_NAME(ptr)   NODE_SHORT_NAME(ptr)
#endif
#define NODE_PARENT(ptr) ((ptr)->parent)
#define NODE_LINK(ptr)   ((ptr)->link)
#define NODE_STAT(ptr)   ((ptr)->sb)
#define NODE_TYPE(ptr)   ((ptr)->sb.st_mode)
#define NODE_SIZE(ptr)   ((ptr)->sb.st_size)
#define NODE_MTIME(ptr)  ((ptr)->sb.st_mtime)

#define IS_LAST_NODE(ptr) ((((ptr)->flag) & NODE_LAST) == NODE_LAST)
#define IS_DIR(ptr) ((NODE_TYPE(ptr) & S_IFMT) == S_IFDIR)
#define IS_REG(ptr) ((NODE_TYPE(ptr) & S_IFMT) == S_IFREG)
#define IS_LNK(ptr) ((NODE_TYPE(ptr) & S_IFMT) == S_IFLNK)
#ifdef S_IFIFO
#define IS_FIFO(ptr) ((NODE_TYPE(ptr) & S_IFMT) == S_IFMT)
#endif
#ifdef S_IFCHR
#define IS_CHR(ptr) ((NODE_TYPE(ptr) & S_IFMT) == S_IFCHR)
#endif
#ifdef S_IFBLK
#define IS_BLK(ptr) ((NODE_TYPE(ptr) & S_IFMT) == S_IFBLK)
#endif
#ifdef S_IFSOCK
#define IS_SOCK(ptr) ((NODE_TYPE(ptr) & S_IFMT) == S_IFSOCK)
#endif

extern nodeinfo_t *alloc_nodeinfo(/* nodeinfo_t *parent, char *node_name */) ;
extern void        free_nodeinfo(/* nodeinfo_t *ptr */) ;

struct nodestack_t
{
  int count ;
  nodeinfo_t *link ;
} ;
#define NODESTACK_COUNT(ptr) ((ptr)->count)
#define NODESTACK_LINK(ptr)  ((ptr)->link)

#define alloc_nodestack() (nodestack_t *)xcalloc(1, sizeof(nodestack_t))
#define free_nodestack(ptr) xfree(ptr)

typedef struct nodestack_t nodestack_t ;

extern int print_nodeinfo() ;
extern void sort_node() ;
extern int sort_by_name() ;
extern int sort_by_mtime() ;
extern int sort_by_size() ;
extern int get_depth() ;
extern void print_node_count() ;
extern char *sjis_pathcpy() ;

extern char *xstrdup() ;
/*
 * ends
 */

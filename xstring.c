/*
 * xstrdup() uses xmalloc().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmalloc.h"

char *
xstrdup(char *str)
{
    char *ptr = (char *)xmalloc(strlen(str) + 1);

    if (ptr == NULL)
    {
        /* NOTREACHED */
        abort();
    }
    strcpy(ptr, str);
    return ptr;
}

/*
 *
 */

#
# Makefile for lstree
#
# $Id: Makefile,v 2.1 1999/09/10 01:24:06 mit Exp $
#

RM	= rm
CFLAGS	= -g -DUNIX -Wall
CSRCS	= lstree.c	\
	    node.c	\
	    print.c     \
	    xmalloc.c	\
	    xstring.c	\
	    wildmatch.c	\
	    dirent_win32.c \
	    sjis_pathcpy.c \
	    getopt.c
COBJS	= ${CSRCS:.c=.o}
HSRCS	= lstree.h	\
	  xmalloc.h	\
	  getopt.h	\
	  dirent_win32.h

ARCHIVE_FILES	= Makefile ${CSRCS} ${HSRCS} README_JP.euc

lstree : ${COBJS}
	cc ${LDFLAGS} ${COBJS} -o $@ 

README_JP.sjis : FORCE
	nkf -s README_JP.euc | sed 's/\(.*\)/\1/' > README_JP.sjis
#
# clean
#
clean : FORCE
	${RM} -f ${COBJS}
realclean : clean
	${RM} -f lstree lstree.exe README_JP.sjis lstree.tar.Z lstree2.zip

FORCE :

tar : ${CSRCS} ${HSRCS}
	tar cf - ${ARCHIVE_FILES} | compress > lstree.tar.Z

shar :  ${CSRCS} ${HSRCS}
	shar ${ARCHIVE_FILES} > lstree.shar

zip : README_JP.sjis lstree.exe
	zip lstree2.zip lstree.exe README_JP.sjis

#
# ends here
#

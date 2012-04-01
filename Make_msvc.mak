!INCLUDE <Win32.Mak>

DEFINES = \
	/D_CRT_SECURE_NO_WARNINGS=1 \
	/D_BIND_TO_CURRENT_VCLIBS_VERSION=1

CCFLAGS = $(cdebug) $(cflags) $(cvarsdll) $(DEFINES)
LDFLAGS = $(ldebug) $(conlflags) $(conlibsdll)

SRC =	\
	dirent_win32.c getopt.c lstree.c node.c print.c sjis_pathcpy.c \
	wildmatch.c xmalloc.c xstring.c

OBJ = $(SRC:.c=.obj)

############################################################################

.c.obj ::
	$(CC) $(CCFLAGS) /c $<

lstree.exe : $(OBJ)
	$(link) /NOLOGO $(LDFLAGS) /OUT:$@ $(OBJ)
	IF EXIST $@.manifest \
	    mt -nologo -manifest $@.manifest -outputresource:$@;1

build : lstree.exe

rebuild : clean build

clean :
	DEL /F *.obj
	DEL /F *.pdb
	DEL /F lstree.exe.manifest
	DEL /F lstree.exe

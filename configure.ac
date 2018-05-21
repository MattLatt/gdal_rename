AC_INIT([gdal_rename], [0.0.8], [mathieu.lattes@yahoo.fr])

AC_CONFIG_SRCDIR([src/gdal_rename.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AC_HEADER_STDC
#AC_CHECK_HEADERS(assert.h fcntl.h unistd.h dbmalloc.h dlfcn.h stdint.h limits.h locale.h values.h float.h errno.h direct.h)

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

AX_LIB_GDAL([2.0.0])
GDAL_FOUND="no"
if test ! -z "$GDAL_CFLAGS" -a ! -z "$GDAL_LDFLAGS"; then
    GDAL_FOUND="yes"
fi 

#GDAL_SRC_DIR="/0/TOOLS/GDAL/gdal-2.3.0"
#GDALSRC_CPPFLAGS="-I${GDAL_SRC_DIR}/apps"
#AC_SUBST(GDALSRC_CPPFLAGS)

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])


AC_OUTPUT

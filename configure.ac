AC_PREREQ(2.61)
AC_INIT([glmemperf], [0.4], [sami.kyostila@nokia.com])
AM_INIT_AUTOMAKE([glmemperf], [0.4])
AC_CONFIG_SRCDIR([util.h])
AC_CONFIG_HEADER([config.h])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([EGL], [eglInitialize])
AC_CHECK_LIB([GLESv2], [glDrawElements])
AC_CHECK_LIB([rt], [clock_gettime])
PKG_CHECK_MODULES([X], [x11 >= 1.0 xext >= 1.0])
AC_SUBST(X_CFLAGS)
AC_SUBST(X_LIBS)
PKG_CHECK_MODULES([OSSO], 
		  [libosso >= 0.9],
                  AC_DEFINE(HAVE_LIBOSSO, [], [Whether libosso is present on the system]), 
		  [AC_MSG_NOTICE([libosso not present.])])
AC_SUBST(OSSO_CFLAGS)
AC_SUBST(OSSO_LIBS)

# Checks for header files.
AC_PATH_X
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_INT64_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([clock_gettime memset munmap strchr strstr])

AC_CONFIG_FILES([Makefile data/Makefile debian/Makefile])
AC_OUTPUT

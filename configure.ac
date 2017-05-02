AC_INIT([hash bench], [0.1.0])

AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC_C11
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CXX_C_O
AX_CXX_COMPILE_STDCXX_11

AC_PATH_PROG([LOG4C_CONFIG], [log4c-config])
AS_IF([test "x$LOG4C_CONFIG" != "x"], [
  log4c_CFLAGS=`$LOG4C_CONFIG --cflags`
  log4c_LIBS=`$LOG4C_CONFIG --libs`
  AC_SUBST([log4c_CFLAGS])
  AC_SUBST([log4c_LIBS])
  AC_DEFINE([HAS_LOG4C], [1], [Define to 1 if log4c is available.])
])

# ugly test to detect gcc and clang that pretends to be gcc
AS_IF([test "x$GCC" = "xyes"],
  [AS_IF([$CC --version | grep "clang"],
   [atomic_LIBS=""],
   [atomic_LIBS="-latomic"])],
  [atomic_LIBS=""])
AC_SUBST([atomic_LIBS])

AC_CONFIG_FILES([Makefile])

AC_CONFIG_SUBDIRS([opic])
AC_OUTPUT

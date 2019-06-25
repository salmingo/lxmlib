dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(lxmlib, 0.1)

AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CXX
AC_PROG_CC
AC_C_INLINE

AC_CHECK_HEADERS([fcntl.h netdb.h stdlib.h string.h sys/socket.h syslog.h unistd.h])
AC_CHECK_FUNCS([ftruncate gettimeofday memset mkdir pow select socket sqrt strerror strcasecmp strpbrk floor memmove])
AC_CHECK_HEADER_STDBOOL
AC_CHECK_TYPES([ptrdiff_t])
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT64_T

AC_CONFIG_FILES(Makefile src/Makefile)
AC_OUTPUT

AC_INIT([blazer], [0.3.8], [blazer@krugerheavyindustries.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_PROG_CC
AC_CONFIG_HEADERS([src/config.h])
AC_CHECK_LIB([crypto], [main])
AC_CHECK_LIB([ssl], [main])
AC_CHECK_LIB([restclient-cpp], [main])
AC_CHECK_LIB([jansson], [main])
AC_CHECK_LIB([pthread], [main])
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT

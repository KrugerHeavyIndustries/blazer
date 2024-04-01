AC_INIT([blazer], [0.4.7], [blazer@krugerheavyindustries.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_PROG_CC
AC_CONFIG_HEADERS([src/config.h])

#PKG_CHECK_MODULES([openssl], [openssl > 1.1.0])

ACCEPT_SSL_LIB="no"
AC_CHECK_LIB(ssl, OPENSSL_init_ssl, [ACCEPT_SSL_LIB="yes"])
AC_CHECK_LIB(ssl, SSL_library_init, [ACCEPT_SSL_LIB="yes"])
AS_IF([test "x$ACCEPT_SSL_LIB" = xno], [AC_MSG_ERROR([library 'ssl' is required for OpenSSL])])
AC_CHECK_LIB([crypto], [EVP_EncryptInit], [],
	     [AC_MSG_FAILURE([can't find openssl crypto lib])])
AC_CHECK_LIB([restclient-cpp], [main])
AC_CHECK_LIB([jansson], [main])
AC_CHECK_LIB([pthread], [main])
AC_CHECK_LIB([curl], [main])
AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT

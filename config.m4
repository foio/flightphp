dnl $Id$
dnl config.m4 for extension flight

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(flight, for flight support,
dnl Make sure that the comment is aligned:
dnl [  --with-flight             Include flight support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(flight, whether to enable flight support,
dnl Make sure that the comment is aligned:
[  --enable-flight           Enable flight support])

if test "$PHP_FLIGHT" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-flight -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/flight.h"  # you most likely want to change this
  dnl if test -r $PHP_FLIGHT/$SEARCH_FOR; then # path given as parameter
  dnl   FLIGHT_DIR=$PHP_FLIGHT
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for flight files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       FLIGHT_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$FLIGHT_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the flight distribution])
  dnl fi

  dnl # --with-flight -> add include path
  dnl PHP_ADD_INCLUDE($FLIGHT_DIR/include)

  dnl # --with-flight -> check for lib and symbol presence
  dnl LIBNAME=flight # you may want to change this
  dnl LIBSYMBOL=flight # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $FLIGHT_DIR/lib, FLIGHT_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_FLIGHTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong flight lib version or lib not found])
  dnl ],[
  dnl   -L$FLIGHT_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(FLIGHT_SHARED_LIBADD)

  PHP_NEW_EXTENSION(flight, 
          flight.c              \
          flight_app.c, 
          $ext_shared)
fi

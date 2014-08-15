/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author:                                                              |
   +----------------------------------------------------------------------+
   */

/* $Id$ */

#ifndef PHP_FLIGHT_H
#define PHP_FLIGHT_H

extern zend_module_entry flight_module_entry;
#define phpext_flight_ptr &flight_module_entry

#ifdef PHP_WIN32
#	define PHP_FLIGHT_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_FLIGHT_API __attribute__ ((visibility("default")))
#else
#	define PHP_FLIGHT_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif



#define PHP_FLIGHT_VERSION                     "1.0.0"
#define FLIGHT_STARTUP_FUNCTION(module)    ZEND_MINIT_FUNCTION(flight_##module)
#define FLIGHT_RINIT_FUNCTION(module)      ZEND_RINIT_FUNCTION(flight_##module)
#define FLIGHT_STARTUP(module)             ZEND_MODULE_STARTUP_N(flight_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define FLIGHT_SHUTDOWN_FUNCTION(module)   ZEND_MINIT_FUNCTION(flight_##module)
#define FLIGHT_SHUTDOWN(module)            ZEND_MODULE_SHUTDOWN_N(flight_##module)(INIT_FUNC_ARGS_PASSTHRU)


PHP_MINIT_FUNCTION(flight);
PHP_MSHUTDOWN_FUNCTION(flight);
PHP_RINIT_FUNCTION(flight);
PHP_RSHUTDOWN_FUNCTION(flight);
PHP_MINFO_FUNCTION(flight);


/* 
   Declare any global variables you may need between the BEGIN
   and END macros here:     

   ZEND_BEGIN_MODULE_GLOBALS(flight)
   long  global_value;
   char *global_string;
   ZEND_END_MODULE_GLOBALS(flight)
   */

/* In every utility function you add that needs to use variables 
   in php_flight_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as FLIGHT_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
   */

#ifdef ZTS
#define FLIGHT_G(v) TSRMG(flight_globals_id, zend_flight_globals *, v)
#else
#define FLIGHT_G(v) (flight_globals.v)
#endif

#endif	/* PHP_FLIGHT_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

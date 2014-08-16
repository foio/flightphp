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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "php.h"
#include "php_ini.h"
#include "main/SAPI.h"
#include "Zend/zend_alloc.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#include "php_flight.h"
#include "flight_app.h"
#include "flight_request.h"

//ZEND_DECLARE_MODULE_GLOBALS(flight)

/* True global resources - no need for thread safety here */
static int le_flight;

/* {{{ flight_functions[]
 *
 * Every user visible function must have an entry in flight_functions[].
 */
const zend_function_entry flight_functions[] = {
    PHP_FE_END	/* Must be the last line in flight_functions[] */
};
/* }}} */

/* {{{ flight_module_entry
*/
zend_module_entry flight_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "flight",
    flight_functions,
    PHP_MINIT(flight),
    PHP_MSHUTDOWN(flight),
    PHP_RINIT(flight),		/* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(flight),	/* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(flight),
#if ZEND_MODULE_API_NO >= 20010901
    "0.1", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FLIGHT
ZEND_GET_MODULE(flight)
#endif

    /* {{{ PHP_INI
    */
    /* Remove comments and fill if you need to have entries in php.ini
       PHP_INI_BEGIN()
       STD_PHP_INI_ENTRY("flight.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_flight_globals, flight_globals)
       STD_PHP_INI_ENTRY("flight.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_flight_globals, flight_globals)
       PHP_INI_END()
       */
    /* }}} */

    /* {{{ php_flight_init_globals
    */
    /* Uncomment this function if you have INI entries
       static void php_flight_init_globals(zend_flight_globals *flight_globals)
       {
       flight_globals->global_value = 0;
       flight_globals->global_string = NULL;
       }
       */
    /* }}} */

    /* {{{ PHP_MINIT_FUNCTION
    */
PHP_MINIT_FUNCTION(flight)
{
    /* If you have INI entries, uncomment these lines 
       REGISTER_INI_ENTRIES();
       */
    FLIGHT_STARTUP(app);
    FLIGHT_STARTUP(request);
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(flight)
{
    /* uncomment this line if you have INI entries
       UNREGISTER_INI_ENTRIES();
       */
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
*/
PHP_RINIT_FUNCTION(flight)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(flight)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(flight)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "flight support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
       DISPLAY_INI_ENTRIES();
       */
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */



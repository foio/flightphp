/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
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
#include "php_ini.h" /* for zend_alter_ini_entry */
#include "Zend/zend.h"
#include "ext/standard/php_smart_str.h"
#include "php_flight.h"
#include "flight_route_static.h"
#include "flight_loader.h"
#include "flight_dispatcher.h"

zend_class_entry *flight_dispatcher_ce;


PHP_METHOD(Flight_Dispatcher, __construct)
{
}


PHP_METHOD(Flight_Dispatcher, route)
{
}

PHP_METHOD(Flight_Dispatcher, dispatch)
{
}



static zend_function_entry flight_dispatcher_methods[] =
{
    ZEND_ME(Flight_Dispatcher, __construct,  NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        ZEND_ME(Flight_Dispatcher, route,  NULL,   ZEND_ACC_PUBLIC)
        ZEND_ME(Flight_Dispatcher, dispatch,  NULL,   ZEND_ACC_PUBLIC)
        {
            NULL, NULL, NULL
        }
};


FLIGHT_STARTUP_FUNCTION(dispatcher)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Flight_Dispatcher",flight_dispatcher_methods);
    flight_dispatcher_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(flight_dispatcher_ce, ZEND_STRL(FIIGHT_DISPATCHER_PROPERTY_NAME_ROUTER), ZEND_ACC_PUBLIC  TSRMLS_CC);
    zend_declare_property_null(flight_dispatcher_ce, ZEND_STRL(FIIGHT_DISPATCHER_PROPERTY_NAME_ROUTER), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC TSRMLS_CC);
    return SUCCESS;
}

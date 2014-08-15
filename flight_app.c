/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+ | Copyright (c) 1997-2013 The PHP Group                                |
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
#include "php_ini.h" /*  for zend_alter_ini_entry */
#include "Zend/zend_interfaces.h" /*  for zend_call_method_with_* */
#include "./flight_app.h"

zend_class_entry *flight_app_ce;

ZEND_METHOD(Flight_App,route)
{
    php_printf("flight app!\n");
}

static zend_function_entry flight_app_methods[] = {
    ZEND_ME(Flight_App,    route,  NULL,   ZEND_ACC_PUBLIC) 
    { NULL, NULL, NULL }
};



FLIGHT_STARTUP_FUNCTION(app)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce,"Flight_App",flight_app_methods);
    flight_app_ce = zend_register_internal_class(&ce TSRMLS_CC);
    return SUCCESS;
}


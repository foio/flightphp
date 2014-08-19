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
#include "Zend/zend_interfaces.h" /* for zend_call_method_with_* */

#include "php_flight.h"
#include "flight_app.h"
#include "flight_request.h"

zend_class_entry *flight_app_ce;


PHP_METHOD(Flight_App,__construct)
{
    flight_request_t       *request;
    flight_app_t *self = getThis();
    zval *route_function_map;
    MAKE_STD_ZVAL(route_function_map);
    array_init(route_function_map);

    add_assoc_string(route_function_map ,"404", "404_function",1); 
    add_assoc_string(route_function_map ,"5xx", "5xx_function",1); 
    zend_update_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), route_function_map TSRMLS_CC);
    zval_ptr_dtor(&route_function_map);

    request = flight_request_instance(NULL, NULL TSRMLS_CC);

    if (!request) {
        FLIGHT_UNINITIALIZED_OBJECT(getThis());
        //TODO throw exception
        zend_throw_exception(NULL, "create request failed", -1 TSRMLS_CC);
        RETURN_FALSE;
    }
    zend_update_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_REQUEST), request TSRMLS_CC);
    zval_ptr_dtor(&request);
}



PHP_METHOD(Flight_App,route)
{
    zval *route_function_map;
    char *url_reg;
    int url_reg_len;
    char *function_name;
    int function_name_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss",&url_reg, &url_reg_len, &function_name, &function_name_len) == FAILURE) {
        RETURN_NULL();
    }
    flight_app_t *self = getThis();
    route_function_map = zend_read_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), 1 TSRMLS_CC);   
    add_assoc_string(route_function_map, url_reg, function_name,1); 
    zend_update_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), route_function_map TSRMLS_CC);
}

static zend_function_entry flight_app_methods[] = {
    ZEND_ME(Flight_App, __construct,  NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        ZEND_ME(Flight_App,    route,  NULL,   ZEND_ACC_PUBLIC) 
        { NULL, NULL, NULL }
};



FLIGHT_STARTUP_FUNCTION(app)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce,"Flight_App",flight_app_methods);
    flight_app_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(flight_app_ce, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), ZEND_ACC_PUBLIC  TSRMLS_CC);
    zend_declare_property_null(flight_app_ce, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_REQUEST), ZEND_ACC_PUBLIC  TSRMLS_CC);
    return SUCCESS;
}


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


PHP_METHOD(Flight_App, __construct)
{
    flight_request_t       *request;
    flight_app_t *self = getThis();
    zval *route_function_map;
    MAKE_STD_ZVAL(route_function_map);
    array_init(route_function_map);

    add_assoc_string(route_function_map , "404", "404_function", 1);
    add_assoc_string(route_function_map , "5xx", "5xx_function", 1);
    zend_update_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), route_function_map TSRMLS_CC);
    zval_ptr_dtor(&route_function_map);

    request = flight_request_instance(NULL, NULL TSRMLS_CC);

    if (!request)
    {
        FLIGHT_UNINITIALIZED_OBJECT(getThis());
        //TODO throw exception
        zend_throw_exception(NULL, "create request failed", -1 TSRMLS_CC);
        RETURN_FALSE;
    }
    zend_update_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_REQUEST), request TSRMLS_CC);
    zval_ptr_dtor(&request);
}



PHP_METHOD(Flight_App, route)
{
    zval *route_function_map;
    char *url_reg;
    int url_reg_len;
    char *function_name;
    int function_name_len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &url_reg, &url_reg_len, &function_name, &function_name_len) == FAILURE)
    {
        RETURN_NULL();
    }
    flight_app_t *self = getThis();
    route_function_map = zend_read_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), 1 TSRMLS_CC);
    add_assoc_string(route_function_map, url_reg, function_name, 1);
    zend_update_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), route_function_map TSRMLS_CC);
}

PHP_METHOD(Flight_App, run)
{
    flight_request_t  *request;
    char *url_purge = NULL;
    char *f_name_purge = NULL;
    zval **function_name = NULL;
    zval *route_function_map;
    zval *url;
    zval *retval_ptr = NULL;
    zend_function    *fptr;
    flight_app_t *self = getThis();
    request = zend_read_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_REQUEST), 1 TSRMLS_CC);
    url = zend_read_property(flight_request_ce, request, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_URI), 1 TSRMLS_CC);
    route_function_map = zend_read_property(flight_app_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), 1 TSRMLS_CC);

    if (Z_TYPE_P(url) != IS_STRING || !Z_STRLEN_P(url))
    {
        zend_throw_exception(NULL, "url is not string", -1 TSRMLS_CC);
        RETURN_FALSE;
    }
    char *url_str = Z_STRVAL_P(url);
    unsigned long url_len = Z_STRLEN_P(url);
    //去除url结尾的斜杠
    if (url_len > 1 && url_str[url_len - 1] == '/')
    {
        url_len--;
    }
    url_purge = zend_str_tolower_dup(url_str, url_len);
    zend_hash_find(Z_ARRVAL_P(route_function_map), url_purge, url_len + 1, (void**)&function_name);

    if (function_name == NULL)
    {
        php_printf("page not found!\n");
    }
    else
    {
        if (Z_TYPE_PP(function_name) != IS_STRING || !Z_STRLEN_PP(function_name))
        {
            zend_throw_exception(NULL, "function_name is not string", -1 TSRMLS_CC);
            RETURN_FALSE;
        }

        //php_printf("Hello ");
        char *f_name = Z_STRVAL_PP(function_name);
        unsigned long f_len = Z_STRLEN_PP(function_name);
        f_name_purge = zend_str_tolower_dup(f_name, f_len);
        //PHPWRITE(f_name,f_len);
        //php_printf("Hello ");

        //检查函数是否存在
        if (zend_hash_find(EG(function_table), f_name_purge, f_len + 1, (void **)&fptr) == FAILURE)
        {
            zend_throw_exception(NULL, "function_name is undefined", -1 TSRMLS_CC);
            RETURN_FALSE;
        }

        MAKE_STD_ZVAL(retval_ptr);

        //根据函数参数个数调用函数
        if (fptr->common.num_args >= 2)
        {
            zend_throw_exception(NULL, "function parameters numbers should be 0 or 1", -1 TSRMLS_CC);
            RETURN_FALSE;
        }
        else if (fptr->common.num_args == 1)
        {
            zval **param = {request};
            call_user_function(CG(function_table), NULL, *function_name, retval_ptr, 1,  &param TSRMLS_CC);
        }
        else
        {
            call_user_function(CG(function_table), NULL, *function_name, retval_ptr, 0, NULL TSRMLS_CC);
        }

        if (Z_TYPE_P(retval_ptr) != IS_NULL)
        {
            zval_ptr_dtor(&retval_ptr);
        }
    }
    if (url_purge)
    {
        efree(url_purge);
    }
    if (f_name_purge){
        efree(f_name_purge);
    }
}

static zend_function_entry flight_app_methods[] =
{
    ZEND_ME(Flight_App, __construct,  NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        ZEND_ME(Flight_App,    route,  NULL,   ZEND_ACC_PUBLIC)
        ZEND_ME(Flight_App,    run,  NULL,   ZEND_ACC_PUBLIC)
        {
            NULL, NULL, NULL
        }
};



FLIGHT_STARTUP_FUNCTION(app)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Flight_App", flight_app_methods);
    flight_app_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(flight_app_ce, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP), ZEND_ACC_PUBLIC  TSRMLS_CC);
    zend_declare_property_null(flight_app_ce, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_REQUEST), ZEND_ACC_PUBLIC  TSRMLS_CC);
    return SUCCESS;
}

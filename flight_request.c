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
#include "main/SAPI.h"
#include "ext/standard/url.h" /* for php_url */
#include "php_ini.h" /* for zend_alter_ini_entry */
#include "Zend/zend_interfaces.h" /* for zend_call_method_with_* */

#include "php_flight.h"
#include "flight_request.h"

 


zend_class_entry *flight_request_ce;



/** {{{ zval * flight_request_query(uint type, char * name, uint len TSRMLS_DC)
*/
zval * flight_request_query(uint type, char * name, uint len TSRMLS_DC) {
    zval        **carrier = NULL, **ret;

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION < 4)
    zend_bool   jit_initialization = (PG(auto_globals_jit) && !PG(register_globals) && !PG(register_long_arrays));
#else
    zend_bool   jit_initialization = PG(auto_globals_jit);
#endif

    switch (type) {
        case FLIGHT_GLOBAL_VARS_POST:
        case FLIGHT_GLOBAL_VARS_GET:
        case FLIGHT_GLOBAL_VARS_FILES:
        case FLIGHT_GLOBAL_VARS_COOKIE:
            carrier = &PG(http_globals)[type];
            break;
        case FLIGHT_GLOBAL_VARS_ENV:
            if (jit_initialization) {
                zend_is_auto_global(ZEND_STRL("_ENV") TSRMLS_CC);
            }
            carrier = &PG(http_globals)[type];
            break;
        case FLIGHT_GLOBAL_VARS_SERVER:
            if (jit_initialization) {
                zend_is_auto_global(ZEND_STRL("_SERVER") TSRMLS_CC);
            }
            carrier = &PG(http_globals)[type];
            break;
        case FLIGHT_GLOBAL_VARS_REQUEST:
            if (jit_initialization) {
                zend_is_auto_global(ZEND_STRL("_REQUEST") TSRMLS_CC);
            }
            (void)zend_hash_find(&EG(symbol_table), ZEND_STRS("_REQUEST"), (void **)&carrier);
            break;
        default:
            break;
    }

    if (!carrier || !(*carrier)) {
        zval *empty;
        MAKE_STD_ZVAL(empty);
        ZVAL_NULL(empty);
        return empty;
    }

    if (!len) {
        Z_ADDREF_P(*carrier);
        return *carrier;
    }

    if (zend_hash_find(Z_ARRVAL_PP(carrier), name, len + 1, (void **)&ret) == FAILURE) {
        zval *empty;
        MAKE_STD_ZVAL(empty);
        ZVAL_NULL(empty);
        return empty;
    }

    Z_ADDREF_P(*ret);
    return *ret;
}
/* }}} */




/** {{{ flight_request_t * flight_request_instance(flight_request_t *this_ptr, char *request_uri, char *base_uri TSRMLS_DC)
*/
//创建flight_request对象，在flight_app的构造函数中调用
flight_request_t * flight_request_instance(flight_request_t *this_ptr, char *request_uri, char *base_uri TSRMLS_DC) {
    flight_request_t *instance;
    zval *method, *params, *settled_uri = NULL;

    if (this_ptr) {
        instance = this_ptr;//已经有对象
    } else {
        MAKE_STD_ZVAL(instance);//否则创建对象
        object_init_ex(instance, flight_request_ce);
    }
    MAKE_STD_ZVAL(method);
    if (SG(request_info).request_method) {//define SG(v) TSRMG(sapi_globals_id, sapi_globals_struct *, v),从sapi中获取请求方法
        ZVAL_STRING(method, (char *)SG(request_info).request_method, 1);
    } else if (strncasecmp(sapi_module.name, "cli", 3)) {//如果sapi实际调用的model不为cli则设置请求参数为Unknow
        ZVAL_STRING(method, "Unknow", 1);
    } else {
        ZVAL_STRING(method, "Cli", 1);//否则设置请求参数为Cli
    }     

    //设置该请求对象的method属性
    zend_update_property(flight_request_ce, instance, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_METHOD), method TSRMLS_CC);
    zval_ptr_dtor(&method);

    if (request_uri) {//如果参数中传递的request_uri不为空,则设置settled_uri   
        MAKE_STD_ZVAL(settled_uri);
        ZVAL_STRING(settled_uri, request_uri, 1);  
    }else{
        zval *uri;
        do{
            
//处理windows下的情况
#ifdef PHP_WIN32

            uri = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("HTTP_X_REWRITE_URL") TSRMLS_CC);
            if (Z_TYPE_P(uri) != IS_NULL) {
                settled_uri = uri;
                break;
            }
            zval_ptr_dtor(&uri);


            uri = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("IIS_WasUrlRewritten") TSRMLS_CC);
            if (Z_TYPE_P(uri) != IS_NULL) {
                zval *rewrited = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("IIS_WasUrlRewritten") TSRMLS_CC);
                zval *unencode = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("UNENCODED_URL") TSRMLS_CC);
                if (Z_TYPE_P(rewrited) == IS_LONG
                        && Z_LVAL_P(rewrited) == 1
                        && Z_TYPE_P(unencode) == IS_STRING
                        && Z_STRLEN_P(unencode) > 0) {
                    settled_uri = uri;
                }
                break;
            }
            zval_ptr_dtor(&uri); 
#endif

            //获取从全局变量$_SERVER['PATH_INFO']
            uri = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("PATH_INFO") TSRMLS_CC);
            if (Z_TYPE_P(uri) != IS_NULL) {
                settled_uri = uri;
                break;
            }
            zval_ptr_dtor(&uri); 

            //获取全局变量$_SERVER['REQUEST_URL']
            uri = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("REQUEST_URI") TSRMLS_CC); 
            if (Z_TYPE_P(uri) != IS_NULL) {
                if (strstr(Z_STRVAL_P(uri), "http") == Z_STRVAL_P(uri)) {//如果url以http开头
                    php_url *url_info = php_url_parse(Z_STRVAL_P(uri));                  

                    zval_ptr_dtor(&uri);

                    if (url_info && url_info->path) {//设置settled_uri为url_info->path
                        MAKE_STD_ZVAL(settled_uri);
                        ZVAL_STRING(settled_uri, url_info->path, 1);
                    }
                    php_url_free(url_info); 
                }else{   //如果url不是以http开头,则设置settled_uri的值为url问号前面的值
                    char *pos  = NULL;
                    if ((pos = strstr(Z_STRVAL_P(uri), "?"))) {
                        MAKE_STD_ZVAL(settled_uri);
                        ZVAL_STRINGL(settled_uri, Z_STRVAL_P(uri), pos - Z_STRVAL_P(uri), 1);
                        zval_ptr_dtor(&uri);
                    } else {
                        settled_uri = uri;
                    }  
                }
                break;
            } 
            zval_ptr_dtor(&uri);
            //从$_SERVER['ORIG_PATH_INFO']中取出变量
            uri = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("ORIG_PATH_INFO") TSRMLS_CC);
            if (Z_TYPE_P(uri) != IS_NULL) {
                                settled_uri = uri;
                break;
            }
            zval_ptr_dtor(&uri);  
        }while(0);
    }

    if (settled_uri) {
        char *p = Z_STRVAL_P(settled_uri);
        //在settled_uri中找到//
        while (*p == '/' && *(p + 1) == '/') {
            p++;
        }

        if (p != Z_STRVAL_P(settled_uri)) {
            char *garbage = Z_STRVAL_P(settled_uri);
            ZVAL_STRING(settled_uri, p, 1);
            efree(garbage);
        }

        //设置flight_request_http的uri值
        zend_update_property(flight_request_ce, instance, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_URI), settled_uri TSRMLS_CC);
        zval_ptr_dtor(&settled_uri);
    }

    MAKE_STD_ZVAL(params);
    array_init(params);
    zend_update_property(flight_request_ce, instance, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_PARAMS), params TSRMLS_CC);
    zval_ptr_dtor(&params);

    return instance;  
}





PHP_METHOD(Flight_Request, __construct) {
    char *request_uri = NULL;
    char *base_uri    = NULL;
    int  rlen         = 0;
    int  blen         = 0;

    flight_request_t *self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ss", &request_uri, &rlen, &base_uri, &blen) == FAILURE) {
        FLIGHT_UNINITIALIZED_OBJECT(getThis());
        return;
    }
    (void)flight_request_instance(self, request_uri, base_uri TSRMLS_CC);
}


static zend_function_entry flight_request_methods[] = {
    ZEND_ME(Flight_Request, __construct,  NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    { NULL, NULL, NULL }
};



FLIGHT_STARTUP_FUNCTION(request)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce,"Flight_Request",flight_request_methods);
    flight_request_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(flight_request_ce, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_METHOD), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(flight_request_ce, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_PARAMS), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(flight_request_ce, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_URI), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(flight_request_ce, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_BASE), ZEND_ACC_PUBLIC TSRMLS_CC);

    return SUCCESS;
}


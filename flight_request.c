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




int flight_request_set_base_uri(flight_request_t *request, char *base_uri, char *request_uri TSRMLS_DC){
    char *basename = NULL;
    uint basename_len = 0;
    zval *container = NULL; 
    if (!base_uri) {
        zval    *script_filename;
        char    *file_name, *ext = "php";
        size_t  file_name_len;
        uint    ext_len;

        ext_len = strlen(ext);  
        script_filename = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("SCRIPT_FILENAME") TSRMLS_CC);
        do {
            if (script_filename && IS_STRING == Z_TYPE_P(script_filename)) { 
                zval *script_name, *phpself_name, *orig_name;
                script_name = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("SCRIPT_NAME") TSRMLS_CC);
                php_basename(Z_STRVAL_P(script_filename), Z_STRLEN_P(script_filename), ext, ext_len, &file_name, &file_name_len TSRMLS_CC);
                if (script_name && IS_STRING == Z_TYPE_P(script_name)){
                    char    *script;
                    size_t  script_len;

                    php_basename(Z_STRVAL_P(script_name), Z_STRLEN_P(script_name),
                            NULL, 0, &script, &script_len TSRMLS_CC);

                    if (strncmp(file_name, script, file_name_len) == 0) {
                        basename    = Z_STRVAL_P(script_name);
                        basename_len    = Z_STRLEN_P(script_name);
                        container   = script_name;
                        efree(file_name);
                        efree(script);
                        break;
                    }
                    efree(script);  
                }
                zval_ptr_dtor(&script_name);
                phpself_name = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("PHP_SELF") TSRMLS_CC);
                if (phpself_name && IS_STRING == Z_TYPE_P(phpself_name)) {
                    char    *phpself;
                    size_t  phpself_len;

                    php_basename(Z_STRVAL_P(phpself_name), Z_STRLEN_P(phpself_name), NULL, 0, &phpself, &phpself_len TSRMLS_CC);
                    if (strncmp(file_name, phpself, file_name_len) == 0) {
                        basename     = Z_STRVAL_P(phpself_name);
                        basename_len = Z_STRLEN_P(phpself_name);
                        container = phpself_name;
                        efree(file_name);
                        efree(phpself);
                        break;
                    }
                    efree(phpself);
                }
                zval_ptr_dtor(&phpself_name);
                orig_name = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("ORIG_SCRIPT_NAME") TSRMLS_CC);
                if (orig_name && IS_STRING == Z_TYPE_P(orig_name)) {
                    char    *orig;
                    size_t  orig_len;
                    php_basename(Z_STRVAL_P(orig_name), Z_STRLEN_P(orig_name), NULL, 0, &orig, &orig_len TSRMLS_CC);
                    if (strncmp(file_name, orig, file_name_len) == 0) {
                        basename     = Z_STRVAL_P(orig_name);
                        basename_len = Z_STRLEN_P(orig_name);
                        container = orig_name;
                        efree(file_name);
                        efree(orig);
                        break;
                    }
                    efree(orig);
                }
                zval_ptr_dtor(&orig_name);
                efree(file_name);         
            }
        }while(0);
        zval_ptr_dtor(&script_filename);
        if (basename && strstr(request_uri, basename) == request_uri) {
            if (*(basename + basename_len - 1) == '/') {
                --basename_len;
            } 
            zend_update_property_stringl(flight_request_ce, request, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_BASE), basename,
                    basename_len TSRMLS_CC);
            if (container) {
                zval_ptr_dtor(&container);
            }
            return 1; 
        }else if(basename){
            size_t  dir_len;
            char    *dir = estrndup(basename, basename_len); /* php_dirname might alter the string */

            dir_len = php_dirname(dir, basename_len);
            if (*(basename + dir_len - 1) == '/') {
                --dir_len;
            }
            if (dir_len) {
                if (strstr(request_uri, dir) == request_uri) {
                    zend_update_property_string(flight_request_ce, request, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_BASE), dir  TSRMLS_CC);
                    efree(dir);
                    if (container) {
                        zval_ptr_dtor(&container);
                    }
                    return 1;
                }
            }
            efree(dir); 
        } 

        if (container) {
            zval_ptr_dtor(&container);
        }
        zend_update_property_string(flight_request_ce, request, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_BASE), "" TSRMLS_CC);
        return 1;  
    }else{
        zend_update_property_string(flight_request_ce, request, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_BASE), base_uri TSRMLS_CC);
        return 1; 
    } 
} 



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
        flight_request_set_base_uri(instance, base_uri, Z_STRVAL_P(settled_uri) TSRMLS_CC);
        zval_ptr_dtor(&settled_uri);
    }

    MAKE_STD_ZVAL(params);
    array_init(params);
    zend_update_property(flight_request_ce, instance, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_PARAMS), params TSRMLS_CC);
    zval_ptr_dtor(&params);

    return instance;  
}

/** {{{ proto public Flight_Request::getQuery(mixed $name, mixed $default = NULL)
*/
FLIGHT_REQUEST_METHOD(Flight_Request, Query,  FLIGHT_GLOBAL_VARS_GET);
/* }}} */

/** {{{ proto public FLIGHT_Request::getPost(mixed $name, mixed $default = NULL)
*/
FLIGHT_REQUEST_METHOD(Flight_Request, Post,      FLIGHT_GLOBAL_VARS_POST);
/* }}} */

/** {{{ proto public FLIGHT_Request::getRequet(mixed $name, mixed $default = NULL)
*/
FLIGHT_REQUEST_METHOD(Flight_Request, Request, FLIGHT_GLOBAL_VARS_REQUEST);
/* }}} */

/** {{{ proto public Flight_Request::getFiles(mixed $name, mixed $default = NULL)
*/
FLIGHT_REQUEST_METHOD(Flight_Request, Files,     FLIGHT_GLOBAL_VARS_FILES);
/* }}} */

/** {{{ proto public Flight_Request::getCookie(mixed $name, mixed $default = NULL)
*/
FLIGHT_REQUEST_METHOD(Flight_Request, Cookie,    FLIGHT_GLOBAL_VARS_COOKIE);
/* }}} */

/** {{{ proto public Yaf_Request_Http::isXmlHttpRequest()
*/
PHP_METHOD(Flight_Request, isXmlHttpRequest) {
    zval * header = flight_request_query(FLIGHT_GLOBAL_VARS_SERVER, ZEND_STRL("HTTP_X_REQUESTED_WITH") TSRMLS_CC);
    if (Z_TYPE_P(header) == IS_STRING
            && strncasecmp("XMLHttpRequest", Z_STRVAL_P(header), Z_STRLEN_P(header)) == 0) {
        zval_ptr_dtor(&header);
        RETURN_TRUE;
    }
    zval_ptr_dtor(&header);
    RETURN_FALSE;
}
/* }}} */


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

    ZEND_ME(Flight_Request, getQuery,      NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Flight_Request, getRequest,        NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Flight_Request, getPost,       NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Flight_Request, getCookie,     NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Flight_Request, getFiles,      NULL, ZEND_ACC_PUBLIC)
//    ZEND_ME(Flight_Request, get,           NULL, ZEND_ACC_PUBLIC)
    ZEND_ME(Flight_Request, isXmlHttpRequest,  NULL, ZEND_ACC_PUBLIC)

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


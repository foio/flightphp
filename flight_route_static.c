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
#include "flight_route_static.h"
#include "flight_request.h"

zend_class_entry *flight_route_static_ce;


PHP_METHOD(Flight_Route_Static, __construct)
{
    zval *url;
    flight_request_t       *request;
    flight_route_static_t *self = getThis();

    //初始化request对象
    request = flight_request_instance(NULL, NULL TSRMLS_CC);

    if (!request)
    {
        FLIGHT_UNINITIALIZED_OBJECT(getThis());
        zend_throw_exception(NULL, "create request failed in Flight_Route_Static", -1 TSRMLS_CC);
        RETURN_FALSE;
    }
    zend_update_property(flight_route_static_ce, self, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_REQUEST), request TSRMLS_CC);
    zval_ptr_dtor(&request);

}



PHP_METHOD(Flight_Route_Static, route)
{
    flight_route_static_t *self = getThis();
    //判断是否已经路由过
    zval* zval_router = zend_read_property(flight_route_static_ce, self, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_ROUTED), 1 TSRMLS_CC);
    if(Z_BVAL_P(zval_router)){
        RETURN_TRUE;
    }

    char *default_controller = "index",*default_action = "index";
    zval *url = NULL;
    char *url_str = NULL;
    char *controller = NULL,*action=NULL;
    char *p = NULL,*nextOccur=NULL;
    uint occur = 1;
    zval *settled_controller = NULL,*settled_action = NULL, *settled_routed = NULL;

    flight_request_t  *request;
    request = zend_read_property(flight_route_static_ce, self, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_REQUEST), 1 TSRMLS_CC);

    //根据request的url确定controller和action
    url = zend_read_property(flight_request_ce, request, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_URI), 1 TSRMLS_CC);   
    if (Z_TYPE_P(url) != IS_STRING || !Z_STRLEN_P(url))
    {
        zend_throw_exception(NULL, "url is not string", -1 TSRMLS_CC);
        RETURN_FALSE;
    }

    url_str = Z_STRVAL_P(url);
    p = url_str;
    
    do{
        if(*p=='/'){
            if(occur==1){
                if((nextOccur = strchr(p+1,'/')) == NULL ){
                    if(*(p+1)=='\0'){
                        //添加默认controller
                        controller = zend_str_tolower_dup(default_controller,strlen(default_controller));
                    }else{
                        controller = zend_str_tolower_dup(p+1,strlen(p+1));
                    }
                    break;
                }else{
                    controller = zend_str_tolower_dup(p+1,nextOccur-(p+1));
                }
            }else if(occur==2){
                if((nextOccur = strchr(p+1,'/')) == NULL){
                    if(*(p+1)=='\0'){
                        //添加默认action
                        action = zend_str_tolower_dup(default_action,strlen(default_action));
                    }else{
                        action = zend_str_tolower_dup(p+1,strlen(p+1));
                    }
                }else{
                    action = zend_str_tolower_dup(p+1,nextOccur-(p+1));
                }
                break;
            }
            occur++;     
        }
        p++;
    }while(*p!='\0');

    if(controller==NULL){
        controller = zend_str_tolower_dup(default_controller,strlen(default_controller));
    }

    if(action==NULL){
        action = zend_str_tolower_dup(default_action,strlen(default_action));
    }


    MAKE_STD_ZVAL(settled_controller);
    MAKE_STD_ZVAL(settled_action);
    MAKE_STD_ZVAL(settled_routed);

    ZVAL_STRING(settled_controller, controller, 0);
    ZVAL_STRING(settled_action, action, 0);
    ZVAL_BOOL(settled_routed, 1);



    //TODO 对controller和action属性赋值
    zend_update_property(flight_route_static_ce, self, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_CONTROLLER), settled_controller TSRMLS_CC);
    zend_update_property(flight_route_static_ce, self, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_ACTION), settled_action TSRMLS_CC);
    zend_update_property(flight_route_static_ce, self, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_ROUTED), settled_routed TSRMLS_CC);


    zval_ptr_dtor(&settled_controller);
    zval_ptr_dtor(&settled_action);
    zval_ptr_dtor(&settled_routed);
    RETURN_TRUE;
}


static zend_function_entry flight_route_static_methods[] =
{
    ZEND_ME(Flight_Route_Static, __construct,  NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        ZEND_ME(Flight_Route_Static, route,  NULL,   ZEND_ACC_PUBLIC)
        {
            NULL, NULL, NULL
        }
};



FLIGHT_STARTUP_FUNCTION(route_static)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Flight_Route_Static",flight_route_static_methods);
    flight_route_static_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(flight_route_static_ce, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_REQUEST), ZEND_ACC_PUBLIC  TSRMLS_CC);
    zend_declare_property_null(flight_route_static_ce, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_CONTROLLER), ZEND_ACC_PRIVATE  TSRMLS_CC);
    zend_declare_property_null(flight_route_static_ce, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_ACTION), ZEND_ACC_PRIVATE  TSRMLS_CC);
    zend_declare_property_bool(flight_route_static_ce, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_ROUTED), 0,ZEND_ACC_PRIVATE  TSRMLS_CC);
    return SUCCESS;
}

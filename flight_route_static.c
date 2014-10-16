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
    zend_update_property(flight_route_static_ce, self, ZEND_STRL(FIIGHT_APP_PROPERTY_NAME_REQUEST), request TSRMLS_CC);
    zval_ptr_dtor(&request);
    
    //根据request的url确定controller和action
    url = zend_read_property(flight_request_ce, request, ZEND_STRL(FLIGHT_REQUEST_PROPERTY_NAME_URI), 1 TSRMLS_CC);   
    if (Z_TYPE_P(url) != IS_STRING || !Z_STRLEN_P(url))
    {
            zend_throw_exception(NULL, "url is not string", -1 TSRMLS_CC);
            RETURN_FALSE;
    }

    char *url_str = Z_STRVAL_P(url);
    unsigned long url_len = Z_STRLEN_P(url);
    char *controller,*action;
    char *p = url_str;
    unsigned int occur = 0;
    while(*p!='\0'&&*p!='/'){
        if(*p=='/'){
            if(occur==1){
                if((char* nextOccur = strchr(p,'/')) == NULL ){
                    if(*(p+1)=='\0'){
                        //TODO  添加默认controller
                        zend_throw_exception(NULL, "can not parse controller from url", -1 TSRMLS_CC);;
                        RETURN_FALSE;
                    }else{
                        //url中只能解析到controller
                    }
                }
                //TODO 获取controller,可以正常解析controller
            }else if(occur==2){
                if((char* nextOccur = strchr(p,'/')) == NULL){
                    if(*(p+1)=='\0'){
                        //TODO  添加默认action
                        zend_throw_exception(NULL, "can not parse action from url", -1 TSRMLS_CC);;
                        RETURN_FALSE;
                    }else{
                        //TODO 获取action
                    }
                }else{
                        //TODO 获取action
                }
            }
            occur++;     
        }
        p++;
    }

    if(controller==NULL){
        //TODO 进行默认处理
    }

    if(action==NULL){
        //TODO 镜像默认处理
    }

    //TODO 对controller和action属性赋值

}



PHP_METHOD(Flight_Route_Static, route)
{
    
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
    zend_declare_property_null(flight_route_static_ce, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_REQUEST), ZEND_ACC_PRIVATE  TSRMLS_CC);
    zend_declare_property_null(flight_route_static_ce, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_CONTROLLER), ZEND_ACC_PRIVATE  TSRMLS_CC);
    zend_declare_property_null(flight_route_static_ce, ZEND_STRL(FIIGHT_ROUTE_STATIC_PROPERTY_NAME_ACTION), ZEND_ACC_PRIVATE  TSRMLS_CC);
    return SUCCESS;
}

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
#include "flight_loader.h"

zend_class_entry *flight_loader_ce;

int flight_loader_register(flight_loader_t *loader TSRMLS_DC) {
    zval *autoload, *method, *function, *ret = NULL;
    zval **params[1] = {&autoload};

    MAKE_STD_ZVAL(autoload);
    array_init(autoload);

    MAKE_STD_ZVAL(method);
    ZVAL_STRING(method, FLIGHT_AUTOLOAD_FUNC_NAME, 1);

    zend_hash_next_index_insert(Z_ARRVAL_P(autoload), &loader, sizeof(flight_loader_t *), NULL);
    zend_hash_next_index_insert(Z_ARRVAL_P(autoload), &method, sizeof(zval *), NULL);

    MAKE_STD_ZVAL(function);
    ZVAL_STRING(function, FLIGHT_SPL_AUTOLOAD_REGISTER_NAME, 0);

    do {
        zend_fcall_info fci = {
            sizeof(fci),
            EG(function_table),
            function,
            NULL,
            &ret,
            1,
            (zval ***)params,
            NULL,
            1
        };

        if (zend_call_function(&fci, NULL TSRMLS_CC) == FAILURE) {
            if (ret) {
                zval_ptr_dtor(&ret);
            }
            efree(function);
            zval_ptr_dtor(&autoload);
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to register autoload function %s", FLIGHT_AUTOLOAD_FUNC_NAME);
            return 0;
        }
        if (ret) {
            zval_ptr_dtor(&ret);
        }
        efree(function);
        zval_ptr_dtor(&autoload);
    } while (0);
    return 1;
}

int flight_loader_import(char *path, int len TSRMLS_DC) {
    zend_file_handle file_handle;
    zend_op_array   *op_array;
    char realpath[MAXPATHLEN];

    if (!VCWD_REALPATH(path, realpath)) {
        return 0;
    }

    file_handle.filename = path;
    file_handle.free_filename = 0;
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.opened_path = NULL;
    file_handle.handle.fp = NULL;

    op_array = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);

    if (op_array && file_handle.handle.stream.handle) {
        int dummy = 1;

        if (!file_handle.opened_path) {
            file_handle.opened_path = path;
        }

        zend_hash_add(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path)+1, (void *)&dummy,
                sizeof(int), NULL);
    }
    zend_destroy_file_handle(&file_handle TSRMLS_CC);

    if (op_array) {
        zval *result = NULL;

        FLIGHT_STORE_EG_ENVIRON();

        EG(return_value_ptr_ptr) = &result;
        EG(active_op_array)      = op_array;

#if ((PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)) || (PHP_MAJOR_VERSION > 5)
        if (!EG(active_symbol_table)) {
            zend_rebuild_symbol_table(TSRMLS_C);
        }
#endif
        zend_execute(op_array TSRMLS_CC);

        destroy_op_array(op_array TSRMLS_CC);
        efree(op_array);
        if (!EG(exception)) {
            if (EG(return_value_ptr_ptr) && *EG(return_value_ptr_ptr)) {
                zval_ptr_dtor(EG(return_value_ptr_ptr));
            }
        }
        FLIGHT_RESTORE_EG_ENVIRON();
        return 1;
    }

    return 0;
}


int flight_internal_autoload(char* class_name,uint class_name_len,char *app_path,uint app_path_len TSRMLS_DC){
    //根据全局参数拼装文件路径名  
    char *php_suffix = ".php",*curp=NULL,*nextp=NULL;
    char seperater = '_';
    int status = 0;
    smart_str buf = {0}; 
    smart_str_appendl(&buf,app_path, app_path_len);

    //根据class_name拆分出路径信息
    curp = class_name;
    while(*curp!='\0'){
        if((nextp = strchr(curp,seperater))==NULL){
            break;
        }
        char *dirname = zend_str_tolower_dup(curp, nextp-curp);  
        smart_str_appendc(&buf, '/');
        smart_str_appendl(&buf, dirname, strlen(dirname));      
        efree(dirname);
        curp = nextp+1;
    }
    smart_str_appendc(&buf, '/');
    smart_str_appendl(&buf, class_name, class_name_len);
    smart_str_appendl(&buf, php_suffix, strlen(php_suffix));
    smart_str_0(&buf);
    status = flight_loader_import(buf.c, buf.len TSRMLS_CC);
    smart_str_free(&buf);
    if (!status)
        return 0;
    return 1;
}

flight_loader_t * flight_loader_instance(flight_loader_t *this_ptr, char *app_path TSRMLS_DC) {
    flight_loader_t *instance;
    zval *settled_app_path;
    //读取静态instance常量
    instance = zend_read_static_property(flight_loader_ce, ZEND_STRL(FLIGHT_LOADER_PROPERTY_NAME_INSTANCE), 1 TSRMLS_CC);

    //如果flight_loader已经有静态实例
    if (IS_OBJECT == Z_TYPE_P(instance)) {
        return instance;
    }

    if(this_ptr){
        instance = this_ptr;
    }else{
        //this_ptr存储有instance则直接使用，创建新的对象
        MAKE_STD_ZVAL(instance);
        object_init_ex(instance, flight_loader_ce);
    }

    zend_update_static_property(flight_loader_ce, ZEND_STRL(FLIGHT_LOADER_PROPERTY_NAME_INSTANCE), instance TSRMLS_CC);

    MAKE_STD_ZVAL(settled_app_path);
    ZVAL_STRING(settled_app_path, app_path, 1);

    zend_update_property(flight_loader_ce, instance, ZEND_STRL(FIIGHT_LOADER_PROPERTY_NAME_APP_PATH), settled_app_path TSRMLS_CC);
    zval_ptr_dtor(&settled_app_path);

    if (!flight_loader_register(instance TSRMLS_CC)) {
        return NULL;
    }
    return instance;
}

PHP_METHOD(Flight_Loader, __construct)
{
    char *app_path;
    uint app_path_len;
    flight_loader_t *self = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &app_path, &app_path_len) == FAILURE) {
        zend_throw_exception(NULL, "need params", -1 TSRMLS_CC);
        return;
    }

    flight_loader_instance(self,app_path TSRMLS_CC);
}


PHP_METHOD(Flight_Loader, autoload)
{
    char *class_name,*app_path;
    uint class_name_len = 0,app_path_len=0;
    zval *z_app_path;
    flight_loader_t *self = getThis();
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &class_name, &class_name_len) == FAILURE) {
        return;
    }
    z_app_path = zend_read_property(flight_loader_ce, self, ZEND_STRL(FIIGHT_LOADER_PROPERTY_NAME_APP_PATH), 1 TSRMLS_CC); 
    app_path = Z_STRVAL_P(z_app_path);
    app_path_len = Z_STRLEN_P(z_app_path);
    flight_internal_autoload(class_name,class_name_len,app_path,app_path_len TSRMLS_CC);
}




static zend_function_entry flight_loader_methods[] =
{
    ZEND_ME(Flight_Loader, __construct,  NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
        ZEND_ME(Flight_Loader, autoload,  NULL,   ZEND_ACC_PUBLIC)
        {
            NULL, NULL, NULL
        }
};


FLIGHT_STARTUP_FUNCTION(loader)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Flight_Loader",flight_loader_methods);
    flight_loader_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_declare_property_null(flight_loader_ce, ZEND_STRL(FIIGHT_LOADER_PROPERTY_NAME_APP_PATH), ZEND_ACC_PUBLIC  TSRMLS_CC);
    zend_declare_property_null(flight_loader_ce, ZEND_STRL(FLIGHT_LOADER_PROPERTY_NAME_INSTANCE), ZEND_ACC_PROTECTED|ZEND_ACC_STATIC TSRMLS_CC);
    return SUCCESS;
}

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


int flight_internal_autoload(char* class_name TSRMLS_DC){
    //根据全局参数拼装文件路径名  
    char *library_path  = "/home/users/zhangpeng12";
    char *php_suffix = ".php";
    unsigned int library_path_len = strlen(library_path);
    int status = 0;
    smart_str buf = {0}; 
    smart_str_appendl(&buf, library_path, library_path_len);
    smart_str_appendc(&buf, '/');
    smart_str_appendl(&buf, class_name, strlen(class_name));
    smart_str_appendl(&buf, php_suffix, strlen(php_suffix));
    smart_str_0(&buf);
    status = flight_loader_import(buf.c, buf.len TSRMLS_CC);
    smart_str_free(&buf);
    if (!status)
        return 0;
    return 1;
}

PHP_METHOD(Flight_Loader, __construct)
{

}


PHP_METHOD(Flight_Loader, autoload)
{
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
    zend_declare_property_null(flight_loader_ce, ZEND_STRL(FIIGHT_LOADER_PROPERTY_NAME_APP_PATH), ZEND_ACC_PRIVATE  TSRMLS_CC);
    zend_declare_property_null(flight_loader_ce, ZEND_STRL(FIIGHT_LOADER_PROPERTY_NAME_LIB_PATH), ZEND_ACC_PRIVATE  TSRMLS_CC);
    return SUCCESS;
}

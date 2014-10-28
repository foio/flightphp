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

#ifndef FLIGHT_LOADER_H 
#define FLIGHT_LOADER_H 
#define FIIGHT_LOADER_PROPERTY_NAME_APP_PATH                   "app_path"
#define FLIGHT_SPL_AUTOLOAD_REGISTER_NAME      "spl_autoload_register"
#define FLIGHT_AUTOLOAD_FUNC_NAME              "autoload"
#define FLIGHT_LOADER_PROPERTY_NAME_INSTANCE   "instance"


#define FLIGHT_STORE_EG_ENVIRON() \
    { \
        zval ** __old_return_value_pp   = EG(return_value_ptr_ptr); \
        zend_op ** __old_opline_ptr     = EG(opline_ptr); \
        zend_op_array * __old_op_array  = EG(active_op_array);

#define FLIGHT_RESTORE_EG_ENVIRON() \
        EG(return_value_ptr_ptr) = __old_return_value_pp;\
        EG(opline_ptr)           = __old_opline_ptr; \
        EG(active_op_array)      = __old_op_array; \
    }

#else

extern zend_class_entry *flight_loader_ce;
int flight_internal_autoload(char* class_name,uint class_name_len,char *app_path,uint app_path_len);
flight_loader_t * flight_loader_instance(flight_loader_t *this_ptr, char *app_path);
FLIGHT_STARTUP_FUNCTION(loader);
#endif

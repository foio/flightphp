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

#ifndef FLIGHT_APP_H 
#define FLIGHT_APP_H 
#define FIIGHT_APP_PROPERTY_NAME_ROUTE_FUNCTION_MAP        "route_function_map"
#define FIIGHT_APP_PROPERTY_NAME_REQUEST                   "request"

#define FLIGHT_HTTP_METHOD_GET                             "GET"
#define FLIGHT_HTTP_METHOD_POST                            "POST"
#define FLIGHT_HTTP_METHOD_PUT                             "PUT"
#define FLIGHT_HTTP_METHOD_DELETE                          "DELETE"
#define FLIGHT_HTTP_METHOD_HEAD                            "HEAD"
#define FLIGHT_HTTP_METHOD_OPTOINS                         "OPTIONS"
#define FLIGHT_HTTP_METHOD_TRACE                           "TRACE"
#define FLIGHT_HTTP_METHOD_CONNECT                         "CONNECT"
#define FLIGHT_HTTP_METHOD_CLI                             "Cli"
#define FLIGHT_HTTP_METHOD(method)                         FLIGHT_HTTP_METHOD_##method
extern zend_class_entry *flight_app_ce;
FLIGHT_STARTUP_FUNCTION(app);
#endif

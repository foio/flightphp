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

#ifndef FLIGHT_ROUTE_STATIC_H 
#define FLIGHT_ROUTE_STATIC_H 
#define FIIGHT_ROUTE_STATIC_PROPERTY_NAME_REQUEST                   "request"
#define FIIGHT_ROUTE_STATIC_PROPERTY_NAME_CONTROLLER                "contorller"
#define FIIGHT_ROUTE_STATIC_PROPERTY_NAME_ACTION                    "action"
#define FIIGHT_ROUTE_STATIC_PROPERTY_NAME_ROUTED                    "routed"

extern zend_class_entry *flight_route_static_ce;
FLIGHT_STARTUP_FUNCTION(route_static);
#endif

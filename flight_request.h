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

#ifndef FLIGHT_REQUEST_H 
#define FLIGHT_REQUEST_H  
#define FLIGHT_REQUEST_PROPERTY_NAME_METHOD        "method"
#define FLIGHT_REQUEST_PROPERTY_NAME_PARAMS        "params"
#define FLIGHT_REQUEST_PROPERTY_NAME_URI       "uri"
#define FLIGHT_REQUEST_PROPERTY_NAME_BASE      "_base_uri"

#define FLIGHT_GLOBAL_VARS_TYPE                    unsigned int
#define FLIGHT_GLOBAL_VARS_POST                TRACK_VARS_POST
#define FLIGHT_GLOBAL_VARS_GET                     TRACK_VARS_GET
#define FLIGHT_GLOBAL_VARS_ENV                     TRACK_VARS_ENV
#define FLIGHT_GLOBAL_VARS_FILES                   TRACK_VARS_FILES
#define FLIGHT_GLOBAL_VARS_SERVER                  TRACK_VARS_SERVER
#define FLIGHT_GLOBAL_VARS_REQUEST                 TRACK_VARS_REQUEST
#define FLIGHT_GLOBAL_VARS_COOKIE                  TRACK_VARS_COOKIE

extern zend_class_entry *flight_request_ce;
FLIGHT_STARTUP_FUNCTION(request);
#endif
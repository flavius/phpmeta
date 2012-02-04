/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2011 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Flavius Aspra <flavius.as@gmail.com>                        |
   +----------------------------------------------------------------------+
*/

#ifndef PHP_META_H
#define PHP_META_H

#define PHP_META_EXTNAME "meta"
#define PHP_META_EXTVER "0.1"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

extern zend_module_entry meta_module_entry;
#define phpext_meta_ptr &meta_module_entry

/* internal API of the scanner and the parser which can be used by other extensions are marked as such */
#define META_API

/*TODO here or in the internal parser?*/
void *meta_alloc(size_t size);
void meta_free(void* ptr);
zval* obj_call_method_internal_ex(zval *obj, zend_class_entry *ce, zend_function *func, zend_class_entry* calling_scope,
        zend_bool native_null
        TSRMLS_DC, char* fmt, ...);

/**
 * Unlike zend_call_method(), this function allows you to call a method with C data types
 * It makes calling objects' methods more enjoyable from the C perspective
 */
//static zval*** get_params_ex(const char *fmt, size_t len TSRMLS_DC, va_list argp)

/*TODO my debug, conditional define in 0.0.1*/
#include <standard/php_var.h>

/*
#define META_ZDUMP(pzv) do { php_printf("-- (%d : '%s') %p: ",__LINE__, __PRETTY_FUNCTION__, pzv); \
    if(NULL != pzv) php_debug_zval_dump(&(pzv), 0 TSRMLS_CC); } while(0)
*/

#if 0
#define META_PRINT(fmt, args...) php_printf("%s (%d): ", __FILE__, __LINE__); php_printf(fmt, ## args); php_printf("\n")
#else
#define META_PRINT(fmt, args...)
#endif

//TODO use php_var_export_ex, see PHP_FUNCTION(var_export) in ext/standard/var.c#513
#define META_ZDUMP(pzv) META_PRINT("%p", (pzv))

#endif /*PHP_META_H*/

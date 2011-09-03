/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2011 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Flavius Aspra <flavius.as@gmail.com>                        |
   +----------------------------------------------------------------------+
*/

#ifndef SCANNER_H
#define SCANNER_H
#include <zend.h>
#include "php_meta.h"

//TODO eventually turn the scanner into a class
#define PHP_META_SCANNER_DESCRIPTOR_RES_NAME "Meta Scanner"
extern int meta_scanner_descriptor;

PHP_FUNCTION(meta_scanner_init);
PHP_FUNCTION(meta_scanner_get);
PHP_FUNCTION(meta_scanner_token_name);
void php_meta_scanner_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

#endif // SCANNER_H

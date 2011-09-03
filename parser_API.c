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

#include <zend.h>
#include "php_meta.h"
#include "parser_API.h"
#include "parser.h"

/*
 * TODO remove these two, as we have the more generalized obj_call_method_internal_ex()
META_API zval* meta_tree_ctor(INTERNAL_FUNCTION_PARAMETERS) {
    zval *tree=NULL;
    zval **params = get_params("l", 42);
    zval *test;
    MAKE_STD_ZVAL(test);
    ZVAL_STRING(test, "hello", 1);
    tree = obj_init_from_ce(php_meta_asttree_ce, return_value, &params, 1 TSRMLS_CC);
    zval_ptr_dtor(params);
    efree(params);
    zval_ptr_dtor(&test);
    //TODO dtor params
    return tree;
}

META_API void meta_tree_dtor(zval **tree) {
    zval_ptr_dtor(tree);
}
*/

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

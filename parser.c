#include <zend.h>
#include <php.h>
#include "parser.h"

#include "php_meta.h" // for meta_zdump, TODO remove if not used

int meta_parser_init_function(INIT_FUNC_ARGS) {
    zend_class_entry ce;

    //--  ASTNode
    INIT_CLASS_ENTRY(ce, PHP_META_ASTNODE_CE_NAME, php_meta_astnode_functions);
    META_CLASS(node) = zend_register_internal_class(&ce TSRMLS_CC);
    META_PROP_ZERO(node, "type");
    META_PROP_ZERO(node, "start_line");
    META_PROP_ZERO(node, "end_line");
    META_PROP_NULL(node, "data");
    META_PROP_NULL(node, "children");
    META_PROP_NULL(node, "root");
    META_PROP_NULL(node, "parent");
    META_PROP_ZERO(node, "index");

    //-- ASTTree
    INIT_CLASS_ENTRY(ce, PHP_META_ASTTREE_CE_NAME, php_meta_asttree_functions);
    META_CLASS(tree) = zend_register_internal_class_ex(&ce, META_CLASS(node), PHP_META_ASTNODE_CE_NAME TSRMLS_CC);
    //META_PROP_NULL(tree, "children");

    //TODO register terminals and nonterminals numeric constants and names, from meta_parser_defs.h (write script for it)

    return SUCCESS;
}

//---------------- the ASTTree class ---------------------

PHP_METHOD(ASTTree, __construct) {
    zval *children;
    zval *obj = getThis();

    MAKE_STD_ZVAL(children);
    array_init(children);
    META_UP_PROP(tree, obj, "children", children);
    Z_DELREF_P(children);
}

PHP_METHOD(ASTTree, __destruct) {
    zval *children=NULL;
    zval *obj = getThis();
    children = zend_read_property(META_CLASS(tree), obj, STRL_PAIR("children")-1, 0 TSRMLS_CC);
    zval_ptr_dtor(&children);
}

PHP_METHOD(ASTTree, appendChild) {
    zval* children;
    zval *child;
    zval* obj;

    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &child)) {
        WRONG_PARAM_COUNT;
    }
    obj = getThis();
    children = zend_read_property(META_CLASS(tree), obj, STRL_PAIR("children")-1, 0 TSRMLS_CC);
    add_next_index_zval(children, child);
}

PHP_METHOD(ASTTree, removeChild) {
    //zval *children;
    //TODO remove child by object or by tree-path, return removed node
}

PHP_METHOD(ASTTree, hasChildNodes) {
    zval *children;
    children = zend_read_property(META_CLASS(tree), getThis(), STRL_PAIR("children")-1, 0 TSRMLS_CC);
    RETURN_BOOL(zend_hash_num_elements(Z_ARRVAL_P(children)));
}

PHP_METHOD(ASTTree, __toString) {
    //TODO serialize the tree to valid PHP code, according to some flags of the tree
    //TODO introduce flags
    //in practice, we would iterate the tree and serialize each node, recursively
}

ZEND_BEGIN_ARG_INFO_EX(php_meta_onearg, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(php_meta_twoargs, 0, 0, 2)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(php_meta_fiveargs, 0, 0, 5)
ZEND_END_ARG_INFO()

//TODO arg info for all methods

static const function_entry php_meta_asttree_functions[] = {
    PHP_ME(ASTTree, __construct,        NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
    PHP_ME(ASTTree, __destruct,         NULL, ZEND_ACC_DTOR|ZEND_ACC_PUBLIC)
    PHP_ME(ASTTree, appendChild,        php_meta_onearg, ZEND_ACC_PUBLIC)
    PHP_ME(ASTTree, removeChild,        NULL, ZEND_ACC_PUBLIC)
    PHP_ME(ASTTree, hasChildNodes,      NULL, ZEND_ACC_PUBLIC)
    ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};

//---------------- the ASTNode class ---------------------
PHP_METHOD(ASTNode, __construct) {
    long major, start_line, end_line;
    zval *obj, *minor, *root;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lzOll", &major, &minor, &root, META_CLASS(tree), &start_line, &end_line)) {
        WRONG_PARAM_COUNT;
    }
    obj = getThis();
    if(!Z_ISREF_P(minor)) {
        Z_DELREF_P(minor);
    }
    META_UP_PROP_L(node, obj, "type", major);
    META_UP_PROP(node, obj, "data", minor);
    META_UP_PROP(node, obj, "root", root);
    META_UP_PROP_L(node, obj, "start_line", start_line);
    META_UP_PROP_L(node, obj, "end_line", end_line);

    /* TODO init these properly if required
    META_PROP_NULL(node, "children");
    META_PROP_NULL(node, "parent");
    META_PROP_ZERO(node, "index");
    */

    //add myself to the root tree
    //TODO instead of searching for the function every time, find it once in MINIT and reuse it every time
    zend_function *appendChild;
    zend_hash_find(&META_CLASS(tree)->function_table, STRL_PAIR("appendchild"), (void**) &appendChild);
    obj_call_method_internal_ex(root, META_CLASS(tree), appendChild, META_CLASS(node), 0, 1 TSRMLS_CC, "z", obj);
}

PHP_METHOD(ASTNode, setParentNode) {
    zval *parent, *obj, *old_parent, *index;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &parent, META_CLASS(node))) {
        WRONG_PARAM_COUNT;
    }
    obj = getThis();
    old_parent = zend_read_property(META_CLASS(node), obj, STRL_PAIR("parent")-1, 0 TSRMLS_CC);
    if(IS_NULL != Z_TYPE_P(old_parent)) {
        //TODO if old_parent is not null, detach from it
    }

    META_UP_PROP(node, obj, "parent", parent);
    zend_function *appendChild;
    zend_hash_find(&META_CLASS(node)->function_table, STRL_PAIR("appendchild"), (void**) &appendChild);
    index = obj_call_method_internal_ex(parent, META_CLASS(node), appendChild, META_CLASS(node), 0, 1 TSRMLS_CC, "z", obj);
    META_UP_PROP(node, obj, "index", index);
}

PHP_METHOD(ASTNode, appendChild) {
    zval *child, *children, *obj;
    ulong index;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &child, META_CLASS(node))) {
        WRONG_PARAM_COUNT;
    }
    obj = getThis();
    children = zend_read_property(META_CLASS(node), obj, STRL_PAIR("children")-1, 0 TSRMLS_CC);
    add_next_index_zval(children, child);
    index = zend_hash_next_free_element(Z_ARRVAL_P(children))-1;
    //TODO integer overflow
    //META_UP_PROP_L(node, child, "index", index);
    RETURN_LONG(index);
}

//TODO arg info for all methods

static const function_entry php_meta_astnode_functions[] = {
    PHP_ME(ASTNode, __construct,        php_meta_fiveargs, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
    PHP_ME(ASTNode, setParentNode,      php_meta_onearg, ZEND_ACC_PUBLIC)
    PHP_ME(ASTNode, appendChild,        php_meta_onearg, ZEND_ACC_PUBLIC)
    ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};


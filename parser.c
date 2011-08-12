#include <zend.h>
#include <php.h>
#include "parser.h"

#include "php_meta.h" // for meta_zdump, TODO remove if not used

//TODO remove this
#if 1
#define DBG(fmt, args...) php_printf("\t\t"); php_printf(fmt, ## args); php_printf("\n")
#else
#define DBG(fmt, args...)
#endif


/* {{{ create and initialize internal classes
 */
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
    META_PROP_NULL(tree, "children");

    //TODO register terminals and nonterminals numeric constants and names, from meta_parser_defs.h (write script for it)

    return SUCCESS;
}
/* }}} */
/* {{{ The ASTTree class
 */
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

    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
                &child)) {
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

/* }}} */
//TODO arg info for all methods
/* {{{ arg info
 */
ZEND_BEGIN_ARG_INFO_EX(php_meta_onearg, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(php_meta_twoargs, 0, 0, 2)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(php_meta_fourargs, 0, 0, 4)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(php_meta_fiveargs, 0, 0, 5)
ZEND_END_ARG_INFO()
/* }}} */
/* {{{ ASTTree methods
 */
static const function_entry php_meta_asttree_functions[] = {
    PHP_ME(ASTTree, __construct,        NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
    PHP_ME(ASTTree, __destruct,         NULL, ZEND_ACC_DTOR|ZEND_ACC_PUBLIC)
    //PHP_ME(ASTTree, appendChild,        php_meta_onearg, ZEND_ACC_PUBLIC)
    //PHP_ME(ASTTree, removeChild,        NULL, ZEND_ACC_PUBLIC)
    //PHP_ME(ASTTree, hasChildNodes,      NULL, ZEND_ACC_PUBLIC)
    ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* {{{ The ASTNode class
 */
/* {{{ proto public void ASTNode::__construct(int major, ASTTree root, int start_line, int end_line [, mixed minor])
   Create new node */
PHP_METHOD(ASTNode, __construct) {
    long major, start_line, end_line;
    zval *obj, *minor=NULL, *root, *children;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lOll|z",
                &major, &root, META_CLASS(tree), &start_line, &end_line, &minor)) {
        WRONG_PARAM_COUNT;
    }
    obj = getThis();
    if(NULL == minor) {
        ALLOC_INIT_ZVAL(minor);
    }
    else if(!Z_ISREF_P(minor)) {//TODO ??? huh? why do we do this?
        Z_DELREF_P(minor);
    }
    META_UP_PROP_L(node, obj, "type", major);
    META_UP_PROP(node, obj, "data", minor);
    META_UP_PROP(node, obj, "root", root);
    META_UP_PROP_L(node, obj, "start_line", start_line);
    META_UP_PROP_L(node, obj, "end_line", end_line);
    MAKE_STD_ZVAL(children);
    array_init(children);
    META_UP_PROP(node, obj, "children", children);
    Z_DELREF_P(children);

    //add myself to the root tree
    //TODO instead of searching for the function every time, find it once in MINIT and reuse it every time
    //zend_function *appendChild;
    //zend_hash_find(&META_CLASS(tree)->function_table, STRL_PAIR("appendchild"), (void**) &appendChild);
    //obj_call_method_internal_ex(root, META_CLASS(tree), appendChild, META_CLASS(node), 0, 1 TSRMLS_CC, "z", obj);
}
/* }}} */
/* {{{ pending for deletion
 */
PHP_METHOD(ASTNode, setParentNode) {
    php_printf("you never setParentNode, appendChild instead!");
    /*
    zval *parent, *obj, *old_parent, *index;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
                &parent, META_CLASS(node))) {
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
    index = obj_call_method_internal_ex(parent, META_CLASS(node), appendChild, META_CLASS(node), 0 TSRMLS_CC, "z", obj);
    META_UP_PROP_L(node, obj, "index", Z_LVAL_P(index));
    zval_ptr_dtor(&index);
    */
}
/* }}} */
/* {{{ proto public int ASTNode::appendChild(mixed child)
   Appends child to this node, returning its index */
PHP_METHOD(ASTNode, appendChild) {
    zval *child, *property, *obj;
    ulong index;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &child)) {
        WRONG_PARAM_COUNT;
    }
    //TODO child can be either string, or ASTNode (?)
    obj = getThis();
    //-- add child to our children
    property = zend_read_property(META_CLASS(node), obj, STRL_PAIR("children")-1, 0 TSRMLS_CC);
    add_next_index_zval(property, child);
    index = zend_hash_next_free_element(Z_ARRVAL_P(property))-1;
    if(IS_OBJECT == Z_TYPE_P(child)) {
        //-- set child's parent
        property = zend_read_property(META_CLASS(node), child, STRL_PAIR("parent")-1, 0 TSRMLS_CC);
        //TODO if old parent !IS_NULL, detach child from it
        META_UP_PROP(node, child, "parent", obj);
        //-- set child's index
        property = zend_read_property(META_CLASS(node), child, STRL_PAIR("index")-1, 0 TSRMLS_CC);
        META_UP_PROP(node, child, "index", property);
    }

    //TODO (signed/unsigned) integer overflow?
    RETURN_LONG(index);
}
/* }}} */
/* {{{ proto public void ASTNode::__destruct()
 */
PHP_METHOD(ASTNode, __destruct) {
    zval *property;
    zval *obj;

    obj = getThis();
    property = zend_read_property(META_CLASS(node), obj, STRL_PAIR("data")-1, 0 TSRMLS_CC);
    zval_ptr_dtor(&property);

    //Z_SET_REFCOUNT_P(property, 1);
    //META_ZDUMP(property);
    //property = zend_read_property(META_CLASS(node), obj, STRL_PAIR("parent")-1, 0 TSRMLS_CC); // TODO do same for "root"?
    //zval_ptr_dtor(&property);
}
/* }}} */
/* {{{ proto public string ASTNode::__toString()
 */
PHP_METHOD(ASTNode, __toString) {
	zval *property;
	zval *obj;
	zval *child;
	obj = getThis();
	property = zend_read_property(META_CLASS(node), obj, STRL_PAIR("data")-1, 0 TSRMLS_CC);
	if(IS_NULL != Z_TYPE_P(property)) {
		//TODO return property
	}
	else {
		//iterate children
		//use concat_function() from zend_operators.c
	}
    RETURN_STRINGL("hello ", sizeof("hello"), 1);
}
/* }}} */
/* }}} */
//TODO arg info for all methods
/* {{{ ASTNode methods
 */
static const function_entry php_meta_astnode_functions[] = {
    PHP_ME(ASTNode, __construct,        php_meta_fiveargs, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
    PHP_ME(ASTNode, __destruct,         NULL, ZEND_ACC_DTOR|ZEND_ACC_PUBLIC)
    PHP_ME(ASTNode, __toString,         NULL, ZEND_ACC_PUBLIC)
    PHP_ME(ASTNode, setParentNode,      php_meta_onearg, ZEND_ACC_PUBLIC)
    PHP_ME(ASTNode, appendChild,        php_meta_onearg, ZEND_ACC_PUBLIC)
    ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 */

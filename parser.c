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
#include <zend_interfaces.h>
#include <zend_operators.h>
#include <php.h>
#include <standard/php_string.h>
#include "meta_parser.h" /* for T_ terminal definitions */
#include "meta_scanner.h"
#include "scanner_API.h"
#define _INTERNAL
#include "parser.h"

/* {{{ create and initialize internal classes
 */
int meta_parser_init_function(INIT_FUNC_ARGS) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, PHP_META_ASTTREEISH_CE_NAME, php_meta_asttreeish_functions);
	META_CLASS(treeish) = zend_register_internal_class(&ce TSRMLS_CC);
	META_CLASS(treeish)->ce_flags |= ZEND_ACC_INTERFACE;

	INIT_CLASS_ENTRY(ce, PHP_META_ASTNODE_CE_NAME, php_meta_astnode_functions);
	META_CLASS(node) = zend_register_internal_class(&ce TSRMLS_CC);
	META_CLASS(node)->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
	META_PROP_ZERO(node, "type", PROTECTED);
	META_PROP_NULL(node, "root", PROTECTED);
	META_PROP_NULL(node, "parent", PROTECTED);
	META_PROP_NULL(node, "index", PROTECTED);
	META_PROP_ZERO(node, "start_line", PROTECTED);
	META_PROP_ZERO(node, "end_line", PROTECTED);
	zend_class_implements(META_CLASS(node) TSRMLS_CC, 1, META_CLASS(treeish));

	INIT_CLASS_ENTRY(ce, PHP_META_ASTNODELIST_CE_NAME, php_meta_astnodelist_functions);
	META_CLASS(nodelist) = zend_register_internal_class(&ce TSRMLS_CC);
	META_PROP_NULL(nodelist, "root", PROTECTED);
	META_PROP_NULL(nodelist, "parent", PROTECTED);
	META_PROP_L(nodelist, "index", PROTECTED, -1);
	META_PROP_ZERO(nodelist, "start_line", PROTECTED);
	META_PROP_ZERO(nodelist, "end_line", PROTECTED);
	META_PROP_NULL(nodelist, "children", PROTECTED);
	memcpy(&nodelist_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	META_CLASS(nodelist)->create_object = create_object_nodelist;
	zend_class_implements(META_CLASS(nodelist) TSRMLS_CC, 1, META_CLASS(treeish)); /* TODO subtree instead of treeish */

	INIT_CLASS_ENTRY(ce, PHP_META_ASTTREE_CE_NAME, php_meta_asttree_functions);
	META_CLASS(tree) = zend_register_internal_class_ex(&ce, META_CLASS(nodelist), PHP_META_ASTNODELIST_CE_NAME TSRMLS_CC);
	META_PROP_NULL(tree, "source", PROTECTED);
	META_PROP_ZERO(tree, "flags", PROTECTED);
	memcpy(&tree_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	META_CLASS(tree)->create_object = create_object_tree;

	INIT_CLASS_ENTRY(ce, PHP_META_ASTUNARYNODE_CE_NAME, php_meta_astunarynode_functions);
	META_CLASS(unarynode) = zend_register_internal_class_ex(&ce, META_CLASS(node), PHP_META_ASTNODE_CE_NAME TSRMLS_CC);
	META_PROP_NULL(unarynode, "operator", PROTECTED);
	META_PROP_NULL(unarynode, "fill", PROTECTED);
	META_PROP_NULL(unarynode, "operand", PROTECTED);
	memcpy(&unarynode_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	META_CLASS(unarynode)->create_object = create_object_unarynode;

	INIT_CLASS_ENTRY(ce, PHP_META_ASTBINARYNODE_CE_NAME, php_meta_astbinarynode_functions);
	META_CLASS(binarynode) = zend_register_internal_class_ex(&ce, META_CLASS(node), PHP_META_ASTNODE_CE_NAME TSRMLS_CC);
	META_PROP_NULL(binarynode, "lhs", PROTECTED);
	META_PROP_NULL(binarynode, "rhs", PROTECTED);
	META_PROP_NULL(binarynode, "operator", PROTECTED);
	META_PROP_NULL(binarynode, "between_lhs_operator", PROTECTED);
	META_PROP_NULL(binarynode, "between_operator_rhs", PROTECTED);
	META_PROP_NULL(binarynode, "after_rhs", PROTECTED);
	memcpy(&binarynode_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	META_CLASS(binarynode)->create_object = create_object_binarynode;

	INIT_CLASS_ENTRY(ce, PHP_META_ASTTERNARYNODE_CE_NAME, php_meta_astternarynode_functions);
	META_CLASS(ternarynode) = zend_register_internal_class_ex(&ce, META_CLASS(node), PHP_META_ASTNODE_CE_NAME TSRMLS_CC);
	META_PROP_NULL(ternarynode, "condition", PROTECTED);
	META_PROP_NULL(ternarynode, "true", PROTECTED);
	META_PROP_NULL(ternarynode, "false", PROTECTED);

	/* TODO register terminals and nonterminals numeric constants and names, from meta_parser_defs.h (write script for it) */
	/* TODO register symbolic constants for flags */

	return SUCCESS;
}
/* }}} */
/* {{{ interface Treeish */
/* {{{ proto public void Treeish::setRoot(ASTTree $root) */
/* no implementation, just an interface */
/* }}} */
/* {{{ proto public ASTTree Treeish::getRoot() */
/* no implementation, just an interface */
/* }}} */
/* {{{ proto public void Treeish::setParent(Treeish $node) */
/* no implementation, just an interface */
/* }}} */
/* {{{ proto public Treeish Treeish::getParent() */
/* no implementation, just an interface */
/* }}} */
/* {{{ proto public void Treeish::setIndex(int $index) */
/* no implementation, just an interface */
/* }}} */
/* {{{ proto public int Treeish::getIndex() */
/* no implementation, just an interface */
/* }}} */
/* {{{ Treeish methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_treeish_setroot, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, root, ASTTree, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_treeish_getroot, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_treeish_setparent, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, parent, Treeish, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_treeish_getparent, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_treeish_setindex, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_treeish_getindex, 0, 0, 0)
ZEND_END_ARG_INFO()

static const function_entry php_meta_asttreeish_functions[] = {
	PHP_ABSTRACT_ME(Treeish, setRoot,			arginfo_treeish_setroot)
	PHP_ABSTRACT_ME(Treeish, getRoot,			arginfo_treeish_getroot)
	PHP_ABSTRACT_ME(Treeish, setParent,			arginfo_treeish_setparent)
	PHP_ABSTRACT_ME(Treeish, getParent,			arginfo_treeish_getparent)
	PHP_ABSTRACT_ME(Treeish, setIndex,			arginfo_treeish_setindex)
	PHP_ABSTRACT_ME(Treeish, getIndex,			arginfo_treeish_getindex)
	ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* }}} */
/* {{{ abstract class ASTNode
 * Base class for all nodes in the tree, except ASTNodeList (and ASTTree) */
/* {{{ proto public void ASTNode::setLines(int $start, int $end)
 * Set the lines across this node spreads out */
PHP_METHOD(ASTNode, setLines) {
	long start, end;
	zval *obj;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll",
	                                    &start, &end)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP_L(node, obj, "start_line", start);
	META_UP_PROP_L(node, obj, "end_line", end);
}
/* }}} */
/* {{{ proto public void ASTNode::setRoot(ASTTree $root)
 * Set the tree to which this node belongs to */
PHP_METHOD(ASTNode, setRoot) {
	zval *obj, *root, *old_root;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &root, META_CLASS(tree))) {
		WRONG_PARAM_COUNT;
	}

	obj = getThis();

	old_root = zend_read_property(META_CLASS(node), obj, STRL_PAIR("root")-1, 0 TSRMLS_CC);
	if(old_root != root) {
		if(IS_NULL != Z_TYPE_P(old_root)) {
			/* TODO detach from old root, if different */
		}
		/* TODO notify the root? */
		META_UP_PROP(node, obj, "root", root);
		Z_ADDREF_P(root);
	}
}
/* }}} */
/* {{{ proto public ASTTree ASTNode::getRoot() */
PHP_METHOD(ASTNode, getRoot) {
	zval *index;
	index = zend_read_property(META_CLASS(node), getThis(), STRL_PAIR("index")-1, 0 TSRMLS_CC);
	RETURN_ZVAL(index, 0, 1);
}
/* }}} */
/* {{{ proto public void ASTNode::setParent(Treeish $parent)
 * Set the parent */
PHP_METHOD(ASTNode, setParent) {
	zval *obj, *parent, *old_parent;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &parent, META_CLASS(treeish))) {
		php_error_docref(NULL TSRMLS_CC, E_USER_WARNING, "Parent is not a valid tree node");
		return;
	}

	obj = getThis();

	old_parent = zend_read_property(META_CLASS(node), obj, STRL_PAIR("parent")-1, 0 TSRMLS_CC);
	if(old_parent != parent) {
		if(IS_NULL != Z_TYPE_P(old_parent)) {
			/* TODO detach from old parent, if different */
		}
		/* TODO notify the parent? */
		META_UP_PROP(node, obj, "parent", parent);
		Z_ADDREF_P(parent);
	}
}
/* }}} */
/* {{{ proto public Treeish ASTNode::getParent() */
PHP_METHOD(ASTNode, getParent) {
	zval *parent;
	parent = zend_read_property(META_CLASS(node), getThis(), STRL_PAIR("parent")-1, 0 TSRMLS_CC);
	RETURN_ZVAL(parent, 0, 1);
}
/* }}} */
/* {{{ proto public void ASTNode::setIndex(int index)
 * set the index of this node within the parent */
PHP_METHOD(ASTNode, setIndex) {
	zval *obj, *parent;
	long index;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
	                                    &index)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	parent = zend_read_property(META_CLASS(node), obj, STRL_PAIR("parent")-1, 0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(parent)) {
		php_error_docref(NULL TSRMLS_CC, E_USER_WARNING, "Cannot set index without a parent");
		return;
	}
	else {
		/* TODO check if index is valid, call parent perhaps? */
	}
	META_UP_PROP_L(node, obj, "index", index);
}
/* }}} */
/* {{{ proto public int ASTNode::getIndex() */
PHP_METHOD(ASTNode, getIndex) {
	zval *index;
	index = zend_read_property(META_CLASS(node), getThis(), STRL_PAIR("index")-1, 0 TSRMLS_CC);
	RETURN_ZVAL(index, 0, 1);
}
/* }}} */
/* {{{ ASTNode methods
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_node_setlines, 0, 0, 2)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_node_setroot, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, root, ASTTree, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_node_getroot, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_node_setparent, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, parent, Treeish, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_node_getparent, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_node_setindex, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_node_getindex, 0, 0, 0)
ZEND_END_ARG_INFO()

static const function_entry php_meta_astnode_functions[] = {
	PHP_ME(ASTNode, setLines,			arginfo_node_setlines, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNode, setRoot,			arginfo_node_setroot, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNode, getRoot,			arginfo_node_getroot, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNode, setParent,			arginfo_node_setparent, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNode, getParent,			arginfo_node_getparent, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNode, setIndex,			arginfo_node_setindex, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNode, getIndex,			arginfo_node_getindex, ZEND_ACC_PUBLIC)
	ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* }}} */
/* {{{ class ASTNodeList */
/* {{{ internal handlers */
static zend_object_handlers nodelist_handlers;
static void meta_nodelist_free(void *object TSRMLS_DC) {
	zend_object *obj;
	zval **property;

	obj = object;
	zend_objects_free_object_storage(obj TSRMLS_CC);
}
static void meta_nodelist_dtor(void *object, zend_object_handle handle TSRMLS_DC) {
	zend_object *obj;
	zval **property;
	obj = object;
	META_DECREF_HTITEM(obj, "*", "children", property);
	//META_DECREF_HTITEM(obj, "*", "root", property);
	zend_objects_destroy_object(obj, handle TSRMLS_CC);
}
static zend_object_value create_object_nodelist(zend_class_entry* ce TSRMLS_DC) {
	zend_object_value retval;
	zend_object *obj;
	zval *property;

	obj = emalloc(sizeof(zend_object));
	zend_object_std_init(obj, ce TSRMLS_CC);
	zend_hash_copy(obj->properties, &ce->default_properties, (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval*));

	MAKE_STD_ZVAL(property);
	array_init(property);
	META_UPDATE_HPROPERTY(obj, "*", "children", property);

	retval.handle = zend_objects_store_put(obj, meta_nodelist_dtor, meta_nodelist_free, NULL TSRMLS_CC);
	retval.handlers = &nodelist_handlers;
	return retval;
}
/* }}} */
/* {{{ proto public void ASTNodeList::__construct(ASTTree $tree) */
PHP_METHOD(ASTNodeList, __construct) {
	zval *obj, *root;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O",
	                                    &root, META_CLASS(tree))) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP(nodelist, obj, "root", root);
}
/* }}} */
/* {{{ proto public string ASTNodeList::__toString() */
PHP_METHOD(ASTNodeList, __toString) {
	zval *obj, *children, *delim;

	obj = getThis();
	children = zend_read_property(META_CLASS(nodelist), obj, STRL_PAIR("children")-1, 0 TSRMLS_CC);
	ALLOC_INIT_ZVAL(delim);
	php_implode(delim, children, return_value TSRMLS_CC);
	zval_ptr_dtor(&delim);
}
/* }}} */
/* {{{ proto public void ASTNodeList::setRoot(ASTTree $root)
 * Set the tree to which this nodelist belongs to */
PHP_METHOD(ASTNodeList, setRoot) {
	zval *obj, *root, *old_root;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &root, META_CLASS(tree))) {
		WRONG_PARAM_COUNT;
	}

	obj = getThis();

	old_root = zend_read_property(META_CLASS(nodelist), obj, STRL_PAIR("root")-1, 0 TSRMLS_CC);
	if(old_root != root) {
		if(IS_NULL != Z_TYPE_P(old_root)) {
			/* TODO detach from old root, if different */
		}
		/* TODO notify the root? */
		META_UP_PROP(nodelist, obj, "root", root);
		Z_ADDREF_P(root);
	}
}
/* }}} */
/* {{{ proto public ASTTree ASTNodeList::getRoot() */
PHP_METHOD(ASTNodeList, getRoot) {
	zval *index;
	index = zend_read_property(META_CLASS(nodelist), getThis(), STRL_PAIR("index")-1, 0 TSRMLS_CC);
	RETURN_ZVAL(index, 0, 1);
}
/* }}} */
/* {{{ proto public void ASTNodeList::setParent(Treeish $parent)
 * Set the parent, an ASTNodeList or ASTNodeList */
PHP_METHOD(ASTNodeList, setParent) {
	zval *obj, *parent, *old_parent;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &parent, META_CLASS(treeish))) {
		php_error_docref(NULL TSRMLS_CC, E_USER_WARNING, "Parent is not a valid tree node");
		return;
	}

	obj = getThis();

	old_parent = zend_read_property(META_CLASS(nodelist), obj, STRL_PAIR("parent")-1, 0 TSRMLS_CC);
	if(old_parent != parent) {
		if(IS_NULL != Z_TYPE_P(old_parent)) {
			/* TODO detach from old parent, if different */
		}
		/* TODO notify the parent? */
		META_UP_PROP(nodelist, obj, "parent", parent);
		Z_ADDREF_P(parent);
	}
}
/* }}} */
/* {{{ proto public Treeish ASTNodeList::getParent() */
PHP_METHOD(ASTNodeList, getParent) {
	zval *parent;
	parent = zend_read_property(META_CLASS(nodelist), getThis(), STRL_PAIR("parent")-1, 0 TSRMLS_CC);
	RETURN_ZVAL(parent, 0, 1);
}
/* }}} */
/* {{{ proto public void ASTNodeList::setIndex(int index)
 * set the index of this node within the parent */
PHP_METHOD(ASTNodeList, setIndex) {
	zval *obj, *parent;
	long index;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &index)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	parent = zend_read_property(META_CLASS(nodelist), obj, STRL_PAIR("parent")-1, 0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(parent)) {
		php_error_docref(NULL TSRMLS_CC, E_USER_WARNING, "Cannot set index without a parent");
		return;
	}
	else {
		/* TODO check if index is valid, call parent perhaps? */
	}
	META_UP_PROP_L(nodelist, obj, "index", index);
}
/* }}} */
/* {{{ proto public int ASTNodeList::getIndex() */
PHP_METHOD(ASTNodeList, getIndex) {
	zval *index;
	index = zend_read_property(META_CLASS(nodelist), getThis(), STRL_PAIR("index")-1, 0 TSRMLS_CC);
	RETURN_ZVAL(index, 0, 1);
}
/* }}} */
/* {{{ proto public void ASTNodeList::appendChild(mixed $node) */
PHP_METHOD(ASTNodeList, appendChild) {
	zval *obj, *children, *child;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
	                                    &child)) {
		WRONG_PARAM_COUNT;
	}
	Z_ADDREF_P(child);
	obj = getThis();
	children = zend_read_property(META_CLASS(nodelist), obj, STRL_PAIR("children")-1, 0 TSRMLS_CC);
	add_next_index_zval(children, child);
	if(IS_OBJECT == Z_TYPE_P(child)) { /* TODO actually check for a marker interface */
		zend_class_entry *child_ce;
		zend_function *setparent=NULL;
		zval *retval;
		child_ce = Z_OBJCE_P(child);
		if(FAILURE == zend_hash_find(&child_ce->function_table, STRL_PAIR("setparent"), (void**)&setparent)) {
			php_error_docref(NULL TSRMLS_CC, E_USER_WARNING, "Child does not have a setParent method");
			return;
		}
		retval = obj_call_method_internal_ex(child, child_ce, setparent, EG(scope), 1 TSRMLS_CC, "z", obj);
		if(NULL != retval) {
			zval_ptr_dtor(&retval);
		}
	}
}
/* }}} */
/* {{{ proto public bool ASTNodeList::hasChildren() */
PHP_METHOD(ASTNodeList, hasChildren) {
	zval *children;
	children = zend_read_property(META_CLASS(nodelist), getThis(), STRL_PAIR("children")-1, 0 TSRMLS_CC);
	RETURN_BOOL(zend_hash_num_elements(Z_ARRVAL_P(children)));
}
/* }}} */
/* {{{ proto public void ASTNodeList::setLines(int $start, $int $end) */
PHP_METHOD(ASTNodeList, setLines) {
	long start, end;
	zval *obj;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll",
	                                    &start, &end)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP_L(nodelist, obj, "start_line", start);
	META_UP_PROP_L(nodelist, obj, "end_line", end);
}
/* }}} */
/* {{{ ASTNodeList methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_construct, 0, 0, 1)
ZEND_ARG_OBJ_INFO(0, tree, ASTTree, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_tostring, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_setroot, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, root, ASTTree, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_getroot, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_setparent, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, parent, Treeish, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_getparent, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_setindex, 0, 0, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_getindex, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_appendchild, 0, 0, 1)
	ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_haschildren, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_nodelist_setlines, 0, 0, 2)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO()

static const function_entry php_meta_astnodelist_functions[] = {
	PHP_ME(ASTNodeList, __construct,		arginfo_nodelist_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, __toString,			arginfo_nodelist_tostring, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, setRoot,			arginfo_nodelist_setroot, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, getRoot,			arginfo_nodelist_getroot, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, setParent,			arginfo_nodelist_setparent, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, getParent,			arginfo_nodelist_getparent, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, setIndex,			arginfo_nodelist_setindex, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, getIndex,			arginfo_nodelist_getindex, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, appendChild,		arginfo_nodelist_appendchild, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, hasChildren,		arginfo_nodelist_haschildren, ZEND_ACC_PUBLIC)
	PHP_ME(ASTNodeList, setLines,			arginfo_nodelist_setlines, ZEND_ACC_PUBLIC)
	ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* }}} */
/* {{{ class ASTTree extends ASTNodeList */
/* {{{ internal handlers */

static zend_object_handlers tree_handlers;

static void meta_tree_free(void *object TSRMLS_DC) {
	zend_object *obj;
	zval **property;

	obj = object;
	zend_objects_free_object_storage(obj TSRMLS_CC);
}

static void meta_tree_dtor(void *object, zend_object_handle handle TSRMLS_DC) {
	zend_object *obj;
	zval **property;
	obj = object;
	META_DECREF_HTITEM(obj, "*", "children", property);
	zend_objects_destroy_object(obj, handle TSRMLS_CC);
}

static zend_object_value create_object_tree(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value retval;
	zend_object *obj;
	zval *property;

	obj = emalloc(sizeof(zend_object));
	zend_object_std_init(obj, ce TSRMLS_CC);
	zend_hash_copy(obj->properties, &ce->default_properties, (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval*));

	MAKE_STD_ZVAL(property);
	array_init(property);
	META_UPDATE_HPROPERTY(obj, "*", "children", property);

	retval.handle = zend_objects_store_put(obj, meta_tree_dtor, meta_tree_free, NULL TSRMLS_CC);
	retval.handlers = &tree_handlers;

	return retval;
}
/* }}} */
/* {{{ proto public void ASTTree::__construct(int $flags [,mixed $source])
 * Construct the root of the tree */
PHP_METHOD(ASTTree, __construct) {
	zval *obj;
	long flags;
	zval *source;

	source=NULL;
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|z",
	                                    &flags, &source)) {
		WRONG_PARAM_COUNT;
	}
	if(NULL != source && (IS_STRING != Z_TYPE_P(source) /* || is stream */)) {
		php_error_docref(NULL TSRMLS_CC, E_USER_WARNING, "The source is not a string or a stream");
		return;
	}
	obj = getThis();
	META_UP_PROP_L(tree, obj, "flags", flags);
	if(source) {
		META_UP_PROP(tree, obj, "source", source);
	}
	META_UP_PROP(tree, obj, "root", obj);
}
/* }}} */
/* {{{ proto public string ASTTree::__toString()
 * Return the string representation of the entire tree, which is valid PHP code
 * For now, it just inherits ASTNodeList, but what about encodings, BOM and the like? */
/* }}} */
/* {{{ proto public void ASTTree::setFlags(int $flags) */
PHP_METHOD(ASTTree, setFlags) {
	zval *obj;
	long flags;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
	                                    &flags)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP_L(tree, obj, "flags", flags);
}
/* }}} */
/* {{{ proto public int ASTTree::getFlags() */
PHP_METHOD(ASTTree, getFlags) {
	zval *flags;
	flags = zend_read_property(META_CLASS(tree), getThis(), STRL_PAIR("flags")-1, 0 TSRMLS_CC);
	RETURN_ZVAL(flags, 0, 1);
}
/* }}} */
/* {{{ proto public void ASTTree::setSource(mixed $source) */
PHP_METHOD(ASTTree, setSource) {
	zval *obj, *source;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
	                                    &source)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP(tree, obj, "source", source);
}
/* }}} */
/* {{{ proto public bool ASTTree::parse([int $flags])
 * Parse the tree, if modified in the meantime or there is a source to parse */
PHP_METHOD(ASTTree, parse) {
	zval *obj, *flags, *source;
	meta_scanner *scanner;
	TOKEN *token, *prev_token;
	void *parser;
	long major;

	flags = NULL;
	prev_token = NULL;
	token = NULL;
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &flags)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	source = zend_read_property(META_CLASS(tree), obj, STRL_PAIR("source")-1, 0 TSRMLS_CC);
	if(Z_TYPE_P(source) == IS_NULL) {
		RETURN_FALSE;
	}
	else {
		/* TODO what about cases when the source is not NULL but we've already got children inserted programmatically? */
	}
	if(!flags) {
		flags = zend_read_property(META_CLASS(tree), obj, STRL_PAIR("flags")-1, 0 TSRMLS_CC);
	}

	Z_ADDREF_P(source);
	scanner = meta_scanner_alloc(source, Z_LVAL_P(flags));
	parser = MetaParserAlloc(meta_alloc);

	do {
		token = meta_scan(scanner TSRMLS_CC);
		/* TODO check scanner->err_no */
		major = TOKEN_MAJOR(token);
		if(NULL == prev_token) {
			prev_token = token;
		}
		else {
			token->prev = prev_token;
			prev_token->next = token;
			prev_token = token;
		}
		if(TOKEN_IS_DISPENSABLE(token)) {
			continue;
		}
		MetaParser(parser, major, token, obj);
		if(major < 0) {
			/* TODO error reporting */
			break;
		}
		if(0 == major) {
			efree(token);
			break;
		}
	} while(major > 0);

	MetaParserFree(parser, meta_free);
	meta_scanner_free(&scanner);
	Z_DELREF_P(source);
	RETURN_TRUE;
}
/* }}} */
/* {{{ ASTTree methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_construct, 0, 0, 1)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_tostring, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_setflags, 0, 0, 1)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_getflags, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_setsource, 0, 0, 1)
	ZEND_ARG_INFO(0, source)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_parse, 0, 0, 0)
ZEND_END_ARG_INFO()


static const function_entry php_meta_asttree_functions[] = {
	PHP_ME(ASTTree, __construct,		arginfo_tree_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(ASTTree, setFlags,			arginfo_tree_setflags, ZEND_ACC_PUBLIC)
	PHP_ME(ASTTree, getFlags,			arginfo_tree_getflags, ZEND_ACC_PUBLIC)
	PHP_ME(ASTTree, setSource,			arginfo_tree_setsource, ZEND_ACC_PUBLIC)
	PHP_ME(ASTTree, parse,				arginfo_tree_parse, ZEND_ACC_PUBLIC)
	ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* }}} */
/* {{{ class ASTUnaryNode extends ASTNode
 * An unary node has an operand and (optionally) an operator. It can also hold unneeded nodes between the two, in the $fill member */
/* {{{ internal handlers */
static zend_object_handlers unarynode_handlers;
static void meta_unarynode_free(void *object TSRMLS_DC) {
	zend_object *obj;
	zval **property;

	obj = object;
	zend_objects_free_object_storage(obj TSRMLS_CC);
}
static void meta_unarynode_dtor(void *object, zend_object_handle handle TSRMLS_DC) {
	zend_object *obj;
	zval **property;

	obj = object;
	META_DECREF_HTITEM(obj, "*", "fill", property);
	//META_DECREF_HTITEM(obj, "*", "root", property);
	zend_objects_destroy_object(obj, handle TSRMLS_CC);
}
static zend_object_value create_object_unarynode(zend_class_entry* ce TSRMLS_DC) {
	zend_object_value retval;
	zend_object *obj;
	zval *property;

	obj = emalloc(sizeof(zend_object));
	zend_object_std_init(obj, ce TSRMLS_CC);
	zend_hash_copy(obj->properties, &ce->default_properties, (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval*));

	MAKE_STD_ZVAL(property);
	array_init(property);
	META_UPDATE_HPROPERTY(obj, "*", "fill", property);

	retval.handle = zend_objects_store_put(obj, meta_unarynode_dtor, meta_unarynode_free, NULL TSRMLS_CC);
	retval.handlers = &unarynode_handlers;
	return retval;
}

/* }}} */
/* {{{ proto public void ASTUnaryNode::__construct(int $type, ASTTree $tree [, mixed $operand [, string $operator]]) */
PHP_METHOD(ASTUnaryNode, __construct) {
	zval *obj, *operand, *root, *operator;
	long type;

	operand = NULL;
	operator = NULL;
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lO|zz",
	                                    &type, &root, META_CLASS(tree), &operand, &operator)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP_L(unarynode, obj, "type", type);
	META_UP_PROP(unarynode, obj, "root", root);
	if(NULL != operand) {
		META_UP_PROP(unarynode, obj, "operand", operand);
	}
	if(NULL != operator) {
		META_UP_PROP(unarynode, obj, "operator", operator);
	}
}
/* }}} */
/* {{{ proto public void ASTUnaryNode::__toString()
 * Serialize an unary node to PHP code. This will also contain any unneeded nodes, if so desired. */
PHP_METHOD(ASTUnaryNode, __toString) {
	zval *obj, *property, *delim, *fill;

	obj = getThis();
	RETVAL_EMPTY_STRING();
	property = zend_read_property(META_CLASS(unarynode), obj, STRL_PAIR("operator")-1, 0 TSRMLS_CC);
	if(IS_NULL == Z_TYPE_P(property)) {
		/* TODO get default representation for operator $type */
	}
	if(FAILURE == concat_function(return_value, return_value, property TSRMLS_CC)) {
		/* TODO never occurs */
	}
	ALLOC_INIT_ZVAL(delim);
	ALLOC_INIT_ZVAL(fill);
	property = zend_read_property(META_CLASS(unarynode), obj, STRL_PAIR("fill")-1, 0 TSRMLS_CC);
	/* TODO XXX if property is empty, fill it with default values (like the ' ' between echo and an expression) */
	php_implode(delim, property, fill TSRMLS_CC);
	zval_ptr_dtor(&delim);
	if(FAILURE == concat_function(return_value, return_value, fill TSRMLS_CC)) {
		/* TODO but concat_function never returns FAILURE */
	}
	zval_ptr_dtor(&fill);
	property = zend_read_property(META_CLASS(unarynode), obj, STRL_PAIR("operand")-1, 0 TSRMLS_CC);
	if(FAILURE == concat_function(return_value, return_value, property TSRMLS_CC)) {
		/* TODO never occurs */
	}
}
/* }}} */
/* {{{ proto public void ASTUnaryNode::setOperand(mixed $operand) */
PHP_METHOD(ASTUnaryNode, setOperand) {
	zval *obj, *operand;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &operand)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP(unarynode, obj, "operand", operand);
}
/* }}} */
/* {{{ proto public void ASTUnaryNode::setOpRepresentation(string $operator)
 * Sometimes you will want something else than the default representation of an operator. */
PHP_METHOD(ASTUnaryNode, setOpRepresentation) {
	zval *obj, *operator;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &operator)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP(unarynode, obj, "operator", operator);
}
/* }}} */
/* {{{ proto public void ASTUnaryNode::appendBetween(mixed $child)
 * Append $child to the nodes filling the space between the operator and the operand. */
PHP_METHOD(ASTUnaryNode, appendBetween) {
	zval *obj, *fill, *child;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
	                                    &child)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	Z_ADDREF_P(child);
	fill = zend_read_property(META_CLASS(unarynode), obj, STRL_PAIR("fill")-1, 0 TSRMLS_CC);
	add_next_index_zval(fill, child);
}

/* }}} */
/* {{{ ASTUnaryNode methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_unarynode_construct, 0, 0, 2)
	ZEND_ARG_INFO(0, type)
ZEND_ARG_OBJ_INFO(0, tree, ASTTree, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_unarynode_tostring, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_unarynode_setoperand, 0, 0, 1)
	ZEND_ARG_INFO(0, operand)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_unarynode_setoprepresentation, 0, 0, 1)
	ZEND_ARG_INFO(0, operator)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_unarynode_appendbetween, 0, 0, 1)
	ZEND_ARG_INFO(0, child)
ZEND_END_ARG_INFO()

static const function_entry php_meta_astunarynode_functions[] = {
	PHP_ME(ASTUnaryNode, __construct,			arginfo_unarynode_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(ASTUnaryNode, __toString,			arginfo_unarynode_tostring, ZEND_ACC_PUBLIC)
	PHP_ME(ASTUnaryNode, setOperand,			arginfo_unarynode_setoperand, ZEND_ACC_PUBLIC)
	PHP_ME(ASTUnaryNode, setOpRepresentation,	arginfo_unarynode_setoprepresentation, ZEND_ACC_PUBLIC)
	PHP_ME(ASTUnaryNode, appendBetween,			arginfo_unarynode_appendbetween, ZEND_ACC_PUBLIC)
	ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* }}} */
/* {{{ class ASTBinaryNode extends ASTNode */
/* {{{ internal handlers */
static zend_object_handlers binarynode_handlers;
static void meta_binarynode_free(void *object TSRMLS_DC) {
	zend_object *obj;
	zval **property;

	obj = object;
	zend_objects_free_object_storage(obj TSRMLS_CC); // zend_object_std_dtor + efree
}
static void meta_binarynode_dtor(void *object, zend_object_handle handle TSRMLS_DC) {
	zend_object *obj;
	zval **property;

	obj = object;
	META_DECREF_HTITEM(obj, "*", "between_lhs_operator", property);
	META_DECREF_HTITEM(obj, "*", "between_operator_rhs", property);
	META_DECREF_HTITEM(obj, "*", "after_rhs", property);
	zend_objects_destroy_object(obj, handle TSRMLS_CC);
}
static zend_object_value create_object_binarynode(zend_class_entry* ce TSRMLS_DC) {
	zend_object_value retval;
	zend_object *obj;
	zval *property;

	obj = emalloc(sizeof(zend_object));
	zend_object_std_init(obj, ce TSRMLS_CC);
	zend_hash_copy(obj->properties, &ce->default_properties, (copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval*));

	MAKE_STD_ZVAL(property);
	array_init(property);
	META_UPDATE_HPROPERTY(obj, "*", "between_lhs_operator", property);
	MAKE_STD_ZVAL(property);
	array_init(property);
	META_UPDATE_HPROPERTY(obj, "*", "between_operator_rhs", property);
	MAKE_STD_ZVAL(property);
	array_init(property);
	META_UPDATE_HPROPERTY(obj, "*", "after_rhs", property);

	retval.handle = zend_objects_store_put(obj, meta_binarynode_dtor, meta_binarynode_free, NULL TSRMLS_CC);
	retval.handlers = &binarynode_handlers;
	return retval;
}
/* }}} */
/* {{{ proto public void ASTBinaryNode::__construct(int $type, ASTTree $tree [, mixed $lhs, mixed $rhs [, mixed $operator]])
 * A binary node has a LHS, a RHS, and an operator. It can also hold unneeded nodes between found between them in the original source. */
PHP_METHOD(ASTBinaryNode, __construct) {
	zval *obj, *root;
	zval *lhs, *operator, *rhs;
	long type;

	lhs=NULL;
	rhs=NULL;
	operator=NULL;
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lO|zzz",
	                                    &type, &root, META_CLASS(tree), &lhs, &rhs, &operator)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	/* bit twiddling with pointers, yes, not readable, but, more compact than nested if/else */
	if((!!(size_t)lhs ^ !!(size_t)rhs)) {
		/* TODO error: either both lhs and rhs, or none */
		/* TODO free memory? */
		return;
	}
	else {
		META_UP_PROP(binarynode, obj, "lhs", lhs);
		META_UP_PROP(binarynode, obj, "rhs", rhs);
	}

	META_UP_PROP_L(binarynode, obj, "type", type);
	META_UP_PROP(binarynode, obj, "root", root);
	if(operator) {
		META_UP_PROP(binarynode, obj, "operator", operator);
	}
}
/* }}} */
/* {{{ proto public string ASTBinaryNode::__toString()
 * Serialize the node to PHP code. */
PHP_METHOD(ASTBinaryNode, __toString) {
	zval *obj, *property;

	obj = getThis();
	RETVAL_EMPTY_STRING();
	property = zend_read_property(META_CLASS(binarynode), obj, STRL_PAIR("lhs")-1, 0 TSRMLS_CC);
	if(FAILURE == concat_function(return_value, return_value, property TSRMLS_CC)) {
		/* TODO error reporting, though concat_function never returns FAILURE (? fix the engine) */
	}
	/* TODO check root's flags and serialize between_lhs_operator if needed */
	property = zend_read_property(META_CLASS(binarynode), obj, STRL_PAIR("operator")-1, 0 TSRMLS_CC);
	if(FAILURE == concat_function(return_value, return_value, property TSRMLS_CC)) {
		/* TODO error reporting, though concat_function never returns FAILURE (? fix the engine) */
	}
	/* TODO check root's flags and serialize between_operator_rhs if needed */
	property = zend_read_property(META_CLASS(binarynode), obj, STRL_PAIR("rhs")-1, 0 TSRMLS_CC);
	if(FAILURE == concat_function(return_value, return_value, property TSRMLS_CC)) {
		/* TODO error reporting, though concat_function never returns FAILURE (? fix the engine) */
	}
}
/* }}} */
/* {{{ proto public void ASTBinaryNode::appendBetween($child, $where)
 * Append $child to the filling area $where. */
PHP_METHOD(ASTBinaryNode, appendBetween) {
	zval *obj, *fill, *child;
	long where;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl", &child, &where)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	switch(where) {
	case META_FILL_BINARY_LHS_OPERATOR:
		fill = zend_read_property(META_CLASS(binarynode), obj, STRL_PAIR("between_lhs_operator")-1, 0 TSRMLS_CC);
		break;
	case META_FILL_BINARY_OPERATOR_RHS:
		fill = zend_read_property(META_CLASS(binarynode), obj, STRL_PAIR("between_operator_rhs")-1, 0 TSRMLS_CC);
		break;
	case META_FILL_BINARY_AFTER_RHS:
		fill = zend_read_property(META_CLASS(binarynode), obj, STRL_PAIR("after_rhs")-1, 0 TSRMLS_CC);
		break;
	default:
		/* TODO error: invalid place to append child to */
		return;
	}
	Z_ADDREF_P(child);
	add_next_index_zval(fill, child);
}
/* }}} */
/* {{{ proto public void ASTBinaryNode::setLHS(mixed $node)
 * Set or change the left hand-side of the binary operation. */
PHP_METHOD(ASTBinaryNode, setLHS) {
	zval *obj, *lhs;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
	                                    &lhs)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP(binarynode, obj, "lhs", lhs);
}
/* }}} */
/* {{{ proto public void ASTBinaryNode::setOpRepresentation(string $representation)
 * Set the string representation of the operator. */
/* NOTE: right now this allows me to do both, specify "iNsTanCeOf" instead of "instanceof", but also turn a '+' into a '*', which does not match $type any more
 * TODO: allow this only for $type's (major numbers) whose operator can have different spellings without affecting the semantics */
PHP_METHOD(ASTBinaryNode, setOpRepresentation) {
	zval *obj, *operator;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
	                                    &operator)) {
		WRONG_PARAM_COUNT;
	}
	/* TODO accept only string (?) */
	obj = getThis();
	META_UP_PROP(binarynode, obj, "operator", operator);
}
/* }}} */
/* {{{ proto public void ASTBinaryNode::setRHS(mixed $node) */
PHP_METHOD(ASTBinaryNode, setRHS) {
	zval *obj, *rhs;

	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z",
	                                    &rhs)) {
		WRONG_PARAM_COUNT;
	}
	obj = getThis();
	META_UP_PROP(binarynode, obj, "rhs", rhs);
}
/* }}} */
/* {{{ ASTBinaryNode methods */
ZEND_BEGIN_ARG_INFO_EX(arginfo_binarynode_construct, 0, 0, 2)
	ZEND_ARG_INFO(0, type)
ZEND_ARG_OBJ_INFO(0, tree, ASTTree, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_binarynode_tostring, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_binarynode_appendbetween, 0, 0, 2)
	ZEND_ARG_INFO(0, child)
	ZEND_ARG_INFO(0, where)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_binarynode_setlhs, 0, 0, 1)
	ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_binarynode_setoprepresentation, 0, 0, 1)
	ZEND_ARG_INFO(0, representation)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_binarynode_setrhs, 0, 0, 1)
	ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()

static const function_entry php_meta_astbinarynode_functions[] = {
	PHP_ME(ASTBinaryNode, __construct,			arginfo_binarynode_construct, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	PHP_ME(ASTBinaryNode, __toString,			arginfo_binarynode_tostring, ZEND_ACC_PUBLIC)
	PHP_ME(ASTBinaryNode, appendBetween,		arginfo_binarynode_appendbetween, ZEND_ACC_PUBLIC)
	PHP_ME(ASTBinaryNode, setLHS,				arginfo_binarynode_setlhs, ZEND_ACC_PUBLIC)
	PHP_ME(ASTBinaryNode, setOpRepresentation,	arginfo_binarynode_setoprepresentation, ZEND_ACC_PUBLIC)
	PHP_ME(ASTBinaryNode, setRHS,				arginfo_binarynode_setrhs, ZEND_ACC_PUBLIC)
	ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* }}} */
/* {{{ class ASTTernaryNode extends ASTNode
 * Not yet implemented */
/* {{{ proto public void ASTTernaryNode::__construct */
PHP_METHOD(ASTTernaryNode, __construct) {

}
/* }}} */
/* {{{ ASTTernaryNode methods */
static const function_entry php_meta_astternarynode_functions[] = {
	PHP_ME(ASTTernaryNode, __construct, NULL, ZEND_ACC_CTOR|ZEND_ACC_PUBLIC)
	ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* }}} */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 */

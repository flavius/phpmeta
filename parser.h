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

/**
 * API that influences the runtime too
 */
#ifndef PARSER_H
#define PARSER_H
#include <zend.h>
#include <php.h>
#include <limits.h>


/* TODO move the following to parser_API.h? */
#define META_CLASS(cls) php_meta_ast ## cls ## _ce

/* convenience macros */
#define META_UP_PROP(class, obj, prop, value) zend_update_property(META_CLASS(class), obj, prop, sizeof(prop)-1, value TSRMLS_CC)
#define META_UP_PROP_L(class, obj, prop, value) zend_update_property_long(META_CLASS(class), obj, prop, sizeof(prop)-1, value TSRMLS_CC)
/* it is designed to work with statically allocated strings ONLY */
#define STRL_PAIR(str) str, sizeof(str)

/**
 * the initialization function, called by MINIT
 */
int meta_parser_init_function(INIT_FUNC_ARGS);

/**
 * Treeish interface, has a parent, a root tree to which it belongs, and it's serializable
 */
zend_class_entry *php_meta_asttreeish_ce;
#define PHP_META_ASTTREEISH_CE_NAME "Treeish"

/**
 * Subtree interface, inherits Treeish, and has children and it's iterable
 */
zend_class_entry *php_meta_astsubtree_ce;
#define PHP_META_ASTSUBTREE_CE_NAME "Subtree"

/**
 * ASTNode, abstract class
 */
zend_class_entry *php_meta_astnode_ce;
#define PHP_META_ASTNODE_CE_NAME "ASTNode"

/**
 * ASTNodeList, a list of child nodes
 */
zend_class_entry *php_meta_astnodelist_ce;
#define PHP_META_ASTNODELIST_CE_NAME "ASTNodeList"

/**
 * ASTTree is basically a ASTNodeList, with some extra features
 */
zend_class_entry *php_meta_asttree_ce;
#define PHP_META_ASTTREE_CE_NAME "ASTTree"

/**
 * ASTUnaryNode, it has only one operand, and optionally an operator
 */
zend_class_entry *php_meta_astunarynode_ce;
#define PHP_META_ASTUNARYNODE_CE_NAME "ASTUnaryNode"

/**
 * ASTBinaryNode, it has an operator, an LHS, and a RHS
 */
zend_class_entry *php_meta_astbinarynode_ce;
#define PHP_META_ASTBINARYNODE_CE_NAME "ASTBinaryNode"

/**
 * Not yet really implemented, but it will be useful for the ?: operator
 */
zend_class_entry *php_meta_astternarynode_ce;
#define PHP_META_ASTTERNARYNODE_CE_NAME "ASTTernaryNode"

/**
 * Many nodes have "filling areas" for CST terminals.
 * These numbers are used to identify those areas across all nodes, homogenously
 */
/* TODO export them to the runtime */

#define META_FILL_BEFORE 0
#define META_FILL_AFTER INT_MAX

#define META_FILL_UNARY_BEFORE_OPERATOR META_FILL_BEFORE
#define META_FILL_UNARY_PREOPERATOR_OPERAND 2
#define META_FILL_UNARY_OPERAND_POSTOPERATOR 4
#define META_FILL_UNARY_AFTER_POSTOPERATOR META_FILL_AFTER
/* an unary node has its operator either pre or post (ASTUnaryNode::$subtype) */
#define META_UNARY_PREOPERATOR 1
#define META_UNARY_POSTOPERATOR 5
#define META_UNARY_NOP 0

#define META_FILL_BINARY_BEFORE_LHS META_FILL_BEFORE
#define META_FILL_BINARY_LHS_OPERATOR 2
#define META_FILL_BINARY_OPERATOR_RHS 4
#define META_FILL_BINARY_AFTER_RHS META_FILL_AFTER

#endif /* PARSER_H */

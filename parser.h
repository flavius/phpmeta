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

#ifndef PARSER_H
#define PARSER_H
#include <zend.h>
#include <php.h>


//TODO move the following into php_meta.h?
#define META_CLASS(cls) php_meta_ast ## cls ## _ce
#define META_UP_PROP(class, obj, prop, value) zend_update_property(META_CLASS(class), obj, prop, sizeof(prop)-1, value TSRMLS_CC)
#define META_UP_PROP_L(class, obj, prop, value) zend_update_property_long(META_CLASS(class), obj, prop, sizeof(prop)-1, value TSRMLS_CC)
#define STRL_PAIR(str) str, sizeof(str)

//--- the following stay here
int meta_parser_init_function(INIT_FUNC_ARGS);

//-- ASTNode, abstract class
zend_class_entry *php_meta_astnode_ce;
#define PHP_META_ASTNODE_CE_NAME "ASTNode"

//-- ASTNodeList, only contains a list of isomorphic children
zend_class_entry *php_meta_astnodelist_ce;
#define PHP_META_ASTNODELIST_CE_NAME "ASTNodeList"

//-- ASTTree is basically a ASTNodeList, with some extra features
zend_class_entry *php_meta_asttree_ce;
#define PHP_META_ASTTREE_CE_NAME "ASTTree"

//-- ASTUnaryNode
zend_class_entry *php_meta_astunarynode_ce;
#define PHP_META_ASTUNARYNODE_CE_NAME "ASTUnaryNode"

//-- ASTBinaryNode
zend_class_entry *php_meta_astbinarynode_ce;
#define PHP_META_ASTBINARYNODE_CE_NAME "ASTBinaryNode"

//-- ASTTernaryNode
zend_class_entry *php_meta_astternarynode_ce;
#define PHP_META_ASTTERNARYNODE_CE_NAME "ASTTernaryNode"


/******** internal macros, functions and variables, only for parser.c */
#ifdef _INTERNAL
//internal macros, make coding more enjoyable
static const function_entry php_meta_astnode_functions[];
static const function_entry php_meta_astnodelist_functions[];
static const function_entry php_meta_asttree_functions[];
static const function_entry php_meta_astunarynode_functions[];
static const function_entry php_meta_astbinarynode_functions[];
static const function_entry php_meta_astternarynode_functions[];

static zend_object_handlers nodelist_handlers;
static zend_object_value create_object_nodelist(zend_class_entry* TSRMLS_DC);

static zend_object_handlers tree_handlers;
static zend_object_value create_object_tree(zend_class_entry *ce TSRMLS_DC);

static zend_object_handlers unarynode_handlers;
static zend_object_value create_object_unarynode(zend_class_entry* TSRMLS_DC);

static zend_object_handlers binarynode_handlers;
static zend_object_value create_object_binarynode(zend_class_entry* TSRMLS_DC);

#define META_PROP_NULL(class, name, access) do { if(FAILURE == zend_declare_property_null( \
            META_CLASS(class), name, sizeof(name)-1, ZEND_ACC_ ## access TSRMLS_CC)) { \
            return FAILURE; \
        } }while(0)

#define META_PROP_ZERO(class, name, access) do { if(FAILURE == zend_declare_property_long( \
            META_CLASS(class), name, sizeof(name)-1, 0, ZEND_ACC_ ## access TSRMLS_CC)) { \
            return FAILURE; \
        } }while(0)

#define META_PROP_L(class, name, access, value) do { if(FAILURE == zend_declare_property_long( \
            META_CLASS(class), name, sizeof(name)-1, value, ZEND_ACC_ ## access TSRMLS_CC)) { \
            return FAILURE; \
        } }while(0)

#define META_UPDATE_HPROPERTY(obj, visibility, name, value) do { char* property_name; int property_len; \
		zend_mangle_property_name(&property_name, &property_len, visibility, sizeof(visibility)-1, STRL_PAIR(name), 0); \
		zend_hash_update(obj->properties, property_name, property_len, &(value), sizeof(zval*), NULL); \
		efree(property_name); \
	} while(0)

#if 0
#define DBG(fmt, args...) php_printf("\t\t"); php_printf(fmt, ## args); php_printf("\n")
#else
#define DBG(fmt, args...)
#endif

#endif

#endif // PARSER_H

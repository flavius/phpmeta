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
zend_class_entry *php_meta_asttree_ce;
#define PHP_META_ASTTREE_CE_NAME "ASTTree"
static const function_entry php_meta_asttree_functions[];

zend_class_entry *php_meta_astnode_ce;
#define PHP_META_ASTNODE_CE_NAME "ASTNode"
static const function_entry php_meta_astnode_functions[];


//internal macros, make coding more enjoyable

#define META_PROP_NULL(class, name) do { if(FAILURE == zend_declare_property_null( \
            META_CLASS(class), name, sizeof(name)-1, ZEND_ACC_PROTECTED TSRMLS_CC)) { \
            return FAILURE; \
        } }while(0)

#define IF_NO_META_PROP_NULL(class, name) if(FAILURE == zend_declare_property_null( \
            META_CLASS(class), name, sizeof(name)-1, ZEND_ACC_PROTECTED TSRMLS_CC))

#define META_PROP_ZERO(class, name) do { if(FAILURE == zend_declare_property_long( \
            META_CLASS(class), name, sizeof(name)-1, 0, ZEND_ACC_PROTECTED TSRMLS_CC)) { \
            return FAILURE; \
        } }while(0)

#define META_PROP_L(class, name, value) do { if(FAILURE == zend_declare_property_long( \
            META_CLASS(class), name, sizeof(name)-1, value, ZEND_ACC_PROTECTED TSRMLS_CC)) { \
            return FAILURE; \
        } }while(0)

#endif // PARSER_H

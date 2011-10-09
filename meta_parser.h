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

#ifndef META_PARSER_H
#define META_PARSER_H

/*NEVER include the defs directly, always do it by including THIS file */
#include "meta_parser_defs.h"
#include "meta_scanner.h"

/*the parser interface*/
void *MetaParserAlloc(void *(*mallocProc)(size_t));
void MetaParserFree(void *p, void (*freeProc)(void*) );
void MetaParser( void *yyp,int yymajor,TOKEN* minor,zval *tree);

/*logistically, this belongs to the scanner*/
const char* meta_token_repr(int n);
/*------------------------------- convenience macros*/
/*TODO: they may be incompatible with VC9 (esp. the __VA__ARGS__ part), fix it when porting to windows*/
#define PRIV(a) _priv_##a
#define META_CALL_METHOD_EX(class, obj, method, retval, ...) do { \
    zend_function* PRIV(method); \
    if(FAILURE == zend_hash_find(&(class)->function_table, STRL_PAIR( #method ), (void**) &PRIV(method))) { \
        DBG("failed finding function %s::%s in '%s' line %d", #class, #method, __FILE__, __LINE__); \
    } \
    retval = obj_call_method_internal_ex(obj, class, PRIV(method), EG(scope), 1 TSRMLS_CC, ##__VA_ARGS__); \
} while(0)

/*use this only for methods which always return NULL or for which you want to discard the retval*/
#define META_CALL_CLASS_METHOD(class, obj, method, ...) do { \
    zval *_retv; \
    META_CALL_METHOD_EX(META_CLASS(class), obj, method, _retv, ##__VA_ARGS__); \
    if(NULL != _retv) { zval_ptr_dtor(&_retv); } \
} while(0)

#define META_CALL_METHOD(obj, method, ...) do { \
    zval *_retv; \
    zend_class_entry *ce = Z_OBJCE_P(obj); \
    META_CALL_METHOD_EX(ce, obj, method, _retv, ##__VA_ARGS__); \
    if(NULL != _retv) { zval_ptr_dtor(&_retv); } \
} while(0)

#define META_NODE_CTOR(class, nt, ...) do { \
    ALLOC_INIT_ZVAL(nt); \
    nt = obj_call_method_internal_ex(nt, META_CLASS(class), META_CLASS(class)->constructor, EG(scope), 1 TSRMLS_CC, ##__VA_ARGS__); \
} while(0)

#define META_PROP(class, obj, prop) zend_read_property(META_CLASS(class), obj, STRL_PAIR(prop)-1, 0 TSRMLS_CC)
/*------------------------------- end convenience macros*/

#endif /* META_PARSER_H*/

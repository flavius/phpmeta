#ifndef META_PARSER_H
#define META_PARSER_H

//NEVER include the defs directly, always do it by including THIS file
#include "meta_parser_defs.h"
#include "meta_scanner.h"

//the parser interface
void *MetaParserAlloc(void *(*mallocProc)(size_t));
void MetaParserFree(void *p, void (*freeProc)(void*) );
void MetaParser( void *yyp,int yymajor,TOKEN* minor,zval *tree);

//logistically, this belongs to the scanner
const char* meta_token_repr(int n);
//------------------------------- convenience macros
//TODO: they may be incompatible with VC9 (esp. the __VA__ARGS__ part), fix it when porting to windows
#define PRIV(a) _priv_##a
#define META_CALL_METHOD_EX(class, obj, method, retval, ...) do { \
    zend_function* PRIV(method); \
    if(FAILURE == zend_hash_find(&META_CLASS(class)->function_table, STRL_PAIR( #method ), (void**) &PRIV(method))) { \
        DBG("failed finding function %s::%s in '%s' line %d", #class, #method, __FILE__, __LINE__); \
    } \
    retval = obj_call_method_internal_ex(obj, META_CLASS(class), PRIV(method), EG(scope), 1 TSRMLS_CC, ##__VA_ARGS__); \
} while(0)

//use this only for methods which always return NULL or for which you want to discard the retval
#define META_CALL_METHOD(class, obj, method, ...) do { \
    zval *_retv; \
    META_CALL_METHOD_EX(class, obj, method, _retv, ##__VA_ARGS__); \
    if(NULL != _retv) { zval_ptr_dtor(&_retv); } \
} while(0)

#define META_NODE_CTOR(class, nt, ...) do { \
    ALLOC_INIT_ZVAL(nt); \
    nt = obj_call_method_internal_ex(nt, META_CLASS(class), META_CLASS(class)->constructor, EG(scope), 1 TSRMLS_CC, ##__VA_ARGS__); \
} while(0)

#define META_PROP(class, obj, prop) zend_read_property(META_CLASS(class), obj, STRL_PAIR(prop)-1, 0 TSRMLS_CC)
//------------------------------- end convenience macros

#endif // META_PARSER_H

#include <zend.h>
#include <zend_API.h>
#include <php.h>
#include "ext/standard/php_var.h"
#include "php_meta.h"
#include "scanner.h"
#include "meta_scanner.h"
#include "scanner_API.h"
#include "meta_parser.h"
#include "parser_API.h"
#include "parser.h"
#include <stdarg.h>

//#define DEBUG

//new feature testing, TODO: remove before 0.0.1
PHP_FUNCTION(meta_test) {
    zval *src;
    meta_scanner* scanner;
    TOKEN *token;
    void *parser;
    zval* tree=NULL;
    long long major;

    /** -------------- instantiate object and call method ------
    //---- instantiate the object
    zval *tree3;
    ALLOC_INIT_ZVAL(tree3);
    tree3 = obj_call_method_internal_ex(tree3, php_meta_asttree_ce, php_meta_asttree_ce->constructor, EG(scope), 1, 1 TSRMLS_CC, NULL);
    //---- find a function of a class
    zend_function *appendChild;
    zend_hash_find(&php_meta_asttree_ce->function_table, "appendchild", sizeof("appendchild"), (void**) &appendChild);
    //---- call the function
    zval *child;
    ALLOC_INIT_ZVAL(child);
    obj_call_method_internal_ex(tree3, php_meta_asttree_ce, appendChild, EG(scope), 0, 1 TSRMLS_CC, "z", child);
    RETVAL_ZVAL(tree3, 0, 1);
    return;

    ** ----------------------- */

    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &src)) {
        WRONG_PARAM_COUNT;
    }
    scanner = meta_scanner_alloc(src, SFLAGS_MOST);
    parser = MetaParserAlloc(meta_alloc);
    ALLOC_INIT_ZVAL(tree);
    tree = obj_call_method_internal_ex(tree, META_CLASS(tree), META_CLASS(tree)->constructor, EG(scope), 1, 1 TSRMLS_CC, NULL);

    do {
        token = meta_scan(scanner TSRMLS_CC);
        major = TOKEN_MAJOR(token);
        php_printf("MAJOR: %lld\n", major);
        MetaParser(parser, major, token, tree);
        if(major < 0) {
            //TODO error reporting
            break;
        }
        if(0 == major) {
            efree(token);
            break;
        }
        else {
            META_ZDUMP(TOKEN_MINOR(token));
        }
    } while(major > 0);
    MetaParserFree(parser, meta_free);
    meta_scanner_free(&scanner);
    META_ZDUMP(tree);
    RETVAL_ZVAL(tree, 0, 1);
}

static function_entry php_meta_functions[] = {
    PHP_FE(meta_test, NULL)
    PHP_FE(meta_scanner_init, NULL)
    PHP_FE(meta_scanner_get, NULL)
    PHP_FE(meta_scanner_token_name, NULL)
    ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};

//TODO detect if the token ext is activated, if no, activate backwards compatibility
//TODO expose both the scanner and the parser to the runtime

PHP_MINIT_FUNCTION(meta) {
    int status = SUCCESS;
    status = meta_parser_init_function(INIT_FUNC_ARGS_PASSTHRU);
    if(FAILURE == status) {
        return FAILURE;
    }
    //TODO call scanner & parser's own initialisation from scanner.c and parser.c respectively
    meta_scanner_descriptor = zend_register_list_destructors_ex(
            php_meta_scanner_dtor, NULL,
            PHP_META_SCANNER_DESCRIPTOR_RES_NAME, module_number);
    zend_register_long_constant("META_SFLAG_SHORT_OPEN_TAG", sizeof("META_SFLAG_SHORT_OPEN_TAG"), SFLAG_SHORT_OPEN_TAG, CONST_CS|CONST_PERSISTENT, module_number TSRMLS_CC);
    return SUCCESS;
}

zend_module_entry meta_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_META_EXTNAME,
    php_meta_functions,
    PHP_MINIT(meta),
    NULL,
    NULL,
    NULL,
    NULL,
    PHP_META_EXTVER,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_META
ZEND_GET_MODULE(meta)
#endif


//"internal" functions

//TODO move these into the parser
void *meta_alloc(size_t size) {
    return emalloc(size);
}

void meta_free(void* ptr) {
    efree(ptr);
}

//TODO use COPY_PZVAL_TO_ZVAL(*return_value, ret) in the caller


//TODO unify return_object and native_null into one single param
zval* obj_call_method_internal_ex(zval *obj, zend_class_entry *ce, zend_function *func, zend_class_entry* calling_scope,
       zend_bool return_object, zend_bool native_null TSRMLS_DC, char* fmt, ...) {

    zval **params;
    zend_fcall_info fci;
    int argc;

    //TODO strlen() is called in get_params_ex() too, fix it
    if(NULL != fmt) {
        argc = strlen(fmt);
        va_list argv;
        va_list temp_argv;

        va_start(argv, fmt);
        //--- START TEST
        //int l = va_arg(argv, int);
        //php_printf("---- L is: %d\n", l);
        //--- END TEST
        //TODO: do we need this? ZE2 doesn't use it
        va_copy(temp_argv, argv);
        //long l = va_arg(argv, long);
        //TODO check against arginfo?
        params = get_params_ex(fmt, &temp_argv);
        va_end(temp_argv);
        va_end(argv);
    }
    else {
        argc = 0;
        params = NULL;
    }

    zval *retval_ptr = NULL;

    fci.size = sizeof(fci);
    fci.function_table = EG(function_table);
    fci.function_name = NULL;
    fci.symbol_table = NULL;
    fci.object_ptr = obj;
    fci.retval_ptr_ptr = &retval_ptr;
    fci.param_count = argc;
    fci.params = &params;
    fci.no_separation = 1;

    zend_fcall_info_cache fcc;
    fcc.initialized = 1;
    fcc.function_handler = func;
    fcc.object_ptr = obj;
    fcc.calling_scope = calling_scope;
    fcc.called_scope = ce;

    //if we want the object, and the object is not an object yet, we init it
    if(return_object) {
        if(IS_NULL == Z_TYPE_P(obj)) {
            object_init_ex(obj, ce);
        }
        else {
            //TODO error: you've requested the object, but obj is not properly initialized
            php_printf("fix TODO on line %d in '%s'\n", __LINE__, __FILE__);
            native_null = 1;
            return_object = 0;
            goto clean_params;
        }
    }

    //TODO create a fcc/keep it around (in extension per-thread globals?)

    if(FAILURE == zend_call_function(&fci, &fcc TSRMLS_CC)) {
        //TODO proper error reporting
        php_printf("fix TODO on line %d in '%s'\n", __LINE__, __FILE__);
        if(NULL != retval_ptr) {
            zval_dtor(retval_ptr);
            efree(retval_ptr);
        }
    }
clean_params:
    if(NULL != params) {
        int i;
        for(i=0; i < argc; i++) {
            zval_ptr_dtor(&params[i]);
        }
        efree(params);
    }
    if(native_null) {
        if(NULL != retval_ptr && IS_NULL == Z_TYPE_P(retval_ptr)) {
            zval_ptr_dtor(&retval_ptr);
        }
        retval_ptr = NULL;
    }
    if(return_object) {
        retval_ptr = fci.object_ptr;
    }
    return retval_ptr;
}

zval** get_params_ex(const char *fmt, va_list *argp) {
    zval** params;
    size_t len;

    len = strlen(fmt);
    params = safe_emalloc(len, sizeof(zval*), 0);
    if(NULL == params) {
        return NULL;
    }

    char *s;
    long l,r;
    double d;
    zval *z;
    zend_bool b;

    size_t i, j;
    for(i=0; i < len; i++) {
        switch(fmt[i]) {
            case 's':
                MAKE_STD_ZVAL(params[i]);
                s = va_arg(*argp, char*);
                ZVAL_STRING(params[i], s, 1);
                break;
            case 'l':
                MAKE_STD_ZVAL(params[i]);
                l = va_arg(*argp, long);
                ZVAL_LONG(params[i], l);
                break;
            case 'd':
                MAKE_STD_ZVAL(params[i]);
                d = va_arg(*argp, double);
                ZVAL_DOUBLE(params[i], d);
                break;
            case 'b':
                MAKE_STD_ZVAL(params[i]);
                b = va_arg(*argp, int);
                ZVAL_BOOL(params[i], b);
                break;
            case 'r':
                MAKE_STD_ZVAL(params[i]);
                r = va_arg(*argp, long);
                ZVAL_RESOURCE(params[i], r);
                break;
            case 'z':
                z = va_arg(*argp, zval*);
                Z_ADDREF_P(z);
                params[i] = z;
                break;
            default:
                for(j = 0; j < i; j++) {
                    zval_ptr_dtor(&params[i]);
                }
                efree(params);
                return NULL;
        }
    }
    return params;

}

zval** get_params(const char *fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    zval** ret = get_params_ex(fmt, &argp);
    va_end(argp);
    return ret;
}

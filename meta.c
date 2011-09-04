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

PHP_FUNCTION(meta_test) {
}
/* {{{ meta functions */
static function_entry php_meta_functions[] = {
    PHP_FE(meta_test, NULL)
    PHP_FE(meta_scanner_init, NULL)
    PHP_FE(meta_scanner_get, NULL)
    PHP_FE(meta_scanner_token_name, NULL)
    ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};
/* }}} */
/* {{{ meta MINIT */
PHP_MINIT_FUNCTION(meta) {
    int status = SUCCESS;
    status = meta_parser_init_function(INIT_FUNC_ARGS_PASSTHRU);
    if(FAILURE == status) {
        return FAILURE;
    }
    /* TODO move the following scanner initialization into it's own function and call that instead */
    meta_scanner_descriptor = zend_register_list_destructors_ex(
            php_meta_scanner_dtor, NULL,
            PHP_META_SCANNER_DESCRIPTOR_RES_NAME, module_number);
    zend_register_long_constant("META_SFLAG_SHORT_OPEN_TAG", sizeof("META_SFLAG_SHORT_OPEN_TAG"), SFLAG_SHORT_OPEN_TAG, CONST_CS|CONST_PERSISTENT, module_number TSRMLS_CC);
    /* end scanner "initialization" (not yet complete) */
    return SUCCESS;
}
/* }}} */
/* {{{ meta_module_entry */
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
/* }}} */
/* {{{ zval** get_params_ex(const char *fmt, va_list *argp) */
static zval** get_params_ex(const char *fmt, va_list *argp) {
    zval** params;
    size_t len;
    size_t i, j;

    char *s;
    long l,r;
    double d;
    zval *z;
    zend_bool b;


    len = strlen(fmt);
    params = safe_emalloc(len, sizeof(zval*), 0);
    if(NULL == params) {
        return NULL;
    }

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
                /*TODO if z is NULL, turn it into a IS_NULL*/
                params[i] = z;
                break;
            default:
                /*TODO output error "wrong fmt specifier"*/
                for(j = 0; j < i; j++) {
                    zval_ptr_dtor(&params[i]);
                }
                efree(params);
                return NULL;
        }
    }
    return params;

}
/* }}} */
/* not used, TODO keep it around, maybe sometime we'll need it
static zval** get_params(const char *fmt, ...) {
    va_list argp;
    zval **ret;
    va_start(argp, fmt);
    ret = get_params_ex(fmt, &argp);
    va_end(argp);
    return ret;
} */
/* {{{ META_API zval* obj_call_method_internal_ex(zval *obj, zend_class_entry *ce, zend_function *func, zend_class_entry* calling_scope, zend_bool native_null TSRMLS_DC, char* fmt, ...)
 * call method func of the object obj of type ce from the specified scope and return the value returned by the calee.
 * if native_null is true, and the method returns a IS_NULL zval, then free that zval and return a NULL pointer
 * valid format specifiers are [sldbrz] for string, long, double, boolean, resource, zval */
META_API zval* obj_call_method_internal_ex(zval *obj, zend_class_entry *ce, zend_function *func, zend_class_entry* calling_scope,
       zend_bool native_null TSRMLS_DC, char* fmt, ...) {
    zval **params;
    int argc;
    zval *retval_ptr;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    /*TODO strlen() is called in get_params_ex() too, fix it*/
    if(NULL != fmt) {
        va_list argv;
        va_list temp_argv;

        argc = strlen(fmt);
        va_start(argv, fmt);
        va_copy(temp_argv, argv);
        params = get_params_ex(fmt, &temp_argv);
        va_end(temp_argv);
        va_end(argv);
    }
    else {
        argc = 0;
        params = NULL;
    }

    retval_ptr = NULL;

    fci.size = sizeof(fci);
    fci.function_table = EG(function_table);
    fci.function_name = NULL;
    fci.symbol_table = NULL;
    fci.object_ptr = obj;
    fci.retval_ptr_ptr = &retval_ptr;
    fci.param_count = argc;
    fci.params = &params;
    fci.no_separation = 1;

    fcc.initialized = 1;
    fcc.function_handler = func;
    fcc.object_ptr = obj;
    fcc.calling_scope = calling_scope;
    fcc.called_scope = ce;

    /*if we want the object, and the object is not an object yet, we init it*/
    if(func == ce->constructor) {
        /*TODO check obj's refcount, it has to be 1*/
        if(IS_NULL == Z_TYPE_P(obj)) {
            object_init_ex(obj, ce);
        }
        else {
            /*TODO error: you've requested the object, but obj is not properly initialized*/
            php_printf("fix TODO on line %d in '%s'\n", __LINE__, __FILE__);
            native_null = 1;
            goto clean_params;
        }
    }

    /*TODO create a fcc/keep it around (in extension per-thread globals?)*/

    if(FAILURE == zend_call_function(&fci, &fcc TSRMLS_CC)) {
        /*TODO proper error reporting*/
        php_printf("fix TODO on line %d in '%s'\n", __LINE__, __FILE__);
        if(NULL != retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
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
    if(func == ce->constructor) {
        zval_ptr_dtor(&retval_ptr);
        retval_ptr = fci.object_ptr;
    }
    if(native_null) {
        if(NULL != retval_ptr && IS_NULL == Z_TYPE_P(retval_ptr)) {
            zval_ptr_dtor(&retval_ptr);
            retval_ptr = NULL;
        }
        else {
            /*TODO internal error: the caller (also C code) expects a native NULL ptr, but the method we've called returns something else than a IS_NULL*/
        }
    }
    return retval_ptr;
}
/* }}} */
/*TODO move these into the parser*/
void *meta_alloc(size_t size) {
    return emalloc(size);
}

void meta_free(void* ptr) {
    efree(ptr);
}

#ifdef COMPILE_DL_META
ZEND_GET_MODULE(meta)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 */

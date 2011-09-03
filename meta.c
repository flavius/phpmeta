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

static function_entry php_meta_functions[] = {
    PHP_FE(meta_test, NULL)
    PHP_FE(meta_scanner_init, NULL)
    PHP_FE(meta_scanner_get, NULL)
    PHP_FE(meta_scanner_token_name, NULL)
    ZEND_RAW_FENTRY(NULL, NULL, NULL, 0)
};

PHP_MINIT_FUNCTION(meta) {
    int status = SUCCESS;
    status = meta_parser_init_function(INIT_FUNC_ARGS_PASSTHRU);
    if(FAILURE == status) {
        return FAILURE;
    }
    //TODO move the following scanner initialization into it's own function and call that instead
    meta_scanner_descriptor = zend_register_list_destructors_ex(
            php_meta_scanner_dtor, NULL,
            PHP_META_SCANNER_DESCRIPTOR_RES_NAME, module_number);
    zend_register_long_constant("META_SFLAG_SHORT_OPEN_TAG", sizeof("META_SFLAG_SHORT_OPEN_TAG"), SFLAG_SHORT_OPEN_TAG, CONST_CS|CONST_PERSISTENT, module_number TSRMLS_CC);
    //end scanner "initialization" (not yet complete)
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


//TODO move these into the parser
void *meta_alloc(size_t size) {
    return emalloc(size);
}

void meta_free(void* ptr) {
    efree(ptr);
}

zval* obj_call_method_internal_ex(zval *obj, zend_class_entry *ce, zend_function *func, zend_class_entry* calling_scope,
       zend_bool native_null TSRMLS_DC, char* fmt, ...) {

    zval **params;
    zend_fcall_info fci;
    int argc;

    //TODO strlen() is called in get_params_ex() too, fix it
    if(NULL != fmt) {
        argc = strlen(fmt);
        va_list argv;
        va_list temp_argv;

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
    if(func == ce->constructor) {
        //TODO check obj's refcount, it has to be 1
        if(IS_NULL == Z_TYPE_P(obj)) {
            object_init_ex(obj, ce);
        }
        else {
            //TODO error: you've requested the object, but obj is not properly initialized
            php_printf("fix TODO on line %d in '%s'\n", __LINE__, __FILE__);
            native_null = 1;
            goto clean_params;
        }
    }

    //TODO create a fcc/keep it around (in extension per-thread globals?)

    if(FAILURE == zend_call_function(&fci, &fcc TSRMLS_CC)) {
        //TODO proper error reporting
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
            //TODO internal error: the caller (also C code) expects a native NULL ptr, but the method we've called returns something else than a IS_NULL
        }
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
                //TODO if z is NULL, turn it into a IS_NULL
                params[i] = z;
                break;
            default:
                //TODO output error "wrong fmt specifier"
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


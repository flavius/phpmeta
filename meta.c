#include <zend.h>
#include <zend_API.h>
#include <php.h>
#include "ext/standard/php_var.h"
#include "php_meta.h"
#include "php_scanner.h"
#include "php_parser.h"

PHP_FUNCTION(meta_test) {
    zval *src;
    meta_scanner* scanner;
    int major;
    zval* minor;

    php_printf("--------------------------------------\n");
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &src)) {
        WRONG_PARAM_COUNT;
    }
    //TODO check if string
    if(!Z_STRLEN_P(src)) {
        RETURN_NULL();
    }
    scanner = meta_scanner_alloc(src);
    do {
        major = meta_scan(scanner, &minor TSRMLS_CC);
        php_printf("major: %d ('%s')\n", major, meta_token_repr(major));
        if(NULL != minor && major > 0) {
            php_printf("minor: ");
            php_debug_zval_dump(&minor, 0 TSRMLS_CC);
            zval_dtor(minor);
            efree(minor);
        }
        //TODO call parser
    } while(major > 0);

    meta_scanner_destroy(&scanner);
}

static function_entry php_meta_functions[] = {
    PHP_FE(meta_test, NULL)
    {NULL, NULL, NULL}
};

PHP_MINIT_FUNCTION(meta) {
    //TODO init meta ast node
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

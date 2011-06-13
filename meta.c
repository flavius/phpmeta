#include <zend.h>
#include <zend_API.h>
#include <php.h>
#include "ext/standard/php_var.h"
#include "php_meta.h"
#include "scanner.h"
#include "meta_scanner.h"

//#define DEBUG

//new feature testing, TODO: remove before 0.0.1
PHP_FUNCTION(meta_test) {
}

static function_entry php_meta_functions[] = {
    PHP_FE(meta_test, NULL)
    PHP_FE(meta_scanner_init, NULL)
    PHP_FE(meta_scanner_get, NULL)
    PHP_FE(meta_scanner_token_name, NULL)
    {NULL, NULL, NULL}
};

//TODO detect if the token ext is activated, if no, activate backwards compatibility
//TODO expose both the scanner and the parser to the runtime

PHP_MINIT_FUNCTION(meta) {
    //TODO init meta ast node
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


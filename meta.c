#include <zend.h>
#include <zend_API.h>
#include <php.h>
#include "ext/standard/php_var.h"
#include "php_meta.h"
#include "scanner.h"
#include "meta_scanner.h"
#include "meta_parser.h"

//#define DEBUG

//new feature testing, TODO: remove before 0.0.1
PHP_FUNCTION(meta_test) {
    zval *src;
    meta_scanner* scanner;
    TOKEN *token;
    void *parser;
    int major;

    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &src)) {
        WRONG_PARAM_COUNT;
    }
    scanner = meta_scanner_alloc(src, SFLAGS_MOST);
    parser = MetaParserAlloc(meta_alloc);
    //TODO initialize ast tree
    major = 1;
    do {
        token = meta_scan(scanner TSRMLS_CC);
        major = TOKEN_MAJOR(token);
        if(major >= 0) {
            //TODO call parser
            //token_free(&token);
            meta_token_dtor(token);
        }
        else {
            //error reporting
        }
    } while(major > 0);
    MetaParserFree(parser, meta_free);
    meta_scanner_free(&scanner);
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


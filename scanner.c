#include <zend.h>
#include "php_meta.h"
#include "scanner.h"
#include "meta_scanner.h"
#include "scanner_API.h"
#include "meta_parser.h"

//TODO implement initialisation code, used by meta.c's MINIT & co

int meta_scanner_descriptor;

PHP_FUNCTION(meta_scanner_init) {
    meta_scanner* scanner;
    zval *rawsrc;
    long flags;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl",
            &rawsrc, &flags
        )) {
            WRONG_PARAM_COUNT;
        }
    //TODO also accept streams
    scanner = meta_scanner_alloc(rawsrc, flags);
    ZEND_REGISTER_RESOURCE(return_value, scanner, meta_scanner_descriptor);
}

PHP_FUNCTION(meta_scanner_get) {
    zval *scanner_res;
    TOKEN *token;
    meta_scanner* scanner;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
        &scanner_res)) {
            WRONG_PARAM_COUNT;
    }
    ZEND_FETCH_RESOURCE(scanner, meta_scanner*, &scanner_res, -1,
        PHP_META_SCANNER_DESCRIPTOR_RES_NAME, meta_scanner_descriptor);
    if(scanner->err_no != ERR_NONE) {
        RETURN_NULL();
    }
    token = meta_scan(scanner TSRMLS_CC);
    meta_token_zval_ex(token, return_value);
    meta_token_dtor(token);

    /*
    if(TOKEN_MAJOR(token) >= 0) {
        //TODO actually return the tokens, not just dump them to stdout
        php_printf("%s (%d) on LINES %ld-%ld", meta_token_repr(TOKEN_MAJOR(token)), TOKEN_MAJOR(token), token->start_line, token->end_line);
        if(TOKEN_MINOR(token)) {
            php_printf(" : ");
            php_debug_zval_dump( &TOKEN_MINOR(token), 0 TSRMLS_CC);
        }
        if(TOKEN_MAJOR(token) == 0) {
            RETVAL_FALSE;
        }
        else {
            RETVAL_TRUE;
        }
        ast_token_dtor(token);
    }
    else {
        //RETURN_NULL();
        //TODO error reporting
    }
    */
}

PHP_FUNCTION(meta_scanner_token_name) {
    long num;
    if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l",
        &num)) {
        WRONG_PARAM_COUNT;
    }
    RETURN_STRING(meta_token_repr(num), 1);
}

void php_meta_scanner_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
    meta_scanner *scanner = (meta_scanner*)rsrc->ptr;
    meta_scanner_free(&scanner);
}

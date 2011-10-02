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
#include "php_meta.h"
#include "scanner.h"
#include "meta_scanner.h"
#include "scanner_API.h"
#include "meta_parser.h"

int meta_scanner_descriptor;
/* {{{ create and initialize scanner-related resources
 */
int meta_scanner_init_function(INIT_FUNC_ARGS) {
	/* TODO: more checking, more constants registered, etc */
	meta_scanner_descriptor = zend_register_list_destructors_ex(php_meta_scanner_dtor, NULL, PHP_META_SCANNER_DESCRIPTOR_RES_NAME, module_number);
	zend_register_long_constant("META_SFLAG_SHORT_OPEN_TAG", sizeof("META_SFLAG_SHORT_OPEN_TAG"), SFLAG_SHORT_OPEN_TAG, CONST_CS|CONST_PERSISTENT, module_number TSRMLS_CC);
	return SUCCESS;
}
/* }}} */
/* {{{ proto scanner_handle meta_scanner_init(string $src, int $flags)
 * Create a new scanner which will scan the provided input */
PHP_FUNCTION(meta_scanner_init) {
	meta_scanner* scanner;
	zval *rawsrc;
	long flags;
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zl", &rawsrc, &flags)) {
		WRONG_PARAM_COUNT;
	}
	/* TODO also accept streams */
	scanner = meta_scanner_alloc(rawsrc, flags);
	ZEND_REGISTER_RESOURCE(return_value, scanner, meta_scanner_descriptor);
}
/* }}} */
/* {{{ proto mixed meta_scanner_get(scanner_handle $handle)
 * get the canonical representation of a token, as an array, with some information attached to it */
PHP_FUNCTION(meta_scanner_get) {
	zval *scanner_res;
	TOKEN *token;
	meta_scanner* scanner;
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &scanner_res)) {
		WRONG_PARAM_COUNT;
	}
	ZEND_FETCH_RESOURCE(scanner, meta_scanner*, &scanner_res, -1, PHP_META_SCANNER_DESCRIPTOR_RES_NAME, meta_scanner_descriptor);
	if(scanner->err_no != ERR_NONE) {
		RETURN_NULL();
	}
	token = meta_scan(scanner TSRMLS_CC);
	meta_token_zval_ex(token, return_value);
	meta_token_dtor(&token, META_TOK_CHAIN_FREESELF, NULL, NULL);
}
/* }}} */
/* {{{ proto void meta_scanner_reset(scanner_handle $handle)
 * reset the state of the scanner, prepare for starting over */
PHP_FUNCTION(meta_scanner_reset) {
	/* TODO implement me */
}
/* }}} */
/* {{{ proto string meta_scanner_token_name(int $type)
 * Given a type (a major number), get the string representation thereof. */
PHP_FUNCTION(meta_scanner_token_name) {
	long num;
	if(FAILURE == zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &num)) {
		WRONG_PARAM_COUNT;
	}
	RETURN_STRING(meta_token_repr(num), 1);
}
/* }}} */
/* {{{ META_API void php_meta_scanner_dtor(zend_rsrc_list_entry* TSRMLS_DC)
 * destroy all the internal data attached to the scanner */
META_API void php_meta_scanner_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) {
	meta_scanner *scanner = (meta_scanner*)rsrc->ptr;
	meta_scanner_free(&scanner);
}
/* }}} */
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 */

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
#include "scanner_API.h"

/* {{{ META_API zval* meta_token_zval(TOKEN*)
 * Get the zval representation of a token */
META_API zval* meta_token_zval(TOKEN *token) {
    zval* tok_repr;
    MAKE_STD_ZVAL(tok_repr);
    array_init_size(tok_repr, 8);/* 8 instead of 5, so zend_hash_init doesn't need to round up */
    add_assoc_long(tok_repr, "major", token->major);
    add_assoc_bool(tok_repr, "dirty", token->dirty);
    add_assoc_long(tok_repr, "start_line", token->start_line);
    add_assoc_long(tok_repr, "end_line", token->end_line);
    if(NULL != TOKEN_MINOR(token)) {
        zval_add_ref(&token->minor);
        add_assoc_zval(tok_repr, "minor", token->minor);
    }
    else {
        add_assoc_null(tok_repr, "minor");
    }
    return tok_repr;
}
/* }}} */
/* {{{ META_API void meta_token_zval_ex(TOKEN*, zval*)
 * create an array representation of token into the preallocated tok_repr */
META_API void meta_token_zval_ex(TOKEN *token, zval *tok_repr) {
    array_init_size(tok_repr, 8);/* 8 inwhich allow this to happenstead of 5, so zend_hash_init doesn't need to round up */
    add_assoc_long(tok_repr, "major", token->major);
    add_assoc_bool(tok_repr, "dirty", token->dirty);
    add_assoc_long(tok_repr, "start_line", token->start_line);
    add_assoc_long(tok_repr, "end_line", token->end_line);
    if(NULL != TOKEN_MINOR(token)) {
        zval_add_ref(&token->minor);
        add_assoc_zval(tok_repr, "minor", token->minor);
    }
    else {
        add_assoc_null(tok_repr, "minor");
    }
}
/* }}} */
/* {{{ META_API zval* meta_scanner_token_zval(TOKEN* t)
 * TODO: remove this, and reuse code among meta_token_zval and meta_token_zval_ex */
META_API zval* meta_scanner_token_zval(TOKEN* t) {
    zval* tzv;
    MAKE_STD_ZVAL(tzv);
    array_init(tzv);
    add_assoc_long(tzv, "major", TOKEN_MAJOR(t));
    return tzv;
}
/* }}} */
/* {{{ META_API void meta_scanner_free(meta_scanner **)
 * Free everything this scanner holds. Held zvals are just rc--'ed */
META_API void meta_scanner_free(meta_scanner **scanner) {
    int elems;
    TOKEN* token;
    zval_ptr_dtor(&((*scanner)->rawsrc));
    /* TODO inspect (*scanner)->buffer->max for real inputs - how big does the stack grow? */
    elems = zend_ptr_stack_num_elements((*scanner)->buffer);
    while(elems--) {
        token = zend_ptr_stack_pop((*scanner)->buffer);
        meta_token_dtor(&token, 1);
    }
    zend_ptr_stack_destroy((*scanner)->buffer);
    efree((*scanner)->buffer);
    efree(*scanner);
}
/* }}} */
/* {{{ META_API void meta_token_dtor(TOKEN** t, zend_bool deep)
 * During the parsing loop, the scanner may spit out tokens which are not covered by the grammar. These
 * are usually comments, whitespaces, etc - putting them in the grammar would make it unnecessarily complex.
 * By linking tokens together, we are able to slice out parts of this ring as reduction happen, and stich
 * them together to their appropiate parents, usually via *::appendChild().
 *
 * This function allows you to free such a slice out of the ring.
 *
 * If deep is true, destroy minors on the way there too.
 */
/* TODO: 1. introduce new parameters for direction (left or right) and pointer up to which to match, usually NULL
 * 2. introduce new parameter to signal whether TOKEN_IS_FREEABLE should be taken into consideration or not
 * 3. introduce a new parameter to signal whether we should stop as soon as we see a non-freeable token in the chain, for the given direction
 * that's three flags: DEEP, IF_FREEABLE, FULL_STOP, TODO put them all in a bitmask */
META_API void meta_token_dtor(TOKEN** t, zend_bool deep) {
    TOKEN *cursor, *prev;
    cursor = (*t)->prev;
    while(NULL != cursor && TOKEN_IS_FREEABLE(cursor)) {
        if(deep) {
            zval_ptr_dtor(&TOKEN_MINOR(cursor));
        }
        prev = cursor;
        cursor = cursor->prev;
        efree(prev);
    }
    cursor = *t;
	/* currently, we are not doing what we've docummented. We are free'ing in both directions. TODO: This will be fixed */
    while(NULL != cursor && TOKEN_IS_FREEABLE(cursor)) {
        if(deep) {
            zval_ptr_dtor(&TOKEN_MINOR(cursor));
        }
        prev = cursor;
        cursor = cursor->next;
        efree(prev);
    }
    *t=NULL;
}
/* }}} */
/* {{{ META_API meta_scanner* meta_scanner_alloc(zval *rawsrc, long flags)
 * Allocate and initialize new scanner */
META_API meta_scanner* meta_scanner_alloc(zval* rawsrc, long flags) {
    meta_scanner *scanner;
    Z_STRVAL_P(rawsrc) = erealloc(Z_STRVAL_P(rawsrc), Z_STRLEN_P(rawsrc)+meta_scanner_maxfill);
    memset(Z_STRVAL_P(rawsrc)+Z_STRLEN_P(rawsrc), 0, meta_scanner_maxfill);

    scanner = emalloc(sizeof(meta_scanner));
    scanner->limit = Z_STRVAL_P(rawsrc) + Z_STRLEN_P(rawsrc) + meta_scanner_maxfill - 1;
    scanner->src = Z_STRVAL_P(rawsrc);
    scanner->src_len = Z_STRLEN_P(rawsrc);
    zval_add_ref(&rawsrc);
    scanner->cursor = scanner->ctxmarker = scanner->marker = scanner->src;

    scanner->state = STATE(ST_INITIAL);
    scanner->position = 0;
    scanner->line_no = 1;
    scanner->rawsrc = rawsrc;
    scanner->flags = flags;
    scanner->err_no = ERR_NONE;

    scanner->buffer = emalloc(sizeof(zend_ptr_stack));
    zend_ptr_stack_init(scanner->buffer);

    return scanner;
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

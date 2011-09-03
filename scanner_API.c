#include <zend.h>
#include "scanner_API.h"

META_API zval* meta_token_zval(TOKEN *token) {
    zval* tok_repr;
    MAKE_STD_ZVAL(tok_repr);
    array_init_size(tok_repr, 8);//8 instead of 5, so zend_hash_init doesn't need to round up
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

//create an array representation of token into the preallocated tok_repr
META_API void meta_token_zval_ex(TOKEN *token, zval *tok_repr) {
    array_init_size(tok_repr, 8);//8 instead of 5, so zend_hash_init doesn't need to round up
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
//TODO integrate with the previous function
META_API zval* meta_scanner_token_zval(TOKEN* t) {
    zval* tzv;
    MAKE_STD_ZVAL(tzv);
    array_init(tzv);
    add_assoc_long(tzv, "major", TOKEN_MAJOR(t));
    return tzv;
}

META_API void meta_scanner_free(meta_scanner **scanner) {
    zval_ptr_dtor(&((*scanner)->rawsrc));
    int elems;
    TOKEN* token;
    //TODO inspect (*scanner)->buffer->max for real inputs - how big does the stack grow?
    elems = zend_ptr_stack_num_elements((*scanner)->buffer);
    while(elems--) {
        token = zend_ptr_stack_pop((*scanner)->buffer);
        meta_token_dtor(&token, 1);
        //efree(token);
    }
    zend_ptr_stack_destroy((*scanner)->buffer);
    //zend_llist_destroy((*scanner)->buffer);
    efree((*scanner)->buffer);
    efree(*scanner);
}

//destroy all the tokens in the chain, and the token itself
//if deep is true, destroy the minors as well
//the entire process frees only an interval in the chain which is marked as freeable via TOKEN_IS_FREEABLE
//as soon as it sees a non-freeable node, it marks it as the limit of the chain to be freed
//TODO turn deep into a bitmask for things to do on the left, on the right, and on the starting node *t itself
//three flags are possible: DEEP, IF_FREEABLE, FULL_STOP (stop as soon as you see a non-freeable element)
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

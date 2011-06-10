#ifndef META_SCANNER_H
#define META_SCANNER_H

#include <zend.h>
#include "php_meta.h"
#include "php_scanner_defs.h"

typedef struct _token {
    int major;
    zval *minor;
    zend_bool dirty;
    long start_line;
    long end_line;
} TOKEN;

#define TOKEN_MAJOR(t) (((TOKEN*)t)->major)
#define TOKEN_MINOR(t) (((TOKEN*)t)->minor)

//useful for zendll
void ast_token_dtor(TOKEN *token);
void token_free(TOKEN **t);

#define SFLAG_SHORT_OPEN_TAG    0x1
#define SFLAG_ASP_TAGS          0x1<<1
#define SFLAG_CHECK_OVERFLOWS   0x1<<2
//ignore spacing, newlines, tabs, while in scripting
#define SFLAG_IGNORE_WHITESPACE 0x1<<3
//do not construct minor values for "language keywords" like "<?php" or "const"
#define SFLAG_SIMPLE_KEYWORDS 0x1<<4
//ignore "/*...*/" comments
#define SFLAG_IGNORE_C_COMMENTS 0x1<<5
//ignore "//" comments
#define SFLAG_IGNORE_CPP_COMMENTS 0x1<<6
//ignore "#" comments
#define SFLAG_IGNORE_SHELL_COMMENTS 0x1<<7
//ignore comments altogether
#define SFLAG_IGNORE_COMMENTS SFLAG_IGNORE_C_COMMENTS | SFLAG_IGNORE_CPP_COMMENTS | SFLAG_IGNORE_SHELL_COMMENTS
//lines starting with "#@" are considered hook-points (called "decorators" from the POV of the parser)
//this flag tells to generate tokens for them, regardless of the IGNORE_SHELL_COMMENTS flag
#define SFLAG_DO_HOOK 0x1<<8

#define SFLAGS_MOST SFLAG_SHORT_OPEN_TAG | SFLAG_ASP_TAGS
#define SFLAGS_STRICT SFLAG_CHECK_OVERFLOWS

#define HAS_FLAG(scanner, flag) (scanner->flags & SFLAG_ ##flag)

typedef struct _meta_scanner {
    zval* rawsrc;
    char *src;
    size_t src_len;
    char* marker;
    char* ctxmarker;
    char* cursor;
    char* limit;
    int state;
    //zend_bool free_raw;
    //if stream-based source, the position of cursor
    int position;
    //the line number
    unsigned int line_no;
    zend_ptr_stack *buffer;
    //zend_llist *buffer;
    //sometimes the scanner looks too far ahead and
    //when it does so, it caches the previous tokens
    //TODO add streams
    unsigned int flags;//see SFLAG_ above
    unsigned int err_no;
} meta_scanner;

meta_scanner* meta_scanner_alloc(zval*, long flags);
void meta_scanner_free(meta_scanner **scanner);
TOKEN* meta_scan(meta_scanner* scanner TSRMLS_DC);

META_API zval* meta_scanner_token_zval(TOKEN* t);
META_API void meta_token_zval_ex(TOKEN *token, zval *tok_repr);
//return codes for meta_scan
#define ERR_NONE 0
#define ERR_EOI 1
#define ERR_FILLOVERFLOW 2


//----------------------------- extension-wide symbols ------------------------------
#define PHP_META_SCANNER_DESCRIPTOR_RES_NAME "Meta Scanner"
extern int meta_scanner_descriptor;
PHP_FUNCTION(meta_scanner_init);
PHP_FUNCTION(meta_scanner_get);
PHP_FUNCTION(meta_scanner_token_name);
void php_meta_scanner_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
META_API zval* meta_token_zval(TOKEN *token);

#endif
/*
 * the big TODO list
 * token types and their attributes:
 * T_OUTSIDE_SCRIPTING
 *  - start_line
 *  - end_line
 * T_OPEN_TAG
 * T_WHITESPACE
 *  - start_line
 *  - end_line (if we have flag merge_whitespace_newline)
 * T_NEWLINE
 *  - start_line
 *  - end_line (if we have flag merge_newlines)
 * T_LNUMBER
 *  - overflow (bool, if we have flag check_overflows)
 */

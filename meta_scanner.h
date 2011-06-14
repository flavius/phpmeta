#ifndef META_SCANNER_H
#define META_SCANNER_H

#include <zend.h>
#include "php_meta.h"
//NEVER include the defs directly, always do it by including THIS file
#include "meta_scanner_defs.h"

//TODO streamline the scanner internal API

typedef struct _token {
    int major;
    zval *minor;
    zend_bool dirty;
    long start_line;
    long end_line;
} TOKEN;

#define TOKEN_MAJOR(t) ((t)->major)
#define TOKEN_MINOR(t) ((t)->minor)

//useful for zendll
void meta_token_dtor(TOKEN *token);
//TODO rename
//void token_free(TOKEN **t);
//TODO used nowhere!
void meta_token_free(TOKEN **t);

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

//the most feature-rich scanning result
#define SFLAGS_MOST SFLAG_SHORT_OPEN_TAG | SFLAG_ASP_TAGS | SFLAG_DO_HOOK
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
    //sometimes the scanner looks too far ahead and
    //when it does so, it caches the previous tokens
    zend_ptr_stack *buffer;
    unsigned int flags;//see SFLAG_ above
    unsigned int err_no;
    //TODO add streams
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
void php_meta_scanner_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);
META_API zval* meta_token_zval(TOKEN *token);

#endif // META_SCANNER_H

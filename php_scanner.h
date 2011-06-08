#ifndef META_SCANNER_H
#define META_SCANNER_H

#include <zend.h>
#include "php_scanner_defs.h"

typedef struct _token {
    int major;
    zval *minor;
    zend_bool dirty;
    unsigned int start_line;
    unsigned int end_line;
} TOKEN;

#define TOKEN_MAJOR(t) (((TOKEN*)t)->major)
#define TOKEN_MINOR(t) (((TOKEN*)t)->minor)

//useful for zendll
void ast_token_dtor(void *token);
void token_free(TOKEN **t);

#define SFLAG_SHORT_OPEN_TAG    0x1
#define SFLAG_ASP_TAGS          0x1<<1
#define SFLAG_CHECK_OVERFLOWS   0x1<<2
#define SFLAG_MERGE_WHITESPACE  0x1<<3
#define SFLAG_MERGE_NEWLINES    0x1<<4
#define SFLAG_IGNORE_WHITESPACE 0x1<<5

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
    zend_llist *buffer;
    //sometimes the scanner looks too far ahead and
    //when it does so, it caches the previous tokens
    //TODO add streams
    //TODO add TSRM
    /* flags:
     * short_open_tag
     * asp_tags
     * check_overflows (check numeric overflows)
     * merge_newlines (EOL + EOL + ...) become one single EOL
     * merge_whitespace (SPACE + EOL + SPACE + SPACE + EOL + ...) become one single T_WHITESPACE
     * ignore_whitespaces (ignore SPACEs and EOLs)
     * TODO: ignore comments
     */
    unsigned int flags;//see SFLAG_ above
    unsigned int err_no;
} meta_scanner;

meta_scanner* meta_scanner_alloc(zval*, long flags);
void meta_scanner_free(meta_scanner **scanner);
TOKEN* meta_scan(meta_scanner* scanner TSRMLS_DC);

zval* meta_scanner_token_zval(TOKEN* t);
//return codes for meta_scan
#define ERR_NONE 0
#define ERR_UNITIALIZED 1
#define ERR_FILLOVERFLOW 2


//----------------------------- extension-wide symbols ------------------------------
#define PHP_META_SCANNER_DESCRIPTOR_RES_NAME "Meta Scanner"
extern int meta_scanner_descriptor;
PHP_FUNCTION(meta_scanner_init);
PHP_FUNCTION(meta_scanner_get);
void php_meta_scanner_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

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

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

#ifndef META_SCANNER_H
#define META_SCANNER_H

#include <zend.h>
#include "php_meta.h"
//NEVER include the defs directly, always do it by including THIS file
#include "meta_scanner_defs.h"

//CTOR: ast_token_ctor in meta_scanner.re
typedef struct _token {
    int major;
    zval *minor;
    //set by the scanner if it detects integer overflows or things like that
    zend_bool dirty;
    //true for white spacing, comments, etc, any NT not handled by the grammar rules, which needs to be done manually
    zend_bool is_dispensable;
    //true if it shouldn't be freed by ast_token_dtor (in the chain)
    zend_bool free_me;
    long start_line;
    long end_line;
    //the parser shifts off some tokens not needed by the grammar (for instance whitespaces)
    //this is in place so we can chain tokens circularily
    //look at the scanner -> parser loop to see how this is used
    struct _token *prev;
    struct _token *next;
} TOKEN;

#define TOKEN_MAJOR(t) ((t)->major)
#define TOKEN_MINOR(t) ((t)->minor)
#define TOKEN_IS_DIRTY(t) ((t)->dirty)
//TODO rename this to TOKEN_IS_SUGAR :-)
#define TOKEN_IS_DISPENSABLE(t) ((t)->is_dispensable)
#define TOKEN_IS_FREEABLE(t) ((t)->free_me)

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
    long line_no;
    //sometimes the scanner looks too far ahead and
    //when it does so, it caches the previous tokens
    zend_ptr_stack *buffer;
    unsigned int flags;//see SFLAG_ below
    unsigned int err_no;
    //TODO add streams
} meta_scanner;

//-------- scanner flags --------------
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
//some minors have redundant data attached to them, for instance "<?php" for TAG_OPEN and the like
//activating this flag tells the scanner to not generate the minor value for such terminals
#define SFLAG_SKIP_REDUNDANT

//shortcut flags
//the most feature-rich scanning result
#define SFLAGS_MOST SFLAG_SHORT_OPEN_TAG | SFLAG_ASP_TAGS | SFLAG_DO_HOOK
#define SFLAGS_STRICT SFLAG_CHECK_OVERFLOWS

#define HAS_FLAG(scanner, flag) (scanner->flags & SFLAG_ ##flag)


//-------- the lexer --------------
META_API TOKEN* meta_scan(meta_scanner* scanner TSRMLS_DC);

//status codes for meta_scan, meta_scanner.err_no
#define ERR_NONE 0
#define ERR_EOI 1
#define ERR_FILLOVERFLOW 2

#define YYCTYPE char
#define STATE(name) yyc##name
#define ST_NAME(name) STATE(ST_ ## name)

#endif // META_SCANNER_H

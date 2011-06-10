#include <zend.h>
#include <zend_API.h>
#include <php.h>
#include <php_scanner.h>
#include <php_parser.h>
#include <errno.h>
#include "php_meta.h"
//TODO remove
#include <stdio.h>
#include "ext/standard/php_var.h"

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
    ast_token_dtor(token);

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

// ---------------------------- scanner internals --------------------------------

#define YYCTYPE char
#define STATE(name) yyc##name
#define ST_NAME(name) STATE(ST_ ## name)

#define IS_EOL(c) *(c) == '\n' || (*(c) == '\r' && *((c)+1) != '\n')

#ifdef DEBUG
# if 0
#  define DBG_SCANNER(state, c) php_printf("\t\t\tlex state %d, cursor '%c'(%d)\n", state, c, c)
# else
#  define DBG_SCANNER(state, c)
# endif
#define DBG(fmt, args...) php_printf("\t\t(pos %d)\t", YYCURSOR - scanner->src); php_printf(fmt, ## args); php_printf("\n")
#else
#define DBG_SCANNER(state, c)
#define PRINT_DBG(fmt, args...)
#endif

/*!max:re2c */

META_API meta_scanner* meta_scanner_alloc(zval* rawsrc, long flags) {
    meta_scanner *scanner;
    Z_STRVAL_P(rawsrc) = erealloc(Z_STRVAL_P(rawsrc), Z_STRLEN_P(rawsrc)+YYMAXFILL);
    memset(Z_STRVAL_P(rawsrc)+Z_STRLEN_P(rawsrc), 0, YYMAXFILL);

    scanner = emalloc(sizeof(meta_scanner));
    scanner->limit = Z_STRVAL_P(rawsrc) + Z_STRLEN_P(rawsrc) + YYMAXFILL - 1;
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
    //zend_llist_init(scanner->buffer, sizeof(TOKEN*), ast_token_dtor, 0);

    return scanner;
}


META_API void meta_scanner_free(meta_scanner **scanner) {
    zval_ptr_dtor(&((*scanner)->rawsrc));
    int elems;
    TOKEN* token;
    //TODO inspect (*scanner)->buffer->max for real inputs - how big does the stack grow?
    elems = zend_ptr_stack_num_elements((*scanner)->buffer);
    while(elems--) {
        token = zend_ptr_stack_pop((*scanner)->buffer);
        ast_token_dtor(token);
        //efree(token);
    }
    zend_ptr_stack_destroy((*scanner)->buffer);
    //zend_llist_destroy((*scanner)->buffer);
    efree((*scanner)->buffer);
    efree(*scanner);
}

META_API void ast_token_dtor(TOKEN *tok) {
    if(NULL != TOKEN_MINOR(tok)) {
        //Z_DELREF_P(tok->minor);
        zval_ptr_dtor(&((tok)->minor));
        //zval_dtor(tok->minor);
        //efree(tok->minor);
    }
    efree(tok);
}

TOKEN* ast_token_ctor(meta_scanner* scanner, int major, char* start, int len) {
    TOKEN* t;
    int errcode=0;
    long number;

    t = emalloc(sizeof(TOKEN));
    TOKEN_MAJOR(t) = major;
    TOKEN_MINOR(t) = NULL;
    t->dirty = 0;
    t->start_line = 0;
    t->end_line = 0;
    switch(major) {
        case 0:
        case T_PLUS:
            break;
        case T_OUTSIDE_SCRIPTING:
        case T_OPEN_TAG:
        case T_OPEN_TAG_WITH_ECHO:
        case T_WHITESPACE:
        case T_CLOSE_TAG:
            MAKE_STD_ZVAL(TOKEN_MINOR(t));
            ZVAL_STRINGL(TOKEN_MINOR(t), start, len, 1);
            break;
        case T_LNUMBER:
            MAKE_STD_ZVAL(TOKEN_MINOR(t));
            if(HAS_FLAG(scanner, CHECK_OVERFLOWS)) {
                errno = 0;
                number = strtol(start, NULL, 0);
                errcode = errno;
                ZVAL_LONG(TOKEN_MINOR(t), number);
                if(ERANGE == errcode) {
                    t->dirty = 1;
                    //TODO remove me
                    perror(NULL);
                    //TODO do we want the original input as string, or INT_MAX as now?
                }
            }
            else {
                ZVAL_LONG(TOKEN_MINOR(t), strtol(start, NULL, 0));
            }
            break;
        default:
            MAKE_STD_ZVAL(TOKEN_MINOR(t));
            ZVAL_STRINGL(TOKEN_MINOR(t), "UNKNOWN", sizeof("UNKNOWN"), 1);
    }
    return t;
}

META_API zval* meta_scanner_token_zval(TOKEN* t) {
    zval* tzv;
    MAKE_STD_ZVAL(tzv);
    array_init(tzv);
    add_assoc_long(tzv, "major", TOKEN_MAJOR(t));
    return tzv;
}

#define TOKENS_COUNT(scanner) zend_ptr_stack_num_elements(scanner->buffer)
#define TOKEN_PUSH(scanner, tok) zend_ptr_stack_push(scanner->buffer, tok)
//#define TOKEN_PUSH(scanner, tok) if(TOKEN_MINOR(tok)) zval_add_ref(&TOKEN_MINOR(tok)); zend_llist_add_element(scanner->buffer, &tok)
//#define TOKEN_POP(scanner) *(TOKEN**)zend_llist_get_last_ex(scanner->buffer, NULL); zend_llist_remove_tail(scanner->buffer)
#define TOKEN_POP(scanner) zend_ptr_stack_pop(scanner->buffer)

/**
 * return
 * - NULL in case of an unrecoverable error, the scanner's err_no will be set
 * - TOKEN* with major < 0 in case of a regular error, specific to a token
 * - TOKEN* with major 0 for EOI
 * - TOKEN* with major > 0 for tokens
 */
META_API TOKEN* meta_scan(meta_scanner* scanner TSRMLS_DC) {
    //where the cursor was positioned the last time, before calling this function
    YYCTYPE* last_cursor;
    //the return value
    TOKEN* token;

    //enables some rules to share code
    int transient_delta;
    int transient_major;
    //if between last_cursor and YYCURSOR are new lines, this will hold the line number at the point last_cursor;
    unsigned int last_line_no;

//interface macros
#define YYCURSOR scanner->cursor
#define YYLIMIT scanner->limit
#define YYMARKER scanner->marker
#define YYCTXMARKER scanner->ctxmarker

#define YYFILL(n) { if(YYCURSOR + n-1 > YYLIMIT) { scanner->err_no = ERR_FILLOVERFLOW; return NULL; } }
#define YYGETCONDITION() scanner->state
#define YYSETCONDITION(cond) scanner->state = cond

//enter a new state
#define SETSTATE(st) YYSETCONDITION(ST_NAME(st))
//jumping around 
#define yymore() goto lex_start
#define yyless() YYCURSOR--; goto lex_start

#ifdef DEBUG
#define RETURN(tok) if(NULL != token) php_printf("\n\n\n\tyou are giving up a token! (line %d)\n\n\n", __LINE__); \
    token = tok; goto lex_end
#else
#define RETURN(tok)
#endif

lex_root:
    token = NULL;
    last_cursor = YYCURSOR;
    last_line_no = scanner->line_no;
if(TOKENS_COUNT(scanner)) {
    token = TOKEN_POP(scanner);
    goto lex_end;
}

/*!re2c
re2c:define:YYDEBUG = DBG_SCANNER;
re2c:yyfill:check = 0;
LNUM    [0-9]+
DNUM    ([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)
EXPONENT_DNUM   (({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM    "0x"[0-9a-fA-F]+
LABEL   [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]
NON_WS [^ \n\r\t]
TABS_AND_SPACES [ \t]
TOKENS [;:,.\[\]()|^&+-/*=%!~$<>?@]
ANY_CHAR [^]
EOI [\000]
NEWLINE ("\r"|"\n"|"\r\n")
PHP_START "<?php"
PHP_START_EX ("<?="|"<?"|"<%="|"<%")
PHP_STOP ("?>"|"%>")
*/

lex_start:
/*!re2c
<ST_INITIAL>{ANY_CHAR} {
    //TODO detect EOL, increment scanner->line_no
    if(IS_EOL(YYCURSOR)) {
        scanner->line_no++;
    }
    if(YYCURSOR > YYLIMIT - YYMAXFILL) {
        TOKEN *outside;
        outside = ast_token_ctor(scanner, T_OUTSIDE_SCRIPTING, last_cursor, YYCURSOR - last_cursor);
        TOKEN *eoi;
        eoi = ast_token_ctor(scanner, 0, NULL, 0);
        TOKEN_PUSH(scanner, eoi);
        RETURN(outside);
    }
    else {
        yymore();
    }
}

<ST_INITIAL>{PHP_START}/{WHITESPACE}|{EOI} {
    transient_delta = 5;
    transient_major = T_OPEN_TAG;
do_transient_start:
    SETSTATE(IN_SCRIPTING);
    TOKEN *open_tag;
    open_tag = ast_token_ctor(scanner, transient_major, YYCURSOR - transient_delta, transient_delta);
    if(last_cursor == YYCURSOR - transient_delta) {
        RETURN(open_tag);
    }
    else {
        TOKEN_PUSH(scanner, open_tag);
        TOKEN* outside;
        outside = ast_token_ctor(scanner, T_OUTSIDE_SCRIPTING, last_cursor, YYMARKER - last_cursor - 2);
        RETURN(outside);
    }
}

<ST_INITIAL>{PHP_START_EX} {
    transient_delta = 2;
    transient_major = T_OPEN_TAG;
    if('=' == *(YYCURSOR-1)) {
        transient_delta++;
        transient_major = T_OPEN_TAG_WITH_ECHO;
    }
    if( (HAS_FLAG(scanner, SHORT_OPEN_TAG) && '?' == *(YYCURSOR - transient_delta + 1)) ||
        (HAS_FLAG(scanner, ASP_TAGS) && '%' == *(YYCURSOR - transient_delta + 1)) ) {
            goto do_transient_start;
    }
    else {
        yymore();
    }
}
<ST_IN_SCRIPTING>{PHP_STOP} {
    TOKEN *stop;
    SETSTATE(INITIAL);
    stop = ast_token_ctor(scanner, T_CLOSE_TAG, last_cursor, YYCURSOR - last_cursor);
    RETURN(stop);
}
<ST_IN_SCRIPTING>{EOI} {
    TOKEN *eoi;
    eoi = ast_token_ctor(scanner, 0, NULL, 0);
    RETURN(eoi);
}

<ST_IN_SCRIPTING>{WHITESPACE}/{NON_WS} {
    TOKEN *ws;
    ws = ast_token_ctor(scanner, T_WHITESPACE, last_cursor, YYCURSOR - last_cursor);
    if(IS_EOL(YYCURSOR-1)) {
        scanner->line_no++;
    }
    RETURN(ws);
}
<ST_IN_SCRIPTING>{WHITESPACE}/{WHITESPACE}{
    if(IS_EOL(YYCURSOR)) {
        scanner->line_no++;
    }
    yymore();
}
/* ***** "top" tokens ***** */
<ST_IN_SCRIPTING>{LNUM} {
    TOKEN* num;
    num = ast_token_ctor(scanner, T_LNUMBER, last_cursor, YYCURSOR - last_cursor);
    RETURN(num);
}
<ST_IN_SCRIPTING>"+" {
    TOKEN *plus;
    plus = ast_token_ctor(scanner, T_PLUS, last_cursor, YYCURSOR - last_cursor);
    RETURN(plus);
}
/*

these could be merged
<ST_IN_SCRIPTING>{TABS_AND_SPACES}+
<ST_IN_SCRIPTING>{NEWLINE}

TODO states:
ST_BACKQUOTE ST_DOUBLE_QUOTES ST_END_HEREDOC ST_IN_SCRIPTING ST_NOWDOC ST_VAR_OFFSET ST_LOOKING_FOR_PROPERTY ST_LOOKING_FOR_VARNAME ST_NOWDOC
*/

*/
lex_end:
    if(!token->start_line) {
        token->start_line = last_line_no;
    }
    if(!token->end_line) {
        token->end_line = scanner->line_no;
    }
    if(0 == TOKEN_MAJOR(token)) {
        scanner->err_no = ERR_EOI;
    }
    return token;
}

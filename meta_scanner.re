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
#include <zend_API.h>
#include <php.h>
#include "meta_scanner.h"
#include "scanner_API.h"
#include "meta_parser.h"
#include <errno.h>

#define IS_EOL(c) *(c) == '\n' || (*(c) == '\r' && *((c)+1) != '\n')

#ifdef DEBUG
# if 1
#  define DBG_SCANNER(state, c) php_printf("\t\t\tlex state %d, cursor '%c'(%d)\n", state, c, c)
# else
#  define DBG_SCANNER(state, c)
# endif
#define DBG(fmt, args...) php_printf("\t\t(pos %d)\t", YYCURSOR - scanner->src); php_printf(fmt, ## args); php_printf("\n")

#define SCANNER_ZDUMP(pzv) do { php_printf("-- (%d : '%s') %p: ",__LINE__, __PRETTY_FUNCTION__, pzv); if(NULL != pzv) php_debug_zval_dump(&(pzv), 0 TSRMLS_CC); } while(0)

#else
#define DBG_SCANNER(state, c)
#define PRINT_DBG(fmt, args...)
#endif

/*!max:re2c */

const unsigned int meta_scanner_maxfill = YYMAXFILL;

/* TODO:
 - create map for T_ numbers and TOKEN_IS_DISPENSABLE
 - create map for T_ numbers and tokens which should not have a minor value (e.g '+', whose value is implicit) */
META_API TOKEN* ast_token_ctor(meta_scanner* scanner, int major, char* start, int len TSRMLS_DC) {
	TOKEN* t;
	int errcode=0;
	long number;

	/* TODO take SFLAG_SIMPLE_KEYWORDS into account */
	t = emalloc(sizeof(TOKEN));
	TOKEN_MAJOR(t) = major;
	TOKEN_MINOR(t) = NULL;
	t->dirty = 0;
	/* TODO do something about free_me, either drop it completely, or redesign its role in token chains (meta_token_dtor) */
	t->free_me = 1;
	t->start_line = 0;
	t->end_line = 0;
	t->prev = NULL;
	t->next = NULL;
	TOKEN_IS_DISPENSABLE(t) = 0;
	switch(major) {
	case 0:
		break;
	case T_PLUS:
		MAKE_STD_ZVAL(TOKEN_MINOR(t));
		ZVAL_STRINGL(TOKEN_MINOR(t), start, len, 1);
		break;
	case T_OUTSIDE_SCRIPTING:
		MAKE_STD_ZVAL(TOKEN_MINOR(t));
		ZVAL_STRINGL(TOKEN_MINOR(t), start, len, 1);
		break;
	case T_OPEN_TAG:
		MAKE_STD_ZVAL(TOKEN_MINOR(t));
		ZVAL_STRINGL(TOKEN_MINOR(t), start, len, 1);
		break;
	case T_OPEN_TAG_WITH_ECHO:
		MAKE_STD_ZVAL(TOKEN_MINOR(t));
		ZVAL_STRINGL(TOKEN_MINOR(t), start, len, 1);
		break;
	case T_WHITESPACE:
		MAKE_STD_ZVAL(TOKEN_MINOR(t));
		ZVAL_STRINGL(TOKEN_MINOR(t), start, len, 1);
		break;
	case T_SEMICOLON:
		MAKE_STD_ZVAL(TOKEN_MINOR(t));
		ZVAL_STRINGL(TOKEN_MINOR(t), start, len, 1);
		break;
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
				/* TODO remove me */
				perror(NULL);
				/* TODO do we want the original input as string, or INT_MAX as now? */
			}
		}
		else {
			ZVAL_LONG(TOKEN_MINOR(t), strtol(start, NULL, 0));
		}
		break;
	default:
		MAKE_STD_ZVAL(TOKEN_MINOR(t));
		ZVAL_STRINGL(TOKEN_MINOR(t), "UNKNOWN", sizeof("UNKNOWN"), 1);
		php_error_docref(NULL TSRMLS_CC, E_CORE_ERROR, "Unknown terminal %d", major);
	}
    if(major == T_WHITESPACE) {
        TOKEN_IS_DISPENSABLE(t) = 1;
    }
	return t;
}

#define TOKENS_COUNT(scanner) zend_ptr_stack_num_elements(scanner->buffer)
#define TOKEN_PUSH(scanner, tok) zend_ptr_stack_push(scanner->buffer, tok)
#define TOKEN_POP(scanner) zend_ptr_stack_pop(scanner->buffer)

/**
 * return
 * - NULL in case of an unrecoverable error, the scanner's err_no will be set
 * - TOKEN* with major < 0 in case of a regular error, specific to a token
 * - TOKEN* with major 0 for EOI
 * - TOKEN* with major > 0 for tokens
 */
META_API TOKEN* meta_scan(meta_scanner* scanner TSRMLS_DC) {
	/* where the cursor was positioned the last time, before calling this function */
	YYCTYPE* last_cursor;
	/* the return value */
	TOKEN* token;

	/* enables some rules to share code */
	int transient_delta;
	int transient_major;
	/* if between last_cursor and YYCURSOR are new lines, this will hold the line number at the point last_cursor; */
	long last_line_no;

	/* interface macros */
#define YYCURSOR scanner->cursor
#define YYLIMIT scanner->limit
#define YYMARKER scanner->marker
#define YYCTXMARKER scanner->ctxmarker

#define YYFILL(n) { if(YYCURSOR + n-1 > YYLIMIT) { scanner->err_no = ERR_FILLOVERFLOW; return NULL; } }
#define YYGETCONDITION() scanner->state
#define YYSETCONDITION(cond) scanner->state = cond

	/* enter a new state */
#define SETSTATE(st) YYSETCONDITION(ST_NAME(st))
	/* jumping around  */
#define yymore() goto lex_start
#define yyless() YYCURSOR--; goto lex_start

#ifdef DEBUG
#define RETURN(tok) if(NULL != token) php_printf("\n\n\n\tyou are giving up a token! (line %d)\n\n\n", __LINE__); \
    token = tok; goto lex_end
#else
#define RETURN(tok) token = tok; goto lex_end
#endif

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
    if(IS_EOL(YYCURSOR)) {
        scanner->line_no++;
    }
    if(YYCURSOR > YYLIMIT - YYMAXFILL) {
        TOKEN *outside, *eoi;
        outside = ast_token_ctor(scanner, T_OUTSIDE_SCRIPTING, last_cursor, YYCURSOR - last_cursor TSRMLS_CC);
        eoi = ast_token_ctor(scanner, 0, NULL, 0 TSRMLS_CC);
        TOKEN_PUSH(scanner, eoi);
        RETURN(outside);
    }
    else {
        yymore();
    }
}

<ST_INITIAL>{PHP_START}/{WHITESPACE}|{EOI} {
    TOKEN *open_tag;
    transient_delta = 5;
    transient_major = T_OPEN_TAG;
do_transient_start:
    SETSTATE(IN_SCRIPTING);
    open_tag = ast_token_ctor(scanner, transient_major, YYCURSOR - transient_delta, transient_delta TSRMLS_CC);
    if(last_cursor == YYCURSOR - transient_delta) {
        RETURN(open_tag);
    }
    else {
        TOKEN* outside;
        TOKEN_PUSH(scanner, open_tag);
        outside = ast_token_ctor(scanner, T_OUTSIDE_SCRIPTING, last_cursor, YYMARKER - last_cursor - 2 TSRMLS_CC);
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
    stop = ast_token_ctor(scanner, T_CLOSE_TAG, last_cursor, YYCURSOR - last_cursor TSRMLS_CC);
    RETURN(stop);
}
<ST_IN_SCRIPTING>{EOI} {
    TOKEN *eoi;
    eoi = ast_token_ctor(scanner, 0, NULL, 0 TSRMLS_CC);
    RETURN(eoi);
}

<ST_IN_SCRIPTING>{WHITESPACE}/{NON_WS} {
    TOKEN *ws;
    ws = ast_token_ctor(scanner, T_WHITESPACE, last_cursor, YYCURSOR - last_cursor TSRMLS_CC);
    if(IS_EOL(YYCURSOR-1)) {
        scanner->line_no++;
    }
    TOKEN_IS_DISPENSABLE(ws) = 1;
    RETURN(ws);
}
<ST_IN_SCRIPTING>{WHITESPACE}/{WHITESPACE}{
    if(IS_EOL(YYCURSOR)) {
        scanner->line_no++;
    }
    yymore();
}
/* ***** "top" tokens ***** */
<ST_IN_SCRIPTING> {LNUM} {
    TOKEN* num;
    num = ast_token_ctor(scanner, T_LNUMBER, last_cursor, YYCURSOR - last_cursor TSRMLS_CC);
    RETURN(num);
}
<ST_IN_SCRIPTING>"+" {
    TOKEN *plus;
    plus = ast_token_ctor(scanner, T_PLUS, last_cursor, YYCURSOR - last_cursor TSRMLS_CC);
    RETURN(plus);
}
<ST_IN_SCRIPTING>";" {
    TOKEN *semicolon;
    semicolon = ast_token_ctor(scanner, T_SEMICOLON, last_cursor, YYCURSOR - last_cursor TSRMLS_CC);
    RETURN(semicolon);
}
/*

these could be merged
<ST_IN_SCRIPTING>{TABS_AND_SPACES}+
<ST_IN_SCRIPTING>{NEWLINE}

TODO states:
ST_BACKQUOTE ST_DOUBLE_QUOTES ST_HEREDOC ST_IN_SCRIPTING ST_NOWDOC ST_VAR_OFFSET ST_LOOKING_FOR_PROPERTY ST_LOOKING_FOR_VARNAME
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

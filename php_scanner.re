#include <zend.h>
#include <string.h>
#include <stdlib.h> //strtol()
#include "php_meta.h"
#include "php_scanner_defs.h"
#include "php_scanner.h"
#include "php_parser.h"

#define YYCTYPE char
#define STATE(name) yyc##name

#define RE2CDBG 0
#define METADBG 1

#if RE2CDBG
#define META_DEBUG(code, msg) php_printf("lex: #%d '%c'(%d)\n", code, msg, msg)
#else 
#define META_DEBUG(code, msg)
#endif

#if METADBG
#define PRINT_DBG(fmt, args...) php_printf("\t\t"); php_printf(fmt, ## args); php_printf("\n")
#else
#define PRINT_DBG(fmt, args...) 
#endif


/*!max:re2c */

meta_scanner* meta_scanner_alloc(zval* src) {
    meta_scanner *r;
    char *lim;
    zend_bool free_raw=0;//TODO remove
    Z_STRVAL_P(src) = erealloc(Z_STRVAL_P(src), Z_STRLEN_P(src) + YYMAXFILL);
    //Z_SET_REFCOUNT_P(src, 0);
    memset(Z_STRVAL_P(src)+Z_STRLEN_P(src), 0, YYMAXFILL);
    lim = Z_STRVAL_P(src) + YYMAXFILL + Z_STRLEN_P(src) - 1;
    r = emalloc(sizeof(meta_scanner));
    r->free_raw = free_raw = 0;
    r->src = Z_STRVAL_P(src);
    r->src_len = Z_STRLEN_P(src);
    r->cursor = r->src;
    r->marker = r->cursor;
    r->ctxmarker = NULL;
    r->state = STATE(ST_INITIAL);
    r->position = 0;
    r->limit = lim;
    r->buffer_majors = NULL;
    r->buffer_minors = NULL;
    r->buffer_size = 0;
    return r;
}

void meta_scanner_destroy(meta_scanner** scanner) {
    if((*scanner)->free_raw) {
        efree((*scanner)->src);
    }
    size_t i;
    for(i=0; i < (*scanner)->buffer_size; i++) {
        if((*scanner)->buffer_minors[i] == NULL) {
            continue;
        }
        zval_dtor((*scanner)->buffer_minors[i]);
        efree((*scanner)->buffer_minors[i]);
    }
    if((*scanner)->buffer_size) {
        efree((*scanner)->buffer_majors);
        efree((*scanner)->buffer_minors);
    }
    efree(*scanner);
}

//TODO out of memory checking
int meta_scanner_pushtoken(meta_scanner* scanner, int major, zval* minor) {
    scanner->buffer_minors = erealloc(scanner->buffer_minors, sizeof(zval*) * (scanner->buffer_size+1));
    scanner->buffer_majors = erealloc(scanner->buffer_majors, sizeof(major) * (scanner->buffer_size+1));
    scanner->buffer_minors[scanner->buffer_size] = minor;
    scanner->buffer_majors[scanner->buffer_size] = major;
    scanner->buffer_size++;
    return 1;
}

zval* meta_scanner_poptoken(meta_scanner* scanner, int* major) {
    zval *minor;
    if(0 == scanner->buffer_size) {
        return NULL;
    }
    scanner->buffer_size--;
    *major = scanner->buffer_majors[scanner->buffer_size];
    minor = scanner->buffer_minors[scanner->buffer_size];
    if(scanner->buffer_size) {
        scanner->buffer_minors = erealloc(scanner->buffer_minors, sizeof(zval*) * (scanner->buffer_size));
        scanner->buffer_majors = erealloc(scanner->buffer_majors, sizeof(int) * (scanner->buffer_size));
    }
    else {
        efree(scanner->buffer_minors);
        efree(scanner->buffer_majors);
        scanner->buffer_majors = NULL;
        scanner->buffer_minors = NULL;
    }
    return minor;
}

static inline int is_start_tag(meta_scanner* s) {
    //TODO return 0 if not, X for each PHP_STARTx, taking scanner's flags into account
    if('<' == *s->cursor) return 1;
    return 0;
}



//TODO return int
int meta_scan(meta_scanner* scanner, zval** minor TSRMLS_DC) {
    char* last_cursor;
    int major;
    //enables some rules to share code
    int transient_delta;
    //fixing side-effects of calling yymore();
    zend_bool is_more;

#define YYCURSOR scanner->cursor
#define YYLIMIT scanner->limit
#define YYMARKER scanner->marker
#define YYCTXMARKER scanner->ctxmarker
//TODO return -1
//#define YYFILL(n) { PRINT_DBG("fill %d\n", n); if(YYCURSOR + n-1 > YYLIMIT) return -1; PRINT_DBG("filled\n"); }
#define YYFILL(n) { if(YYCURSOR + n-1 > YYLIMIT) return ERR_FILLOVERFLOW; }
//#define YYFILL(n) { PRINT_DBG("fill %d\n", n); if(!(YYCURSOR + n > YYLIMIT)) PRINT_DBG("filled\n"); }
#define YYGETCONDITION() scanner->state
#define YYSETCONDITION(s) scanner->state = s
#define SETSTATE(s) YYSETCONDITION(STATE(s))

#define yymore() is_more = 1; goto lex_start
#define yyless() YYCURSOR--; goto lex_start;
//ignore the current terminal
#define yyrerun() goto lex_root
#define RETURN(m) major = m; goto lex_end;


if(scanner->buffer_size) {
    *minor = meta_scanner_poptoken(scanner, &major);
    //TODO increment the number of lexemes of the scanner
    RETURN(major);
}

lex_root:
    last_cursor = scanner->cursor;
    *minor = NULL;
    major=ERR_UNITIALIZED;
    is_more = 0;

/*!re2c
re2c:define:YYDEBUG = META_DEBUG;
re2c:yyfill:check = 0;
LNUM    [0-9]+
DNUM    ([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)
EXPONENT_DNUM   (({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM    "0x"[0-9a-fA-F]+
LABEL   [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]*
TOKENS [;:,.\[\]()|^&+-/*=%!~$<>?@]
ANY_CHAR [^]
EOI [\000]
NEWLINE ("\r"|"\n"|"\r\n")
PHP_START "<?php"
PHP_START_EX ("<?="|"<?"|"<%="|"<%")
*/

//TODO flags to set for T_OPEN_TAG: IS_SHORT IS_ASP IS_ECHO EOL_PLATFORM {"unix", "windows", "mac"}

lex_start:
/*!re2c
<!*> := PRINT_DBG("new LEXEME\n"); //TODO increment scanner's lexemes count

<ST_INITIAL>{ANY_CHAR} {
    PRINT_DBG("inline html '%c'\n", *YYCURSOR);
    if(*YYCURSOR == '\0') {//TODO more detection so we're not hampered by NULs in input
        meta_scanner_pushtoken(scanner, 0, NULL);
        MAKE_STD_ZVAL(*minor);
        ZVAL_STRINGL(*minor, last_cursor, YYCURSOR - last_cursor + 1, 1);
        return T_OUTSIDE_SCRIPTING;
    }
    else {
        yymore();
    }
    /*
    else if(*YYCURSOR == '<' && (*(YYCURSOR+1) == '?' || *(YYCURSOR+1) == '%')) {
        if(YYCURSOR != last_cursor) {//if not at the beginning, we want to 
                                    //unwind one char and give IN_SCRIPTING kick in
            yyless();
        }
        else {
            yymore();
        }
    }
    else {
        yymore();
    }
    */
}
<ST_INITIAL>{PHP_START}/{WHITESPACE}|{EOI} {
    PRINT_DBG("PHP_START");
    transient_delta=5;
do_start:
    SETSTATE(ST_IN_SCRIPTING);
    zval* open_tag=NULL;
    MAKE_STD_ZVAL(open_tag);
    ZVAL_STRINGL(open_tag, YYCURSOR - transient_delta, transient_delta, 1);
    if(last_cursor == YYCURSOR - transient_delta) {
        *minor = open_tag;
        RETURN(T_OPEN_TAG);
    }
    else {
        meta_scanner_pushtoken(scanner, T_OPEN_TAG, open_tag);
        MAKE_STD_ZVAL(*minor);
        ZVAL_STRINGL(*minor, last_cursor, YYMARKER - last_cursor - 1, 1);
        RETURN(T_OUTSIDE_SCRIPTING);
    }
}

<ST_INITIAL>{PHP_START_EX} {
    PRINT_DBG("PHP_START_EX");
    //TODO check if asp_tags on
    if('=' == *(YYCURSOR-1)) {//TODO && scanner.short_open_tags, otherwise pass through as HTML
        transient_delta = 3;
    }
    else {
        transient_delta = 2;
    }
    goto do_start;
}
<ST_INITIAL>{EOI} {
    RETURN(0);
}
<ST_IN_SCRIPTING>{EOI} {
    RETURN(0);
}
<ST_IN_SCRIPTING>{TABS_AND_SPACES} {
    //TODO if tabs and spaces, fill them, else yyrerun()
    PRINT_DBG("indentation '%s' '%c'\n", last_cursor, *YYCURSOR);
    MAKE_STD_ZVAL(*minor);
    ZVAL_STRINGL(*minor, last_cursor, YYCURSOR - last_cursor, 1);
    RETURN(T_WHITESPACE);
}
<ST_IN_SCRIPTING>{NEWLINE} {
    //TODO adjust line number, then if newline-as-lexeme, RETURN(T_NEWLINE), else yyrerun()
}

<ST_IN_SCRIPTING>{LNUM} {
    MAKE_STD_ZVAL(*minor);
    char* end;
    ZVAL_LONG(*minor, strtol(last_cursor, &end, 0));
    PRINT_DBG("lnumber '%s' last: '%c'\n", last_cursor, *end);
    return T_LNUMBER;
}
<ST_IN_SCRIPTING>{TABS_AND_SPACES} {
    //TODO check if whitespace is wanted
    MAKE_STD_ZVAL(*minor);
    ZVAL_STRINGL(*minor, last_cursor, YYCURSOR - last_cursor, 1);
    return T_WHITESPACE;
}
<ST_IN_SCRIPTING>"?>" {
    PRINT_DBG("out of scripting '%s'\n", last_cursor);
    SETSTATE(ST_INITIAL);
    return T_CLOSE_TAG;
}
*/

lex_end:
//TODO prepare minor, eventually based on hooks
    return major;
}

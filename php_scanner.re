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
	r->marker = NULL;
	r->cursor = r->src;
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
int meta_scanner_pushtoken(meta_scanner* scanner, ulong major, zval* minor) {
	scanner->buffer_minors = erealloc(scanner->buffer_minors, sizeof(zval*) * (scanner->buffer_size+1));
	scanner->buffer_majors = erealloc(scanner->buffer_majors, sizeof(ulong) * (scanner->buffer_size+1));
	scanner->buffer_minors[scanner->buffer_size] = minor;
	scanner->buffer_majors[scanner->buffer_size] = major;
	scanner->buffer_size++;
	return 1;
}

zval* meta_scanner_poptoken(meta_scanner* scanner, ulong* major) {
	zval *minor;
	if(0 == scanner->buffer_size) {
		return NULL;
	}
	scanner->buffer_size--;
	*major = scanner->buffer_majors[scanner->buffer_size];
	minor = scanner->buffer_minors[scanner->buffer_size];
	if(scanner->buffer_size) {
		scanner->buffer_minors = erealloc(scanner->buffer_minors, sizeof(zval*) * (scanner->buffer_size));
		scanner->buffer_majors = erealloc(scanner->buffer_majors, sizeof(ulong) * (scanner->buffer_size));
	}
	else {
		efree(scanner->buffer_minors);
		efree(scanner->buffer_majors);
		scanner->buffer_majors = NULL;
		scanner->buffer_minors = NULL;
	}
	return minor;
}

/*
static inline char *
zend_memnstr(char *haystack, char *needle, int needle_len, char *end)
{
        char *p = haystack;
        char ne = needle[needle_len-1];

        if (needle_len == 1) {
                return (char *)memchr(p, *needle, (end-p));
        }

        if (needle_len > end-haystack) {
                return NULL;
        }

        end -= needle_len;

        while (p <= end) {
                if ((p = (char *)memchr(p, *needle, (end-p+1))) && ne == p[needle_len-1]) {
                        if (!memcmp(needle, p, needle_len-1)) {
                                return p;
                        }
                }

                if (p == NULL) {
                        return NULL;
                }

                p++;
        }

        return NULL;
}

*/
static inline int is_start_tag(meta_scanner* s) {
	//TODO return 0 if not, X for each PHP_STARTx, taking scanner's flags into account
	if('<' == *s->cursor) return 1;
	return 0;
}



//TODO return int
int meta_scan(meta_scanner* scanner, zval** minor TSRMLS_DC) {
	 char* last_cursor = scanner->cursor;
	*minor = NULL;

#define YYCURSOR scanner->cursor
#define YYLIMIT scanner->limit
#define YYMARKER scanner->marker
//TODO return -1
//#define YYFILL(n) { PRINT_DBG("fill %d\n", n); if(YYCURSOR + n-1 > YYLIMIT) return -1; PRINT_DBG("filled\n"); }
#define YYFILL(n) { if(YYCURSOR + n-1 > YYLIMIT) return -1; }
//#define YYFILL(n) { PRINT_DBG("fill %d\n", n); if(!(YYCURSOR + n > YYLIMIT)) PRINT_DBG("filled\n"); }
#define YYGETCONDITION() scanner->state
#define YYSETCONDITION(s) scanner->state = s
#define SETSTATE(s) YYSETCONDITION(STATE(s))

#define yymore() goto lex_start


if(scanner->buffer_size) {
	int major;
	*minor = meta_scanner_poptoken(scanner, &major);
	return major;
}

lex_start:

/*!re2c
re2c:define:YYDEBUG = META_DEBUG;
re2c:yyfill:check = 0;
LNUM	[0-9]+
DNUM	([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)
EXPONENT_DNUM	(({LNUM}|{DNUM})[eE][+-]?{LNUM})
HNUM	"0x"[0-9a-fA-F]+
LABEL	[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*
WHITESPACE [ \n\r\t]+
TABS_AND_SPACES [ \t]+
ANY_CHAR [^\000]
EOI	[\000]
NEWLINE ("\r"|"\n"|"\r\n")
NEWLINE_WS {NEWLINE}|[ \t]
END_OF_START {NEWLINE}|[ \t]|{EOI}
PHP_START1 "<?php"{END_OF_START}
PHP_START2 "<?="
PHP_START3 "<?"
PHP_START4 "<%="
PHP_START5 "<%"
PHP_START "<?"
PHP_START_ASP "<%"
*/

//TODO flags to set for T_OPEN_TAG: IS_SHORT IS_ASP IS_ECHO EOL_PLATFORM {"unix", "windows", "mac"}

/*!re2c
<!*> := PRINT_DBG("setting up\n");

<ST_INITIAL>{PHP_START1} {
	SETSTATE(ST_IN_SCRIPTING);
	int ret;
	zval* open_tag=NULL;
	MAKE_STD_ZVAL(open_tag);
	ZVAL_STRINGL(open_tag, YYMARKER-1, YYCURSOR-YYMARKER, 1);
	if(0 == YYMARKER - last_cursor - 1) {
		PRINT_DBG("%d\n", __LINE__);
		PRINT_DBG("clean start\n");
		*minor = open_tag;
		ret = T_OPEN_TAG;
	}
	else {
		PRINT_DBG("%d\n", __LINE__);
		meta_scanner_pushtoken(scanner, T_OPEN_TAG, open_tag);
		MAKE_STD_ZVAL(*minor);
		ZVAL_STRINGL(*minor, last_cursor, YYMARKER - last_cursor -1, 1);
		ret = T_OUTSIDE_SCRIPTING;
	}
	//TODO check for EOL, whitespaces, and if desired, return them too
	if('\n' == *(YYCURSOR-1) && '\r' == *(YYCURSOR-2)) {
	}
	PRINT_DBG("initial state\n");
	return ret;
}

<ST_INITIAL>{ANY_CHAR} {
	PRINT_DBG("inline html\n");
	if(*YYCURSOR == '\0') {
		meta_scanner_pushtoken(scanner, 0, NULL);
		MAKE_STD_ZVAL(*minor);
		ZVAL_STRINGL(*minor, last_cursor, YYCURSOR - last_cursor + 1, 1);
		return T_OUTSIDE_SCRIPTING;
	}
	else if(*YYCURSOR == '<' && (*(YYCURSOR+1) == '?' || *(YYCURSOR+1) == '%')) {
		yymore();
	}
	PRINT_DBG("filling inline html with '%c'\n", *YYCURSOR);
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
<ST_INITIAL>{EOI} {
	*minor = NULL;
	PRINT_DBG("end of input\n");
	return 0;
}
<ST_IN_SCRIPTING>{EOI} {
	PRINT_DBG("end of input\n");
	return 0;
}
*/
}
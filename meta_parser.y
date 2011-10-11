%include {
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
#include <php.h>
#include "meta_scanner.h"
#include "meta_parser.h"
#include "parser_API.h"
#include "scanner_API.h"
#include "parser.h"

#if ZTS
#undef TSRMLS_C
#define TSRMLS_C yypParser->tsrm_ls
#else
#define TSRMLS_C
#define TSRMLS_CC
#endif

#undef TSRMG
#define TSRMG(id, type, element) (((type) (*((void ***) yypParser->tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])->element)

#ifdef TSRMLS_C
#undef TSRMLS_C
#define TSRMLS_C yypParser->tsrm_ls
#endif


	/*TODO rename it globally*/
	typedef TOKEN Token;

#ifndef NDEBUG
	/*TODO remove*/
#include <assert.h>
#endif

#include "meta_parser.h"

	/*TODO maybe move it to the scanner?*/
	static const char *const yyTokenName[];
	const char* meta_token_repr(int n) {
		if(n > T_INTERNAL_SKIP) {
			return "UNKNOWN";
		}
		return yyTokenName[n];
	}


#if 1
#define DBG(fmt, args...) php_printf("\t\t"); php_printf(fmt, ## args); php_printf("\n")
#else
#define DBG(fmt, args...)
#endif

#define META_PARSER_REV_FILL(upto, start, obj, where) do { TOKEN* cursor; cursor = start; \
        while(upto != cursor->prev) { cursor = cursor->prev; } \
        while(cursor != start) { \
            META_CALL_METHOD(obj, appendbetween, "zl", TOKEN_MINOR(cursor), (long)where); \
            cursor = cursor->next; \
            efree(cursor->prev); \
        } \
    } while(0)

#define META_PARSER_FW_FILL(upto, start, class, obj, where) do { \
        if(upto != start->next) { \
            TOKEN *cursor, *prev; \
            cursor = start->next; \
            while(upto != cursor) { \
                META_CALL_METHOD(obj, appendbetween, "zl", TOKEN_MINOR(cursor), (long)where); \
                prev = cursor; \
                cursor = cursor->next; \
                efree(prev); \
            } \
        } \
    } while(0)

}
%name MetaParser
%start_symbol start
%token_prefix T_
%token_type {Token*}
%extra_argument { zval* tree }

%token_destructor {
	/* yeah, we shouldn't hide the warning */
	if(tree == tree) {}
}

%parse_accept {
	/* php_printf("\t\t\t\t\t ACCEPT!\n"); */
}
%parse_failure {
	php_printf("\t\t\t\t\t FAILURE!\n");
}

%left INCLUDE INCLUDE_ONCE EVAL REQUIRE REQUIRE_ONCE.
%left COMMA.
%left LOGICAL_OR.
%left LOGICAL_XOR.
%left LOGICAL_AND.
%right PRINT.
%right ASSIGN PLUS_ASSIGN MINUS_ASSING MUL_ASSIGN DIV_ASSIGN DOT_ASSIGN MOD_ASSIGN BIN_AND_ASSIGN BIN_OR_ASSIGN BIN_XOR_ASSIGN BIN_SHL_ASSIGN BIN_SRH_ASSIGN.
%left QUESTION_MARK COLON.
%left BOOL_OR.
%left BOOL_AND.
%left BIN_OR.
%left BIN_XOR.
%left BIN_AND.
%nonassoc EQUAL NOT_EQUAL IDENTICAL NOT_IDENTICAL.
%nonassoc LESS_THAN LESS_OR_EQUAL GREATER_THAN GREATER_OR_EQUAL DIFFERENT.
%left BIN_SHL BIN_SHR.
%left PLUS MINUS DOT.
%left MUL DIV MOD.
%right NEGATE.
%nonassoc INSTANCEOF.
%right BIN_NOT INCREMENT DECREMENT NEGATIVE CAST_INT CAST_FLOAT CAST_STRING CAST_ARRAY CAST_OBJECT CAST_BOOL SHUTUP.
%nonassoc CLONE NEW.
%nonassoc EXIT.
%nonassoc IF.
%left ELSEIF.
%left ELSE.
%left ENDIF.
%nonassoc LNUMBER.
%nonassoc DNUMBER.
%nonassoc STRING.
%nonassoc STRING_VARNAME.
%nonassoc VARIABLE.
%nonassoc NUM_STRING.
%nonassoc INLINE_HTML.
%nonassoc CHARACTER.
%nonassoc BAD_CHARACTER.
%nonassoc ENCAPSED_AND_WHITESPACE.
%nonassoc CONSTANT_ENCAPSED_STRING.
%nonassoc ECHO.
%nonassoc DO.
%nonassoc WHILE.
%nonassoc ENDWHILE.
%nonassoc FOR.
%nonassoc ENDFOR.
%nonassoc FOREACH.
%nonassoc ENDFOREACH.
%nonassoc DECLARE.
%nonassoc ENDDECLARE.
%nonassoc AS.
%nonassoc SWITCH.
%nonassoc ENDSWITCH.
%nonassoc CASE.
%nonassoc DEFAULT.
%nonassoc BREAK.
%nonassoc CONTINUE.
%nonassoc GOTO.
%nonassoc FUNCTION.
%nonassoc CONST.
%nonassoc RETURN.
%nonassoc TRY.
%nonassoc CATCH.
%nonassoc THROW.
%nonassoc USE.
%nonassoc GLOBAL.
%right STATIC ABSTRACT FINAL PRIVATE PROTECTED PUBLIC.
%nonassoc VAR.
%nonassoc UNSET.
%nonassoc ISSET.
%nonassoc EMPTY.
%nonassoc HALT_COMPILER.
%nonassoc CLASS.
%nonassoc INTERFACE.
%nonassoc EXTENDS.
%nonassoc IMPLEMENTS.
%nonassoc OBJECT_OPERATOR.
%nonassoc DOUBLE_ARROW.
%nonassoc LIST.
%nonassoc ARRAY.
%nonassoc CLASS_C.
%nonassoc METHOD_C.
%nonassoc FUNC_C.
%nonassoc LINE.
%nonassoc FILE.
%nonassoc COMMENT.
%nonassoc DOC_COMMENT.
%nonassoc OPEN_TAG.
%nonassoc OPEN_TAG_WITH_ECHO.
%nonassoc CLOSE_TAG.
%nonassoc WHITESPACE.
%nonassoc START_HEREDOC.
%nonassoc END_HEREDOC.
%nonassoc PAAMAYIM_NEKUDOTAYIM.
%nonassoc NAMESPACE.
%nonassoc NS_C.
%nonassoc DIR.
%nonassoc NS_SEPARATOR.
%left LBRACKET RBRACKET LPAREN RPAREN LCURLY RCURLY.
%nonassoc SEMICOLON.

/* dummy tokens*/
%nonassoc OUTSIDE_SCRIPTING.
/* MUST be last, reserved, TODO: not used yet*/
%nonassoc INTERNAL_SKIP.

%type processing {zval*}
%type processing_stmt {zval*}
%type top_stmt_list {zval*}
%type top_stmt {zval*}
%type expr {zval*}
%type stmt_with_semicolon{zval*}

start(A) ::=  processing(B) . {
    DBG("reduction %d", __LINE__);
	/* A B */
}

processing(A) ::= . {
    DBG("reduction %d", __LINE__);
	/* A B C */
}

processing(A) ::= processing(B) processing_stmt(C) . {
    DBG("reduction %d", __LINE__);
	/*TODO instead of crafting nodes manually, use the tree as a factory,
	which instantiates the right classes if the user has some specific preferences*/
	META_CALL_METHOD(tree, appendchild, "z", C);
	/* A B C */
}

processing_stmt(A) ::= OUTSIDE_SCRIPTING(B) . {
    DBG("reduction %d", __LINE__);
	/*TODO init A as unary, set value to B, efree(B)*/
}

processing_stmt(A) ::= OPEN_TAG(B) top_stmt_list(C) . {
	zval *end_line;
    DBG("reduction %d", __LINE__);

	Z_ADDREF_P(tree);
	META_NODE_CTOR(nodelist, A, "z", tree);
	end_line = META_PROP(nodelist, C, "end_line");
	META_CALL_METHOD(A, setlines, "ll", B->start_line, Z_LVAL_P(end_line));
	META_CALL_METHOD(A, appendchild, "z", TOKEN_MINOR(B));
	/*TODO take tree's flags into consideration*/
	if(NULL != B->next) {
		TOKEN *cursor, *prev;
		cursor = B->next;
		while(NULL != cursor) {
			META_CALL_METHOD(A, appendchild, "z", TOKEN_MINOR(cursor));
			prev = cursor;
			cursor = cursor->next;
			efree(prev);
		}
	}
	META_CALL_METHOD(A, appendchild, "z", C);
	efree(B);
}

processing_stmt(A) ::= OPEN_TAG(B) top_stmt_list(C) CLOSE_TAG(D) . {
    DBG("reduction %d", __LINE__);
	/* A B C D */
}

top_stmt_list(A) ::= top_stmt(B) . {
	zval *start_line, *end_line;
    DBG("reduction %d", __LINE__);

	start_line = META_PROP(node, B, "start_line");
	end_line = META_PROP(node, B, "end_line");
	Z_ADDREF_P(tree);
	META_NODE_CTOR(nodelist, A, "z", tree);
	META_CALL_METHOD(A, appendchild, "z", B);
	META_CALL_METHOD(A, setlines, "ll", Z_LVAL_P(start_line), Z_LVAL_P(end_line));
}
top_stmt_list(A) ::= top_stmt_list(B) top_stmt(C) . {
    DBG("reduction %d", __LINE__);
	META_CALL_METHOD(B, appendchild, "z", C);
	A = B;
	/* A B C */
}

top_stmt(A) ::= stmt_with_semicolon(B) . {
    DBG("reduction %d", __LINE__);
    A = B; // C
}

stmt_with_semicolon(A) ::= expr(B) SEMICOLON(C) . {
    DBG("reduction %d", __LINE__);
    A = B;
    META_PARSER_REV_FILL(NULL, C, A, META_FILL_AFTER);
    META_CALL_METHOD(B, appendbetween, "zl", TOKEN_MINOR(C), (long)META_FILL_AFTER);
    efree(C);
}

expr(A) ::= expr(B) PLUS(C) expr(D) . {
    DBG("reduction %d", __LINE__);
	Z_ADDREF_P(tree); // C
	META_NODE_CTOR(binarynode, A, "lzzzz", (long)T_PLUS, tree, B, D, TOKEN_MINOR(C));
    META_PARSER_REV_FILL(NULL, C, A, META_FILL_BINARY_LHS_OPERATOR);
    META_PARSER_FW_FILL(NULL, C, binarynode, A, META_FILL_BINARY_OPERATOR_RHS);
	efree(C);
}

expr(A) ::= LNUMBER(B) . {
    DBG("reduction %d", __LINE__);
	Z_ADDREF_P(tree);
	META_NODE_CTOR(unarynode, A, "zlz", tree, (long)T_LNUMBER, TOKEN_MINOR(B));
	META_CALL_METHOD(A, setlines, "ll", B->start_line, B->end_line);
	B->prev->next = NULL;
	B->next->prev = NULL;
	efree(B);
}


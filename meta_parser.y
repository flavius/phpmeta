%include {
#include <zend.h>
#include <php.h>
#include "meta_scanner.h"
#include "meta_parser.h"
#include "parser_API.h"
#include "scanner_API.h"
#include "parser.h"
#include "php_meta.h" //TODO remove, as far as META_ZDUMP() is not used


//TODO use the same names for TSRM-related stuff
#define M_TSRMLS_D TSRMLS_D
#define M_TSRMLS_DC TSRMLS_DC

#if ZTS
#define M_TSRMLS_C yypParser->tsrm_ls
#define M_TSRMLS_CC , M_TSRMLS_C
#else
#define M_TSRMLS_C
#define M_TSRMLS_CC
#endif

#undef TSRMG
#define TSRMG(id, type, element) (((type) (*((void ***) yypParser->tsrm_ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])->element)

#ifdef TSRMLS_C
#undef TSRMLS_C
#define TSRMLS_C yypParser->tsrm_ls
#endif

//TODO rename it globally
typedef TOKEN Token;

#ifndef NDEBUG
//TODO remove
#include <assert.h>
#endif

#include "meta_parser.h"

//TODO maybe move it to the scanner?
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

//we need to rewrite emalloc and efree, as these are macros and the real functions need other parameters as well

}
%name MetaParser
%start_symbol start
%token_prefix T_
%token_type{Token*}
%extra_argument{ zval* tree }

%token_destructor {
    php_printf("\t\t\ttoken dtor major=%d addr=%p minor: ", TOKEN_MAJOR($$), $$);
    META_ZDUMP(TOKEN_MINOR($$));
    php_printf("\n");
    //yeah, we shouldn't hide the warning
    if(tree == tree) {}
    //TODO we should ast_token_dtor($$)
    //zval_ptr_dtor(&TOKEN_MINOR($$));
    //efree($$); $$ = NULL;
}

%parse_accept {
    php_printf("\t\t\t\t\t ACCEPT!\n");
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

// dummy tokens
%nonassoc OUTSIDE_SCRIPTING.
// MUST be last, reserved, TODO: not used yet
%nonassoc INTERNAL_SKIP.

%type processing {zval*}
%type processing_stmt {zval*}
%type top_stmt_list {zval*}
%type top_stmt {zval*}
%type expr {zval*}

start(A) ::=  processing(B) . {
    DBG("meta_parser.y %d", __LINE__);
    zval *t_B = B;
    /* A B */ // NEVER USED, the node is attached directly to the tree
}

processing ::= . {
    DBG("meta_parser.y %d", __LINE__);
}

processing(A) ::= processing(B) processing_stmt(C) . {
    DBG("meta_parser.y %d", __LINE__);
    zval *t_B = B;
    zval *t_C = C;
    //TODO instead of crafting nodes manually, use the tree as a factory,
    //which instantiates the right classes if the user has some specific preferences
    zend_function *appendChild;
    zend_hash_find(&META_CLASS(tree)->function_table, STRL_PAIR("appendchild"), (void**) &appendChild);
    zval* ret_t = obj_call_method_internal_ex(tree, META_CLASS(node), appendChild, EG(scope), 1 M_TSRMLS_CC, "z", C);
    zval_ptr_dtor(&ret_t);
    /* A B C */
}

processing_stmt(A) ::= OUTSIDE_SCRIPTING(B) . {
    DBG("meta_parser.y %d", __LINE__);
    TOKEN *t_B = B;
    ALLOC_INIT_ZVAL(A);
    A = obj_call_method_internal_ex(A, META_CLASS(node), META_CLASS(node)->constructor, EG(scope), 1 M_TSRMLS_CC,
        "lzllz", TOKEN_MAJOR(B), tree, B->start_line, B->end_line, TOKEN_MINOR(B));
    efree(B);
}

processing_stmt(A) ::= OPEN_TAG(B) top_stmt_list(C) . {
    DBG("meta_parser.y %d", __LINE__);
    zval *retv;
    zend_function *function;
    ALLOC_INIT_ZVAL(A);
    retv = zend_read_property(META_CLASS(node), C, STRL_PAIR("end_line")-1, 0 M_TSRMLS_CC);
    A = obj_call_method_internal_ex(A, META_CLASS(node), META_CLASS(node)->constructor, EG(scope), 1 M_TSRMLS_CC, "lzllz", NT_processing_stmt, tree, B->start_line, Z_LVAL_P(retv), TOKEN_MINOR(B));
    zend_hash_find(&META_CLASS(node)->function_table, STRL_PAIR("appendchild"), (void**) &function);
    if(NULL != B->next) {
        TOKEN *cursor, *prev;
        cursor = B->next;
        while(NULL != cursor) {
            retv = obj_call_method_internal_ex(A, META_CLASS(node), function, EG(scope), 1 M_TSRMLS_CC, "z", TOKEN_MINOR(cursor));
            zval_ptr_dtor(&retv);
            prev = cursor;
            cursor = cursor->next;
            efree(prev);
        }
    }
    retv = obj_call_method_internal_ex(A, META_CLASS(node), function, EG(scope), 1 M_TSRMLS_CC, "z", C);
    zval_ptr_dtor(&retv);
    efree(B);
}
processing_stmt(A) ::= OPEN_TAG(B) top_stmt_list(C) CLOSE_TAG(D) . {
    DBG("meta_parser.y %d", __LINE__);
    /* A B C D */
}


top_stmt_list(A) ::= top_stmt(B) . {
    zval *start_line, *end_line;
    DBG("meta_parser.y %d", __LINE__);
    zval *t_B = B;

    start_line = zend_read_property(META_CLASS(node), B, STRL_PAIR("start_line")-1, 0 M_TSRMLS_CC);
    end_line = zend_read_property(META_CLASS(node), B, STRL_PAIR("end_line")-1, 0 M_TSRMLS_CC);
    ALLOC_INIT_ZVAL(A);
    A = obj_call_method_internal_ex(A, META_CLASS(node), META_CLASS(node)->constructor, EG(scope), 1 M_TSRMLS_CC, "lzll", NT_top_stmt_list, tree, Z_LVAL_P(start_line), Z_LVAL_P(end_line));
    zend_function *appendChild;
    zend_hash_find(&META_CLASS(node)->function_table, STRL_PAIR("appendchild"), (void**) &appendChild);
    zval *retv = obj_call_method_internal_ex(A, META_CLASS(node), appendChild, EG(scope), 1 M_TSRMLS_CC, "z", B);
    zval_ptr_dtor(&retv);
}
top_stmt_list(A) ::= top_stmt_list(B) top_stmt(C) . {
    DBG("meta_parser.y %d", __LINE__);
    A = B;
    /* A B C */ // ADD C to B
}

top_stmt(A) ::= expr(B) . {
    DBG("meta_parser.y %d", __LINE__);
    A = B;
}

expr(A) ::= LNUMBER(B) . {
    DBG("meta_parser.y %d", __LINE__);
    TOKEN *t_B = B;
    ALLOC_INIT_ZVAL(A);
    A = obj_call_method_internal_ex(A, META_CLASS(node), META_CLASS(node)->constructor, EG(scope), 1 M_TSRMLS_CC, "lzllz", TOKEN_MAJOR(B), tree, B->start_line, B->end_line, TOKEN_MINOR(B));
    B->prev->next = NULL;
    B->next->prev = NULL;
    efree(B);
}

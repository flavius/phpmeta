%include {
#include <zend.h>
#include <php.h>
#include "meta_scanner.h"
#include "meta_parser.h"
#include "parser_API.h"
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
    php_printf("\t\t\ttoken dtor: %p\n", $$);
    //yeah, we shouldn't hide the warning
    if(tree == tree) {}
    //TODO we should ast_token_dtor($$)
    zval_ptr_dtor(&TOKEN_MINOR($$));
    efree($$);
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
%type top_stmt_list{zval*}

start(A) ::=  processing(B) . {
    //TODO instead of crafting nodes manually, use the tree as a factory,
    //which instantiates the right classes if the user has some specific preferences
    //META_ZDUMP(B);/* A B */
    zend_function *appendChild;
    zend_hash_find(&META_CLASS(tree)->function_table, STRL_PAIR("appendchild"), (void**) &appendChild);
    obj_call_method_internal_ex(tree, META_CLASS(node), appendChild, EG(scope), 0, 1 M_TSRMLS_CC, "z", B);
    //META_ZDUMP(B);
    //zval_ptr_dtor(&B);
}
processing(A) ::= OUTSIDE_SCRIPTING(B) . {
    DBG("line2 %d", __LINE__);
    ALLOC_INIT_ZVAL(A);
    A = obj_call_method_internal_ex(A, META_CLASS(node), META_CLASS(node)->constructor, EG(scope), 1, 1 M_TSRMLS_CC,
        "lzzll", TOKEN_MAJOR(B), TOKEN_MINOR(B), tree, B->start_line, B->end_line);
    efree(B);
    //META_ZDUMP(A);
}
processing(A) ::= OPEN_TAG(B) top_stmt_list(C) . {
    DBG("meta_parser.y %d", __LINE__);
    //------------ create the processing node
    ALLOC_INIT_ZVAL(A);
    //TODO make obj_call_method_internal_ex turn a NULL into a IS_NULL zval* internally, so we don't have to create data it manually
    zval *data;
    ALLOC_INIT_ZVAL(data);
    zval *stmt_endline = zend_read_property(META_CLASS(node), C, STRL_PAIR("end_line")-1, 0 TSRMLS_CC);
    A = obj_call_method_internal_ex(A, META_CLASS(node), META_CLASS(node)->constructor, EG(scope), 1, 1 M_TSRMLS_CC, "lzzll", NT_processing, data, tree, B->start_line, Z_LVAL_P(stmt_endline));
    zval_ptr_dtor(&data);
    //------------- create the OPEN_TAG node
    zval *open_tag;
    ALLOC_INIT_ZVAL(open_tag);
    open_tag = obj_call_method_internal_ex(open_tag, META_CLASS(node), META_CLASS(node)->constructor, EG(scope), 1, 1 M_TSRMLS_CC, "lzzll", TOKEN_MAJOR(B), TOKEN_MINOR(B), tree, B->start_line, B->end_line);
    efree(B);
    //------------- add children open_tag and C to A
    zend_function *setparent;
    if(FAILURE == zend_hash_find(&META_CLASS(node)->function_table, STRL_PAIR("setparentnode"), (void**) &setparent)) {
        DBG("FAIL %d", __LINE__);
    }
    obj_call_method_internal_ex(open_tag, META_CLASS(node), setparent, META_CLASS(node), 0, 1 M_TSRMLS_CC, "z", A);
    obj_call_method_internal_ex(C, META_CLASS(node), setparent, META_CLASS(node), 0, 1 M_TSRMLS_CC, "z", A);

    //META_ZDUMP(C);
        /* A B C */
}
processing(A) ::= OPEN_TAG(B) top_stmt_list(C) CLOSE_TAG(D) . { /* A B C D */ DBG("line3 %d", __LINE__); }

top_stmt_list(A) ::= LNUMBER(B) . {
    DBG("meta_parser.y %d", __LINE__);
    ALLOC_INIT_ZVAL(A);
    A = obj_call_method_internal_ex(A, META_CLASS(node), META_CLASS(node)->constructor, EG(scope), 1, 1 M_TSRMLS_CC, "lzzll", TOKEN_MAJOR(B), TOKEN_MINOR(B), tree, B->start_line, B->end_line);
    efree(B);
    /* A B */ DBG("line4 %d", __LINE__);
}

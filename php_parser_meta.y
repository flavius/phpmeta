%include {

//TODO modify lemon to always include yyTokenName, as well as the indexes where terminals/nonterminals start/end
#ifndef NDEBUG

#include <assert.h>
#include "php_parser_meta.h"

static const char *const yyTokenName[];
const char* meta_token_repr(int n) {
    if(n > T_INTERNAL_SKIP) {
        return "UNKNOWN";
    }
    return yyTokenName[n];
}

#endif
}
%start_symbol start
%token_prefix T_


%left QUESTION_MARK COLON.
%left PLUS MINUS DOT.
%left MULT DIV MOD.

%nonassoc LNUMBER.

// dummy tokens
%nonassoc OPEN_TAG OUTSIDE_SCRIPTING WHITESPACE COMMENT CLOSE_TAG DOC_COMMENT NEWLINE.
// must be last
%nonassoc INTERNAL_SKIP.


start ::= top_statement_list. 
top_statement_list ::= top_statement_list top_statement.
top_statement ::= statement.
statement ::= unticked_statement.
unticked_statement ::= expr COLON.

expr ::= scalar.
expr ::= expr PLUS expr.
           
scalar ::= common_scalar.
common_scalar ::= LNUMBER.

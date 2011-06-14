#ifndef META_PARSER_H
#define META_PARSER_H

//NEVER include the defs directly, always do it by including THIS file
#include "meta_parser_defs.h"
#include "meta_scanner.h"

typedef struct AstNode {
    int type;
    zval* children;
} AstNode;

typedef struct AstTree {
    //TODO pointers to classes, functions-nodes
    TOKEN** children;
} AstTree;

//the parser interface
void *MetaParserAlloc(void *(*mallocProc)(size_t));
void MetaParserFree(void *p, void (*freeProc)(void*) );
void MetaParser( void *yyp,int yymajor,TOKEN* minor,AstTree *tree);

//logistically it belongs to the scanner
const char* meta_token_repr(int n);

//tree functions
AstTree* meta_tree_ctor(void);

#endif // META_PARSER_H

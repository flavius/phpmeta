#ifndef META_PARSER_H
#define META_PARSER_H

//NEVER include the defs directly, always do it by including THIS file
#include "meta_parser_defs.h"
#include "meta_scanner.h"

//TODO replace the AstNode and AstTree by classes in parser.c/h
typedef struct AstNode {
    int type;
    zval* terminals;
} AstNode;

typedef struct AstTree {
    //TODO pointers to classes, functions-nodes
    TOKEN** children;
} AstTree;

//the parser interface
//TODO write a wrapper for it and expose it instead
void *MetaParserAlloc(void *(*mallocProc)(size_t));
void MetaParserFree(void *p, void (*freeProc)(void*) );
void MetaParser( void *yyp,int yymajor,TOKEN* minor,zval *tree);

//logistically it belongs to the scanner
const char* meta_token_repr(int n);

#endif // META_PARSER_H

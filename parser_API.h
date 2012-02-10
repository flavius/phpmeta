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

#ifndef PARSER_API_H
#define PARSER_API_H
/* never include the following directly, always use THIS file */
#include "meta_parser.h"
/* TODO never include meta_parser_defs.h directly, do it through this file */

/* we need pointers to CST nodes when we want to construct the CST
 * and all we've got in the grammar rule are non-terminals;
 * the problem is solved when we have terminals by making TOKEN a
 * doubly-linked list */
typedef struct _meta_node {
    zend_object std;
    TOKEN *follow;
} MetaNode;

#define METANODE_FOLLOW(pnode) ((pnode)->follow)

MetaNode* metanode_ctor();
void metanode_dtor(MetaNode*);

#endif /* PARSER_API_H */

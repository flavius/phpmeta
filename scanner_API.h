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

#ifndef SCANNER_API_H
# define SCANNER_API_H
#include <zend.h>
#include "meta_scanner.h"

extern const unsigned int meta_scanner_maxfill;

META_API zval* meta_token_zval(TOKEN *token);
META_API void meta_token_zval_ex(TOKEN *token, zval *tok_repr);
META_API void meta_scanner_free(meta_scanner **scanner);

#define META_TOK_CHAIN_GO_LEFT          1
#define META_TOK_CHAIN_GO_RIGHT         2
#define META_TOK_CHAIN_DEEPFREE_LEFT    4
#define META_TOK_CHAIN_DEEPFREE_RIGHT   8
#define META_TOK_CHAIN_FREESELF         16
#define META_TOK_CHAIN_FREESELF_DEEP    32

META_API void meta_token_dtor(TOKEN** start, unsigned int flags, void* leftlimit, void* rightlimit);
//META_API void meta_token_dtor(TOKEN** t, zend_bool deep);
META_API zval* meta_scanner_token_zval(TOKEN* t);
META_API meta_scanner* meta_scanner_alloc(zval* rawsrc, long flags);

#endif /* SCANNER_API_H */

#ifndef SCANNER_API_H
# define SCANNER_API_H
#include <zend.h>
#include "meta_scanner.h"

extern const unsigned int meta_scanner_maxfill;

META_API zval* meta_token_zval(TOKEN *token);
META_API void meta_token_zval_ex(TOKEN *token, zval *tok_repr);
META_API void meta_scanner_free(meta_scanner **scanner);
META_API void meta_token_dtor(TOKEN *tok);
META_API zval* meta_scanner_token_zval(TOKEN* t);
META_API meta_scanner* meta_scanner_alloc(zval* rawsrc, long flags);

#endif // SCANNER_API_H

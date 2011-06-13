#ifndef SCANNER_H
#define SCANNER_H
#include <zend.h>
#include "php_meta.h"

#define PHP_META_SCANNER_DESCRIPTOR_RES_NAME "Meta Scanner"
extern int meta_scanner_descriptor;

PHP_FUNCTION(meta_scanner_init);
PHP_FUNCTION(meta_scanner_get);
PHP_FUNCTION(meta_scanner_token_name);

#endif // SCANNER_H

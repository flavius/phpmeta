#ifndef SCANNER_H
#define SCANNER_H
#include <zend.h>
#include "php_meta.h"

//TODO eventually turn the scanner into a class
#define PHP_META_SCANNER_DESCRIPTOR_RES_NAME "Meta Scanner"
extern int meta_scanner_descriptor;

PHP_FUNCTION(meta_scanner_init);
PHP_FUNCTION(meta_scanner_get);
PHP_FUNCTION(meta_scanner_token_name);
void php_meta_scanner_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

#endif // SCANNER_H

#ifndef META_SCANNER_H
#define META_SCANNER_H

#include <zend.h>

typedef struct _ast_token {
	int major;
	zval *minor;
} ast_token;

typedef struct _meta_scanner {
	//zval* rawsrc;
	char *src;
	size_t src_len;
	char* marker;
	char* cursor;
	char* limit;
	int state;
	zend_bool free_raw;
	//if stream-based source, the position of cursor
	int position;
	//sometimes the scanner looks too far ahead and
	//when it does so, it caches the previous tokens
	ulong* buffer_majors;
	zval** buffer_minors;
	size_t buffer_size;
	//TODO add streams
	//TODO add TSRM
	//TODO different flags as to process comments or whitespaces, etc
} meta_scanner;


meta_scanner* meta_scanner_alloc(zval*);
int meta_scan(meta_scanner* scanner, zval** minor TSRMLS_DC);
void meta_scanner_destroy(meta_scanner** scanner);
int meta_scanner_pushtoken(meta_scanner* scanner, ulong major, zval* minor);
zval* meta_scanner_poptoken(meta_scanner* scanner, ulong* major);
#endif
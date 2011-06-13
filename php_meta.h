#ifndef PHP_META_H
#define PHP_META_H

#define PHP_META_EXTNAME "meta"
#define PHP_META_EXTVER "0.1"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

extern zend_module_entry meta_module_entry;
#define phpext_meta_ptr &meta_module_entry

//internal API of the scanner and the parser are marked as such
#define META_API

//TODO here or in the internal parser?
void *meta_alloc(size_t size);
void meta_free(void* ptr);

#endif //PHP_META_H

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

#include <zend.h>
#include "php_meta.h"
#include "parser_API.h"
#include "parser.h"

/* see parser_API.h */

MetaNode* metanode_ctor() {
	MetaNode* node;
	node = emalloc(sizeof(MetaNode));
	METANODE_FOLLOW(node) = NULL;
	return node;
}
void metanode_dtor(MetaNode* meta_obj) {
	/* TODO deep free */
	if(meta_obj->follow) {
		efree(meta_obj->follow);
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 */

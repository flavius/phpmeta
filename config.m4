CFLAGS="$CFLAGS -Wall -Wextra -Wno-unused -g3 -DDEBUG"
PHP_ARG_ENABLE(meta,
	[Whether to enable the "meta" extension],
	[  --enable-meta Enable "meta" extension support])

if test $PHP_META != "no"; then
	PHP_SUBST(META_SHARED_LIBADD)
	PHP_NEW_EXTENSION(meta, meta.c php_parser.c meta_scanner.c, $ext_shared)
	PHP_ADD_MAKEFILE_FRAGMENT
fi

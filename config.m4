CFLAGS="$CFLAGS -Wall -Wextra -Wno-unused"
PHP_ARG_ENABLE(meta,
	[Whether to enable the "meta" extension],
	[  --enable-meta Enable "meta" extension support])

if test $PHP_META != "no"; then
	PHP_SUBST(META_SHARED_LIBADD)
	PHP_NEW_EXTENSION(meta, meta.c php_scanner.c php_parser.c, $ext_shared)

	dnl we need an empty line
	PHP_ADD_MAKEFILE_FRAGMENT
fi

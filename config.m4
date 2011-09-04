dnl TODO remove these
dnl CFLAGS="$CFLAGS -Wall -Wextra -Werror -Wno-unused -g3 -DDEBUG"
dnl CFLAGS="$CFLAGS -Wall -Wextra -Wshadow -Wpointer-arith -Wstrict-prototypes -Wno-unused-parameter -g3"
CFLAGS="$CFLAGS -Wall -g3 -Wdeclaration-after-statement"
PHP_ARG_ENABLE(meta,
	[Whether to enable the "meta" extension],
	[  --enable-meta Enable "meta" extension support])

if test $PHP_META != "no"; then
	PHP_SUBST(META_SHARED_LIBADD)
    PHP_NEW_EXTENSION(meta, meta.c scanner.c parser.c meta_scanner.c meta_parser.c scanner_API.c parser_API.c, $ext_shared)
dnl TODO allow out-of-srctree building, for this we need to duplicate
dnl a lot from AC_DEFUN([PHP_NEW_EXTENSION]) php-src/acinclude.m4
dnl the following won't simply work for all cases (static, shared, cli, pecl):
dnl    PHP_ADD_SOURCES(PHP_EXT_BUILDDIR(meta), meta_scanner.c meta_parser.c)
	PHP_ADD_MAKEFILE_FRAGMENT
fi

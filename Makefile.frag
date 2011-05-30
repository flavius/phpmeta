
RE2C_FLAGS=--no-generation-date --case-inverted -cbdF

$(srcdir)/meta.c: $(srcdir)/php_parser.h

$(srcdir)/php_scanner.c: $(srcdir)/php_scanner.re $(srcdir)/php_parser.h
	$(RE2C) $(RE2C_FLAGS) -t php_scanner_defs.h -o php_scanner.c php_scanner.re

#$(srcdir)/php_parser.c $(srcdir)/php_parser.h: $(srcdir)/lemon $(srcdir)/php_parser.y

$(srcdir)/php_parser.c $(srcdir)/php_parser.h: $(srcdir)/lemon
	@(cd $(srcdir); ./lemon -q php_parser_meta.y; mv php_parser_meta.c php_parser.c)

$(srcdir)/lemon:
	$(CC) lemon.c -o lemon


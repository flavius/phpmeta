RE2C_FLAGS=--no-generation-date --case-inverted -cbdF

$(srcdir)/php_scanner.c: $(srcdir)/php_scanner.re $(srcdir)/php_parser.h
	$(RE2C) $(RE2C_FLAGS) -t php_scanner_defs.h -o php_scanner.c php_scanner.re
$(srcdir)/php_parser.c $(srcdir)/php_parser.h: $(srcdir)/lemon $(srcdir)/php_parser.y
	@(cd $(srcdir); ./lemon php_parser.y)
$(srcdir)/lemon:
	$(CC) lemon.c -o lemon
#this makefile is supposed to work(not 100% sure) with out-of-srctree builds, so it
#makes extensive use of $(builddir). However
#TODO config.m4 needs more tweaking

RE2C_FLAGS=--no-generation-date --case-inverted -cbdF

$(srcdir)/meta.c: $(srcdir)/scanner.c $(srcdir)/parser.c

$(srcdir)/scanner.c: $(builddir)/meta_scanner.c $(builddir)/meta_scanner_defs.h

$(srcdir)/parser.c: $(builddir)/meta_parser.c $(builddir)/meta_parser_defs.h

$(builddir)/meta_scanner.c $(builddir)/meta_scanner_defs.h: $(srcdir)/meta_scanner.re $(builddir)/meta_parser_defs.h
	$(RE2C) $(RE2C_FLAGS) -t $(builddir)/meta_scanner_defs.h -o $(builddir)/meta_scanner.c $(srcdir)/meta_scanner.re

$(builddir)/meta_parser_defs.h $(builddir)/meta_parser.c: $(builddir)/lemon
	$(builddir)/lemon -q T=$(srcdir)/lempar.c u=meta_parser_defs.h $(srcdir)/meta_parser.y

$(builddir)/lemon:
	$(CC) $(srcdir)/lemon.c -o $(builddir)/lemon

internalclean:clean
	rm -f $(builddir)/meta_scanner_defs.h $(builddir)/meta_scanner.c
	rm -f $(builddir)/{lemon}
	rm -f $(builddir)/{meta_parser.c,meta_parser_defs.h}


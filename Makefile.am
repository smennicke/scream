SUBDIRS=libs/pnapi src tests

EXTRA_DIST = utils tests

dist-clean: clean
	-@rm -rf Makefile autom4te.cache
	-@rm -rf compile config.log config.status
	-@rm -rf configure
	-@rm -rf libs/pnapi/Makefile
	-@rm -rf libs/pnapi/config.pnapi.h
	-@rm -rf src/.deps
	-@rm -rf src/Makefile src/Makefile.in
	-@rm -rf src/config.h.in src/config.h.in~
	-@rm -rf src/stamp-h1

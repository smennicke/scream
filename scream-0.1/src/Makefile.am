bin_PROGRAMS=scream

scream_SOURCES=splot-parser.yy splot-scanner.ll \
	main.cpp \
	Feature.h Feature.cpp \
	FeatureGroup.h FeatureGroup.cpp \
	FeatureModel.h FeatureModel.cpp \
	Stage.h Stage.cpp \
	StageModel.h StageModel.cpp \
	types.h tools.h tools.cpp \
	cmdline.h cmdline.c

BUILT_SOURCES = cmdline.c splot-scanner.cc
EXTRA_DIST = splot-scanner.cc splot-parser.h splot-parser.cc cmdline.ggo cmdline.h cmdline.c
CLEANFILES =

scream_CPPFLAGS = -I$(top_srcdir)/libs -I$(top_srcdir)/libs/pnapi
scream_LDADD = $(top_builddir)/libs/pnapi/libpnapi.a

################################################################
# use GNU gengetopt to create command line parser
################################################################
cmdline.c: cmdline.ggo
	$(GENGETOPT) --input=cmdline.ggo


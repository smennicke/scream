SCREAM = scream
BASEPATH = "" 
TIME = time

SPLX_FILES = $(wildcard *.splx)
SCREAM_FILES = $(SPLX_FILES:.splx=.splxx)
ONCE = $(SCREAM_FILES:.splxx=.once)
TRY_ALL = $(SCREAM_FILES:.splxx=.all)

all: $(SCREAM_FILES) $(ONCE)

clean:
	-@rm *.scream *.log *.lola *.result *.task *.once *.all

%.splxx: %.splx
	-@sed -n "/<feature_tree>/,/<\/constraints>/p" $< | sed -e "s/<\/meta>//" > $@

%.once: %.splxx
	-@$(TIME) -o$@ $(SCREAM) --basepath=$(BASEPATH) -r -l $<

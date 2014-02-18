# flcli -v 1d50:602b:0003 -l 0:sample4.dat
# make waves SIGS="TCK TDO TDI TMS"

SIGS := SS SCK MOSI MISO

all: raw2fst

raw2fst: raw2fst.c libfstapi.a
	gcc -O2 -o $@ $+ -lz

libfstapi.a: gtkwave-3.3.34.tar.gz
	rm -rf gtkwave-3.3.34 fstapi.h libfstapi.a && \
	tar zxf gtkwave-3.3.34.tar.gz && \
	cd gtkwave-3.3.34 && \
	./configure --with-tcl=/usr/lib/tcl8.4 --with-tk=/usr/lib/tk8.4 && \
	cd src/helpers && \
	make fstapi.o fastlz.o && \
	ar cr ../../../libfstapi.a fstapi.o fastlz.o && \
	cp fst/fstapi.h ../../../fstapi.h && \
	cd ../../.. && \
	rm -rf gtkwave-3.3.34

gtkwave-3.3.34.tar.gz:
	wget http://gtkwave.sourceforge.net/gtkwave-3.3.34.tar.gz

waves: raw2fst startup.tcl sample4.dat
	./raw2fst sample4.dat sample4.fst $(SIGS)
	gtkwave -T startup.tcl sample4.fst

startup.tcl:
	printf 'set signalList [list]\nlappend signalList "a"\nlappend signalList "b"\nset num_added [ gtkwave::addSignalsFromList $$signalList ]\ngtkwave::setZoomFactor -21\ngtkwave::setMarker 23ns\n' > $@

clean: FORCE
	rm -rf gtkwave-3.3.34.tar.gz gtkwave-3.3.34 fstapi.h libfstapi.a raw2fst *.fst startup.tcl

FORCE:

/* 
 * Copyright (C) 2013 Chris McClelland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Compile like this:
 *   gcc -O2 -o raw2fst4 raw2fst4.c libfstapi.a -lz
 *
 * This uses the FST API library from GTKWave; I built it like this:
 *   wget http://gtkwave.sourceforge.net/gtkwave-3.3.34.tar.gz
 *   tar zxf gtkwave-3.3.34.tar.gz
 *   cd gtkwave-3.3.34
 *   ./configure --with-tcl=/usr/lib/tcl8.4 --with-tk=/usr/lib/tk8.4
 *   cd src/helpers
 *   make fstapi.o fastlz.o
 *   ar cr ../../../libfstapi.a fstapi.o fastlz.o
 *   cp fst/fstapi.h ../../../fstapi.h
 *   cd ../../..
 *   rm -rf gtkwave-3.3.34
 */
#include <stdio.h>
#include <stdlib.h>
#include "fstapi.h"

int main(int argc, const char *argv[]) {
	struct fstContext *ctx;
	fstHandle vara, varb, varc, vard;
	const char * const ZERO = "0";
	const char * const ONE = "1";
	const char *const rawFile = argv[1];
	const char *const fstFile = argv[2];
	const char *const *sig = argv + 3;
	FILE *raw;
	unsigned char byte;
	unsigned char block[1024*1024];
	const unsigned char *ptr;
	const char *olda = NULL, *oldb = NULL, *oldc = NULL, *oldd = NULL;
	const char *newa, *newb, *newc, *newd;
	uint64_t time = 0;
	size_t bytesRead;
	FILE *tclFile;
	if ( argc != 7 ) {
		fprintf(stderr, "Synopsis: %s <input.raw> <output.fst> <P1> <P2> <P5> <P6>\n", argv[0]);
		exit(1);
	}
	raw = fopen(rawFile, "rb");
	if ( !raw ) {
		fprintf(stderr, "Unable to open %s\n", rawFile);
		exit(2);
	}
	ctx = fstWriterCreate(fstFile, 1);
	fstWriterSetPackType(ctx, 0);
	fstWriterSetRepackOnClose(ctx, 0);
	fstWriterSetVersion(ctx, "MSLAv0.1");
	fstWriterSetTimescale(ctx, -10);

	vara = fstWriterCreateVar(ctx, FST_VT_VCD_REG, FST_VD_IMPLICIT, 1, sig[0], 0);
	varb = fstWriterCreateVar(ctx, FST_VT_VCD_REG, FST_VD_IMPLICIT, 1, sig[1], 0);
	varc = fstWriterCreateVar(ctx, FST_VT_VCD_REG, FST_VD_IMPLICIT, 1, sig[2], 0);
	vard = fstWriterCreateVar(ctx, FST_VT_VCD_REG, FST_VD_IMPLICIT, 1, sig[3], 0);

	bytesRead = fread(block, 1, 1024*1024, raw);
	while ( bytesRead ) {
		ptr = block;
		while ( bytesRead-- ) {
			byte = *ptr++;
			//printf("%02X\n", byte);
						
			newa = (byte & 0x10) ? ONE : ZERO;
			newb = (byte & 0x20) ? ONE : ZERO;
			newc = (byte & 0x40) ? ONE : ZERO;
			newd = (byte & 0x80) ? ONE : ZERO;
			if ( newa != olda ) {
				fstWriterEmitTimeChange(ctx, time);
				fstWriterEmitValueChange(ctx, vara, newa);
				olda = newa;
			}
			if ( newb != oldb ) {
				fstWriterEmitTimeChange(ctx, time);
				fstWriterEmitValueChange(ctx, varb, newb);
				oldb = newb;
			}
			if ( newc != oldc ) {
				fstWriterEmitTimeChange(ctx, time);
				fstWriterEmitValueChange(ctx, varc, newc);
				oldc = newc;
			}
			if ( newd != oldd ) {
				fstWriterEmitTimeChange(ctx, time);
				fstWriterEmitValueChange(ctx, vard, newd);
				oldd = newd;
			}
			time += 208;

			newa = (byte & 0x01) ? ONE : ZERO;
			newb = (byte & 0x02) ? ONE : ZERO;
			newc = (byte & 0x04) ? ONE : ZERO;
			newd = (byte & 0x08) ? ONE : ZERO;
			if ( newa != olda ) {
				fstWriterEmitTimeChange(ctx, time);
				fstWriterEmitValueChange(ctx, vara, newa);
				olda = newa;
			}
			if ( newb != oldb ) {
				fstWriterEmitTimeChange(ctx, time);
				fstWriterEmitValueChange(ctx, varb, newb);
				oldb = newb;
			}
			if ( newc != oldc ) {
				fstWriterEmitTimeChange(ctx, time);
				fstWriterEmitValueChange(ctx, varc, newc);
				oldc = newc;
			}
			if ( newd != oldd ) {
				fstWriterEmitTimeChange(ctx, time);
				fstWriterEmitValueChange(ctx, vard, newd);
				oldd = newd;
			}
			time += 208;
		}
		bytesRead = fread(block, 1, 1024*1024, raw);
	}
	fstWriterEmitTimeChange(ctx, time);
	fstWriterClose(ctx);
	tclFile = fopen("startup.tcl", "w");
	fprintf(tclFile, "gtkwave::addSignalsFromList %s\n", sig[0]);
	fprintf(tclFile, "gtkwave::addSignalsFromList %s\n", sig[1]);
	fprintf(tclFile, "gtkwave::addSignalsFromList %s\n", sig[2]);
	fprintf(tclFile, "gtkwave::addSignalsFromList %s\n", sig[3]);
	fprintf(tclFile, "gtkwave::setZoomFactor -40\n");
	fprintf(tclFile, "for { set i 0 } { $i <= [ gtkwave::getVisibleNumTraces ] } { incr i } { gtkwave::setTraceHighlightFromIndex $i off }\n");
	fclose(tclFile);
	return 0;
}

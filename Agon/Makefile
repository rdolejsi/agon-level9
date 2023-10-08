# Makefile for Level9, Agon Light 8-bit computer port.
# The build uses 24-bit mode ez80 LLVM-based compiler suite AgDev based on CE C/C++ Toolset.
#
# Please note: Agon CEdev environment always grabs sources from single directory
# We need to link main shared sources to local directory to ensure compilation, unfortunately.
# This is performed by 'make clean', which thus needs to be run prior the first 'make'.

NAME = level9
DESCRIPTION = "Level 9 Interpreter"
COMPRESSED = NO

BINDIR = dist

BSSHEAP_LOW = 054000
BSSHEAP_HIGH = 0AAFFF
STACK_HIGH = 0AFFFF
INIT_LOC = 040000

# Compilation flags
# Please note: -Oz does not work on Agon/CEdev at the moment:
# LLVM fails at legalizing instructions within CheckCallDriverV4 method
CFLAGS = -Wall -Wextra -I.. -DBITMAP_DECODER -DHAVE_PLATFORM_H -D__AGON__ -v # -DGFX_ENABLED -DGFX_DEBUG #-Oz #-DL9DEBUG

LIBS = -ltermcap

OBJS = level9.o bitmap.o

SRCDIR=.

include $(shell cedev-config --makefile)

##################################################################

BASE_SOURCES = ../level9.c ../level9.h ../bitmap.c

clean:
	rm -f $(OBJS) *~ obj/*~ obj/*.c.d obj/*.c.bc obj/lto.bc obj/*.src dist/*.bin
	ln -sfr $(BASE_SOURCES) .

agon.o: agon.c
	$(CC) $(CFLAGS) -s -o $@ -c $<

bitmap.o: ../bitmap.c
	$(CC) $(CFLAGS) -s -o $@ -c $<

level9.o: ../level9.c
	$(CC) $(CFLAGS) -s -o $@ -c $<

#level9: $(OBJS)
#	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

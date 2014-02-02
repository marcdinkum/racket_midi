#
# Makefile for Racket C extensions for MIDI I/O
#
# Marc Groenewegen 2014
#
# N.B. we're using C++ bindings which has these consequences:
# - C++ files must have .cpp as extension
# - we need to link libstdc++ to be able to use cout etc...
# - we can't use raco for C++ so we need to compile everything with mzc
#   and put garbage collector stuff into our code (something that raco
#   possibly would have done for us, if I understand correctly)
#

CC = g++
CFLAGS = -Wall -I${RACKET_HOME}/include
LDFLAGS = -L/usr/local/lib -lportmidi -lpthread -lstdc++


# determine OS
# For Linux:
# - shared libs have extension so
# - module path is compiled/native/x86_64-linux/3m
# For OSX (Darwin):
# - shared libs have extension dylib
# - module path is compiled/native/i386-macosx/3m

UNAME=$(shell uname)

# machine name, i386 (32-bit), x86_64 (64-bit)
# NB: don't put this comment on the next line as it will add a SPACE
#  to ARCH (!)
# Actually we won't be using this as we need Racket's architecure type
ARCH=$(shell uname -m)

# derive Racket architecture by evaluating a command in $RACKET_HOME/bin/racket
RACKET_ARCH=$(shell ./tell64bitracket.sh)

ifeq ($(UNAME),Linux)
 LIBEXT=so
 MODPATH=compiled/native/${RACKET_ARCH}-linux/3m
endif

ifeq ($(UNAME),Darwin)
 LIBEXT=dylib
 MODPATH=compiled/native/${RACKET_ARCH}-macosx/3m
endif

# ---------------------------

all: midi_extension.so

midi_extension.so: midi_extension.cpp midi_io.cpp
	${RACKET_HOME}/bin/mzc --cc midi_extension.cpp
	${RACKET_HOME}/bin/mzc --cc midi_io.cpp
	# put the shared object in a 'known' place for module extensions
	mkdir -p ${MODPATH}
	${RACKET_HOME}/bin/mzc --3m --ld \
	  ${MODPATH}/midi_extension_rkt.${LIBEXT} midi_extension.o midi_io.o ${LDFLAGS}

clean:
	rm -f *.o ${MODPATH}/midi_extension_rkt.${LIBEXT}


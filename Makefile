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
LDFLAGS = -lportmidi -lpthread -lstdc++


all: midi_extension.so

midi_extension.so: midi_extension.cpp midi_io.cpp
	${RACKET_HOME}/bin/mzc --cc midi_extension.cpp
	${RACKET_HOME}/bin/mzc --cc midi_io.cpp
	# put the shared object in a 'known' place for module extensions
	mkdir -p compiled/native/x86_64-linux/3m
	${RACKET_HOME}/bin/mzc --3m --ld \
	  compiled/native/x86_64-linux/3m/midi_extension_rkt.so \
	  midi_extension.o midi_io.o ${LDFLAGS}

clean:
	rm -f *.o compiled/native/x86_64-linux/3m/racket_extension_rkt.so
	rm -f `find . -perm +111 -type f`


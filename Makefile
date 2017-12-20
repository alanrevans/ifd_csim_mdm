# Muscle Smartcard Development
# Makefile
# David Corcoran

CC       = cc
CFLAGS   = -g -fpic -w -DCPU_ICAP_PC
LD       = ld
LEX      = flex
OBJ     := $(filter-out test.o, $(patsubst %.c,%.o,$(wildcard *.c)))
INCLUDE  = -I.

MAKEXE   = make
LIBNAME  = libcsim_mdm.so
PREFIX   = /usr/local/pcsc

DEFS     = -DPCSC_DEBUG=1 #-DATR_DEBUG=1

all: unix

clean:
	rm -f *.o $(LIBNAME) core

osx: $(OBJ)
	$(CC) -dynamiclib $(OBJ) -o $(LIBNAME)

unix: $(OBJ)
	$(LD) -shared $(OBJ) -o $(LIBNAME)

$(patsubst %.c,%.o,$(wildcard *.c)) : %.o : %.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(DEFS)


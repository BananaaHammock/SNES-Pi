# Generated automatically from Makefile.in by configure.
CC=gcc
LD=$(CC)

CFLAGS=-g -lwiringPi -Wall -DVERSION_STR=\"1.02\" -DPROGRESS_SPINNER -O3
LDFLAGS= -lwiringPi -O3

PROG=apuplay
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
OBJS_XMMS = $(patsubst %.c,%.fpic.o,$(wildcard *.c)) xmms.o

all: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

xmms: $(OBJS_XMMS)
	$(LD) $(OBJS_XMMS) -shared `xmms-config --libs` -o hwapu.so

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

%.fpic.o: %.c %.h
	$(CC) $(CFLAGS) `xmms-config --cflags` -fPIC -c $< -o $@
	
%.fpic.o: %.c 
	$(CC) $(CFLAGS) `xmms-config --cflags` -fPIC -c $< -o $@

xmms.o: xmms/xmms.c
	$(CC) $(CFLAGS) `xmms-config --cflags` -fPIC -c $< -o $@

clean:
	rm -f *.o hwapu.so $(PROG)

distclean:
	rm -rf *.o $(PROG) autom4te.cache config.cache config.status config.log Makefile config.h

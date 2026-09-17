# Makefile for compiling NSGA-II source code
CC=gcc
LD=gcc
RM=rm -f
CFLAGS=-Wall -pedantic -g -O2 -D_GNU_SOURCE
OBJS:=$(patsubst %.c,%.o,$(wildcard *.c))
MAIN=nsga2r
all:$(MAIN)
$(MAIN):$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(MAIN) -lm
%.o: %.c global.h rand.h
	$(CC) $(CFLAGS) -c $<
clean:
	$(RM) $(OBJS)


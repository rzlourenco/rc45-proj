.PHONY: all clean

OUTDIR  := bin

CC      ?= gcc
CFLAGS  := -std=c99 -Wall -Wextra -pedantic -D_BSD_SOURCE 
LDFLAGS :=

all: user central storage

-include $(wildcard *.d)

%.o: %.c
	$(CC) -MMD $(CFLAGS) -c $< -o $@

user: user.o common.o
	@-mkdir -p $(OUTDIR)
	$(CC) $(LDFLAGS) $? -o $(OUTDIR)/$@

central: central.o common.o
	@-mkdir -p $(OUTDIR)
	$(CC) $(LDFLAGS) $? -o $(OUTDIR)/$@

storage: storage.o common.o
	@-mkdir -p $(OUTDIR)
	$(CC) $(LDFLAGS) $? -o $(OUTDIR)/$@

clean:
	@-rm -f $(OUTDIR)/* *.o *.d


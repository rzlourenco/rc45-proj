.PHONY: all clean

CLIENT_SRCS := $(wildcard user/*.c)
CLIENT_OBJS := $(CLIENT_SRCS:%.c=%.o)
OUTDIR      := bin

CC      := gcc
# _POSIX_C_SOURCE is defined to shut up the warnings about some POSIX
# functions not being defined in unistd.h
CFLAGS  := -std=c99 -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200112L
LDFLAGS :=

all: user

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

user: $(CLIENT_OBJS)
	@-mkdir -p $(OUTDIR)
	$(CC) $(LDFLAGS) $(CLIENT_OBJS) -o $(OUTDIR)/$@

clean:
	@-rm -f $(OUTDIR)/* $(CLIENT_OBJS)


.PHONY: all clean

CLIENT_SRCS := $(wildcard user/*.c)
CLIENT_OBJS := $(CLIENT_SRCS:%.c=%.o)
OUTDIR      := bin

CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -pedantic -D_BSD_SOURCE -DNG=10
LDFLAGS :=

all: user

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

user: $(CLIENT_OBJS)
	@-mkdir -p $(OUTDIR)
	$(CC) $(LDFLAGS) $(CLIENT_OBJS) -o $(OUTDIR)/$@

clean:
	@-rm -f $(OUTDIR)/* $(CLIENT_OBJS)


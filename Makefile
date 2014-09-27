.PHONY: all clean

OUTDIR  := bin
OBJDIR  := .obj

$(shell mkdir -p $(OBJDIR) $(OUTDIR))

CC      ?= gcc
CFLAGS  := -std=c99 -Wall -Wextra -pedantic -D_BSD_SOURCE -g 
LDFLAGS :=

all: user central storage

-include $(wildcard $(OBJDIR)/*.d)

$(OBJDIR)/%.o: %.c
	$(CC) -MMD $(CFLAGS) -c $< -o $@

user: $(OBJDIR)/user.o $(OBJDIR)/common.o
	$(CC) $(LDFLAGS) $? -o $(OUTDIR)/$@

central: $(OBJDIR)/central.o $(OBJDIR)/common.o
	$(CC) $(LDFLAGS) $? -o $(OUTDIR)/$@

storage: $(OBJDIR)/storage.o $(OBJDIR)/common.o
	$(CC) $(LDFLAGS) $? -o $(OUTDIR)/$@

clean:
	@-rm -f $(OUTDIR)/* $(OBJDIR)/*


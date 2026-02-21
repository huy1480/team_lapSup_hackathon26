# Makefile for LS-HACK 2026

CC = gcc
CFLAGS = -Wall -g

BINS=hello

all: $(BINS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $?

clean:
	$(RM) $(BINS)
	$(RM) -r *.dSYM
	clear

submit:
	make clean
	git add .
	git commit -m "[LS-HACK] auto-submit"
	git push
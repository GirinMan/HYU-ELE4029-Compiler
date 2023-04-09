# Makefile for C-Minus Scanner
# ./lex/tiny.l --> ./cminus.l

CC = gcc 
FLEX = flex
CFLAGS = -W -Wall
FILE_REGREX = [/\.A-Za-z0-9]+

OBJS = main.o util.o scan.o
OBJS_LEX = main.o util.o lex.yy.o

.PHONY: all clean test_cimpl test_lex
all: cminus_cimpl cminus_lex

clean:
	-rm -vf cminus_cimpl cminus_lex *.o lex.yy.c

test_cimpl: cminus_cimpl
	./$< ./test.1.txt | diff result.1.txt - -I "C-MINUS COMPILATION: ${FILE_REGREX}"
	./$< ./test.2.txt | diff result.2.txt - -I "C-MINUS COMPILATION: ${FILE_REGREX}"
	./$< ./test.3.txt | diff result.3.txt - -I "C-MINUS COMPILATION: ${FILE_REGREX}"
	@echo "Success"

test_lex: cminus_lex
	./$< ./test.1.txt | diff result.1.txt - -I "C-MINUS COMPILATION: ${FILE_REGREX}"
	./$< ./test.2.txt | diff result.2.txt - -I "C-MINUS COMPILATION: ${FILE_REGREX}"
	./$< ./test.3.txt | diff result.3.txt - -I "C-MINUS COMPILATION: ${FILE_REGREX}"
	@echo "Success"

cminus_cimpl: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) 

cminus_lex: $(OBJS_LEX)
ifeq ($(shell uname), Darwin) # it only works when in OSX!
	$(CC) $(CFLAGS) -o $@ $(OBJS_LEX) -ll
else
	$(CC) $(CFLAGS) -o $@ $(OBJS_LEX) -lfl
endif

main.o: main.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c -o $@ $<

scan.o: scan.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c -o $@ $<

util.o: util.c globals.h util.h
	$(CC) $(CFLAGS) -c -o $@ $<

lex.yy.o: lex.yy.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c -o $@ $<

lex.yy.c: cminus.l
	$(FLEX) -o $@ $<


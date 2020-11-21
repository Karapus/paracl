$CFLAGS+=-O0 -Wall -Wextra -g
all: driver.out
driver.out: grammar.o lex.o inode.o driver.o
	g++ $? -o $@
driver.o: driver.cc
	g++ $(CFLAGS) -c $< -o $@
lex.o: lex.yy.cc grammar.tab.cc
	g++ $(CFLAGS) -c $< -o $@
lex.yy.cc: lexer.ll
	flex $<
grammar.tab.cc: grammar.yy
	bison -v -Wall -rall grammar.yy
grammar.o: grammar.tab.cc
	g++ $(CFLAGS) -c $< -o $@
inode.o: inode.cc inode.h
	g++ $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o *.out
	rm -f lex.yy.cc grammar.tab.* grammar.output location.hh
	rm -f vgcore.*

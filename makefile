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
	bison -dv -Wall -rall grammar.yy
grammar.o: grammar.tab.cc
	g++ $(CFLAGS) -c $< -o $@
inode.o: inode.cc inode.h
	g++ $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o
	rm -f lex.yy.cc grammar.tab.*

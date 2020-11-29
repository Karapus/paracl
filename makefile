$CFLAGS=-O0 -Wall -Wextra -g
all: driver.out
driver.out: driver.o lex.o grammar.o inode.o
	g++ $(CFLAGS) $? -o $@
driver.o: driver.cc driver.h grammar.tab.hh inode.h location.hh lexer.h exec.h
	g++ $(CFLAGS) -c $< -o $@
lex.o: lex.yy.cc grammar.tab.hh inode.h location.hh lexer.h
	g++ $(CFLAGS) -c $< -o $@
lex.yy.cc: lexer.ll
	flex $<
grammar.tab.hh: grammar.yy
	bison -v -Wall -rall grammar.yy
grammar.o: grammar.tab.cc grammar.tab.hh inode.h location.hh driver.h lexer.h exec.h
	g++ $(CFLAGS) -c $< -o $@
inode.o: inode.cc ast.h inode.h exec.h
	g++ $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o *.out
	rm -f lex.yy.cc grammar.tab.* grammar.output location.hh
	rm -f vgcore.*

CFLAGS= -O0 -Wall -Wextra -g -std=c++20
all: driver.out
driver.out: driver.o lex.o grammar.o inode.o ast.o
	g++ $(CFLAGS) $? -o $@
driver.o: driver.cc driver.h grammar.tab.hh inode.hh location.hh lexer.h exec.h
	g++ $(CFLAGS) -c $< -o $@
lex.o: lex.yy.cc grammar.tab.hh inode.hh location.hh lexer.h
	g++ $(CFLAGS) -c $< -o $@
lex.yy.cc: lexer.ll
	flex $<
grammar.tab.hh: grammar.yy
	bison -v -Wall -rall grammar.yy
grammar.o: grammar.tab.cc grammar.tab.hh inode.hh location.hh driver.h lexer.h exec.h
	g++ $(CFLAGS) -c $< -o $@
inode.o: inode.cc ast.hh inode.hh exec.h
	g++ $(CFLAGS) -c $< -o $@
ast.o: ast.cc ast.hh
	g++ $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o *.out
	rm -f lex.yy.cc grammar.tab.* grammar.output location.hh
	rm -f vgcore.*

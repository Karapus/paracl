$CFLAGS+=-Wall -Wextra
all: driver.out
driver.out: lex.o grammar.o driver.cc
	g++ $(CFLAGS) $? -o $@
lex.o: lex.yy.cc grammar.tab.cc
	g++ $(CFLAGS) -c $< -o $@
lex.yy.cc: lexer.ll
	flex $<
grammar.tab.cc: grammar.yy
	bison -dv -Wall -rall grammar.yy
grammar.o: grammar.tab.cc
	g++ $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o
	rm -f lex.yy.cc grammar.tab.*

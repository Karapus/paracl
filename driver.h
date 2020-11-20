#include "grammar.tab.hh"
#include "lexer.h"
#include "ast.h"
#include <iostream>

namespace yy {
class Driver final {
	public:
	Lexer lexer;
	AST::INode *yylval;
//	yy::location yyloc;
	Driver(std::istream *is) :
		lexer(is)
	{}
	AST::INode *parse() {
		yy::parser parser{*this};
		//parser.set_debug_level(trace_parsing);
		parser();
		return yylval;
	}
};
}

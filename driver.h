#include "grammar.tab.hh"
#include "lexer.h"
#include "ast.h"
#include <iostream>

namespace yy {
class Driver final {
	public:
	Lexer lexer;
	AST::INode *yylval;
	Driver(std::istream *is) : lexer(is), yylval(nullptr)
	{}
	AST::INode *parse() {
		yy::parser parser{*this};
		if (parser())
			return nullptr;
		return yylval;
	}
};
}

#include "grammar.tab.hh"
#include "lexer.h"
#include "exec.h"
#include <iostream>

namespace yy {
class Driver final {
	public:
	Lexer lexer;
	AST::INode *yylval;
	Driver(std::istream *is) : lexer(is), yylval(nullptr)
	{}
	AST::IExecable *parse() {
		yy::parser parser{*this};
		if (parser())
			return nullptr;
		return static_cast<AST::IExecable *>(yylval);
	}
};
}

#include "grammar.tab.hh"
#include "lexer.h"
#include "exec.h"
#include <iostream>

namespace yy {
struct Driver final {
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

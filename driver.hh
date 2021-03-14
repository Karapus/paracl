#include "grammar.tab.hh"
#include "inode.hh"
#include "lexer.hh"
#include "exec.hh"
#include <iostream>

namespace yy {
struct Driver final {
	Lexer lexer;
	AST::INode *yylval;
	Driver(std::istream *is) : lexer(is), yylval(nullptr)
	{}
	AST::INode *parse() {
		yy::parser parser{*this};
		if (parser())
			return nullptr;
		return static_cast<AST::INode *>(yylval);
	}
};
}

#include "grammar.tab.hh"
#include "lexer.h"
#include <iostream>

namespace yy {
class Driver final {
	Lexer lexer_;
	parser parser_;
	AST::INode *root_;
	public:
	Driver(std::istream *is) :
		lexer_(is),
		parser_(lexer_)
	{}
	AST::INode *parse() {
		parser_();
		return root_;
	}
};
}

int main() {
	yy::Driver driver{&std::cin};
	AST::INode *root = driver.parse();
}

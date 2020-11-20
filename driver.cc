#include "driver.h"

int main() {
	yy::Driver driver{&std::cin};
	AST::INode *root = driver.parse();
//	std::cout << typeid(*root).name() << std::endl;
	static_cast<AST::Scope *>(root)->exec();
}

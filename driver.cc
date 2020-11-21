#include "driver.h"
#include <fstream>

int main(int argc, char **argv) {
//	if (argc != 2)
//		return 0;
	std::ifstream code_file;
	code_file.open(argv[1]);
	yy::Driver driver{&code_file};
	AST::INode *root = driver.parse();
//	std::cout << typeid(*root).name() << std::endl;
	static_cast<AST::Scope *>(root)->exec();
}

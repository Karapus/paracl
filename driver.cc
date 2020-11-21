#include "driver.h"
#include <fstream>

int main(int argc, char **argv) {
	std::ifstream code_file;
	code_file.open(argv[1]);
	yy::Driver driver{&code_file};
	AST::INode *root = driver.parse();
	if (root)
		static_cast<AST::Scope *>(root)->exec();
	delete root;
}

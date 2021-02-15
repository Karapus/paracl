#include "driver.hh"
#include <fstream>

int main(int argc, char **argv) {
	std::ifstream code_file;
	code_file.open(argv[1]);
	yy::Driver driver{&code_file};
	auto root = driver.parse();
	if (root)
		root->exec();
	delete root;
}

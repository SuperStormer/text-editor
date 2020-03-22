#include <iostream>
#include <string>
#include <vector>

#include "editor.h"

int main(int argc, const char** argv) {
	if (argc < 2) {
		std::cerr << "Missing filename arg" << std::endl;
		exit(1);
	}
	std::string filename = argv[1];
	if (filename.find_first_of("\\/\f") != std::string::npos || filename[0] == '-') {
		std::cerr << "Invalid filename" << std::endl;
		exit(1);
	}
	std::vector<std::string> args{argv + 2, argv + argc};
	Editor editor{filename, args};
	editor.start();
}
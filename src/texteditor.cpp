#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

#include "editor.hpp"

int main(int argc, const char** argv) {
	if (argc < 2) {
		std::cerr << "Missing filename arg" << std::endl;
		return 1;
	}
	std::string filename = argv[1];
	std::ios_base::sync_with_stdio(false);
	std::vector<std::string> args{argv + 2, argv + argc};
	Editor editor{filename, args};
	editor.start();
	return 0;
}
#include <string>
#include <vector>

#include "editor.h"

int main(int argc, const char** argv) {
	std::string filename;
	std::vector<std::string> args;
	if (argc >= 2) {
		filename = argv[1];
		args = {argv + 2, argv + argc};
	} else {
		filename = "x.txt";
		args = {};
	}

	Editor editor{filename, args};
	editor.start();
}
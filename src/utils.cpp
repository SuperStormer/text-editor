#include "utils.hpp"

#include <string>
// from https://stackoverflow.com/a/24315631/7941251
std::string replace_all(std::string str, const std::string& search, const std::string& replace) {
	size_t start_pos = 0;
	while ((start_pos = str.find(search, start_pos)) != std::string::npos) {
		str.replace(start_pos, search.length(), replace);
		start_pos += replace.length();	// Handles case where 'search' is a substring of 'replace'
	}
	return str;
}
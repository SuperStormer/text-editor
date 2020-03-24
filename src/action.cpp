#include "action.hpp"

#include <memory>
#include <stdexcept>
#include <utility>
Action::Action(size_t line, size_t col, std::vector<std::string> lines) : line{line}, col{col}, lines{std::move(lines)} {}
std::pair<size_t, size_t> Add::operator()(std::vector<std::string>& lines) {
	auto it = this->lines.begin();
	std::string& curr_line = lines[line];
	size_t new_line{line};
	size_t new_col{col};
	std::string suffix;
	if (new_col == 0) {
		if (curr_line.size() > 1) {
			suffix = curr_line;
		}
		curr_line.insert(0, *it);
		new_col++;
	} else {
		if (curr_line.size() > new_col - 1) {
			suffix = curr_line.substr(new_col - 1);
		}
		curr_line.insert(new_col - 1, *it);
	}
	it++;
	auto i = lines.begin() + line + 1;
	for (; it < this->lines.end(); it++, i++) {
		lines.insert(i, *it);
		new_line++;
	}
	if (this->lines.size() > 1) {
		std::string& orig_line = lines[line];  // in case of realloc of memory
		orig_line.erase(orig_line.size() - suffix.size());
		lines[new_line].append(suffix);
		new_col = 0;
	}
	/*if (this->lines.size() > 1) {
		i->insert(0, *it);
	}*/
	return std::pair<size_t, size_t>{new_line, new_col + 1};
}
std::shared_ptr<Action> Add::reverse() {
	return std::make_shared<Remove>(line, col, lines);
}
std::pair<size_t, size_t> Remove::operator()(std::vector<std::string>& lines) {
	auto it = this->lines.begin();

	if (col > 0) {
		lines[line].erase(col - 1, it->size());
	} else if (col == 1) {
		lines[line].erase(1, it->size());
	}
	it++;
	const size_t next_line = line + 1;
	for (; it < this->lines.end() - 1; it++) {
		// lines[curr_line - 1].append(lines[curr_line]);
		lines.erase(lines.begin() + next_line);
	}
	if (this->lines.size() > 1) {
		lines[line].append(lines[next_line].substr(it->size()));
		lines.erase(lines.begin() + next_line);
	}
	/*auto i = lines.begin() + line;
	for (; it < lines.end() - 1; it++, i++) {
		lines.erase(i);
	}
	if (this->lines.size() > 1) {
		i->erase(0, it->size());
	}*/
	return std::pair<size_t, size_t>{line, col};
}
std::shared_ptr<Action> Remove::reverse() {
	return std::make_shared<Add>(line, col, lines);
}
#include "action.hpp"

#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "position.hpp"
Action::Action(size_t line, size_t col, std::vector<std::string> lines) : line{line}, col{col}, lines{std::move(lines)} {}
Position Action::get_end() const {
	size_t end_line = line;
	auto it = lines.begin();
	for (; it < lines.end() - 1; it++) {
		end_line++;
	}
	size_t end_col{0};
	if (lines.size() > 1) {
		end_col = it->size() + 1;
	} else {
		end_col = col + it->size();
	}
	return Position{end_line, end_col};
}
std::shared_ptr<Action> Action::merge_if_adj(const std::shared_ptr<Action>& action1, const std::shared_ptr<Action>& action2,
											 const std::vector<std::string>& lines) {
	if (dynamic_cast<Add*>(action1.get()) != nullptr && dynamic_cast<Add*>(action2.get()) != nullptr) {
		auto end = action1->get_end();
		if (end.line == action2->line && end.col == action2->col && action2->lines.size() < 2) {
			auto it = action2->lines.begin();
			action1->lines.back().append(*it);
			it++;
			if (lines.size() > 1) {
				action1->lines.insert(action1->lines.end(), it, action2->lines.end());
			}
			return action1;
		}
	} else if (dynamic_cast<Remove*>(action1.get()) != nullptr && dynamic_cast<Remove*>(action2.get()) != nullptr) {
		auto end = action2->get_end();
		if (end.line == action1->line && end.col == action1->col) {
			auto it = action1->lines.begin();
			action2->lines.back().append(*it);
			it++;
			if (lines.size() > 1) {
				action2->lines.insert(action2->lines.end(), it, action1->lines.end());
			}
			return action2;
		}
	}
	return nullptr;
}
Position Add::operator()(std::vector<std::string>& lines) {
	auto it = this->lines.begin();
	std::string& curr_line = lines[line];
	size_t new_line{line};
	size_t new_col{col + it->size()};
	std::string suffix;
	if (new_col == 0) {
		if (curr_line.size() > 1) {
			suffix = curr_line;
		}
		curr_line.insert(0, *it);
		new_col++;
	} else {
		if (curr_line.size() > col - 1) {
			suffix = curr_line.substr(col - 1);
		}
		curr_line.insert(col - 1, *it);
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
		new_col = lines[new_line].size() + 1;
		lines[new_line].append(suffix);
	}
	return Position{new_line, new_col};
}
std::shared_ptr<Action> Add::reverse() {
	return std::make_shared<Remove>(line, col, lines);
}
Position Remove::operator()(std::vector<std::string>& lines) {
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
	return Position{line, col};
}
std::shared_ptr<Action> Remove::reverse() {
	return std::make_shared<Add>(line, col, lines);
}
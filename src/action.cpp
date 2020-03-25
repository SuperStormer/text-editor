#include "action.hpp"

#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
Action::Action(size_t line, size_t col, std::vector<std::string> lines) : line{line}, col{col}, lines{std::move(lines)} {}
const std::pair<size_t, size_t> Action::get_end() const {
	size_t end_line = line;
	auto it = lines.begin();
	for (; it < lines.end() - 1; it++) {
		end_line++;
	}
	size_t end_col;
	if (lines.size() > 1) {
		end_col = it->size();
	} else {
		end_col = col + it->size();
	}
	return std::pair<size_t, size_t>{end_line, end_col};
}
void Action::merge(std::shared_ptr<Action> action) {
	auto it = action->lines.begin();
	lines.back().append(*it);
	it++;
	if (action->lines.size() > 1) {
		lines.insert(lines.end(), it, action->lines.end());
	}
}
std::shared_ptr<Action> Action::merge_if_adj(const std::shared_ptr<Action>& action1, const std::shared_ptr<Action>& action2,
											 const std::vector<std::string>& lines) {
	if (dynamic_cast<Add*>(action1.get()) != nullptr && dynamic_cast<Add*>(action2.get()) != nullptr) {
		auto end = action1->get_end();
		if (end.first == action2->line && end.second == action2->col) {
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
		if ((end.first == action1->line && end.second == action1->col) ||
			(end.first == action1->line - 1 && action1->col == 0 && end.second == lines[end.first].size() - 1)) {
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
std::pair<size_t, size_t> Add::operator()(std::vector<std::string>& lines) {
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
	/*if (this->lines.size() > 1) {
		i->insert(0, *it);
	}*/
	return std::pair<size_t, size_t>{new_line, new_col};
}
std::shared_ptr<Action> Add::reverse() {
	return std::make_shared<Remove>(line, col, lines);
} /*
 bool Add::merge_if_adj(std::shared_ptr<Action>& action, const std::vector<std::string>& lines) {
	 if (dynamic_cast<Add*>(action.get()) != nullptr) {
		 auto end = action->get_end();
		 if ((end.first == line && end.second == col) ||
			 (end.first == line - 1 && col == 0 && end.second == lines[end.first].size() - 1)) {
			 auto it = this->lines.begin();
			 action->lines.back().append(*it);
			 it++;
			 if (lines.size() > 1) {
				 action->lines.insert(action->lines.end(), it, this->lines.end());
			 }
			 return true;
		 }
	 }
	 return false;
 }*/
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
} /*
 std::shared_ptr<Action> Remove::merge_if_adj(std::shared_ptr<Action>& action, const std::vector<std::string>& lines) {
	 if (dynamic_cast<Remove*>(action.get()) != nullptr) {
		 auto end = get_end();
		 if ((end.first == action->line && end.second == action->col) ||
			 (end.first == action->line - 1 && action->col == 0 && end.second == lines[end.first].size() - 1)) {
			 // action->merge(std::shared_ptr<Remove>{this});
			 // return action;
			 auto it = action->lines.begin();
			 this->lines.back().append(*it);
			 it++;
			 if (action->lines.size() > 1) {
				 this->lines.insert(this->lines.end(), it, action->lines.end());
			 }
		 }
	 }
	 return nullptr;
 }*/
#include "editor.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "action.hpp"
#include "key.hpp"
#include "position.hpp"
#include "utils.hpp"
#if defined(unix) || defined(__unix__) || defined(__unix)
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif
Editor::Editor(const std::string& filename, const std::vector<std::string>& args) : filename{filename} {
	std::ifstream input{filename};
	for (std::string line; std::getline(input, line);) {
		lines.push_back(line);
	}
	if (lines.empty()) {
		lines.emplace_back("");
	}
}
Editor::~Editor() {
	save();
	disable_raw_mode();
	while (!actions.empty()) {
		actions.pop();
	}
	std::cout << "\033[2J\033[H";
}
void Editor::start() {
	enable_raw_mode();
	display();
	done = false;
	while (!done) {
		update();
	}
}
inline Key Editor::get_key() {
	return static_cast<Key>(std::cin.get());
}
void Editor::update() {
	Key key = get_key();
	auto key_handler = keybinds.keybinds.find(key);
	if (key_handler != keybinds.keybinds.end()) {
		(this->*(*key_handler).second)();
	} else if (std::isprint(static_cast<int>(key)) != 0 || static_cast<char>(key) == '\t') {
		handle_key(key);
	} else if (key == Key::ESCAPE_START) {
		handle_escape();
	}
	display();
}
void Editor::handle_escape() {
	get_key();	// ignore [
	Key key = get_key();
	if (key != Key::MODIFIER_ARROW_START) {
		clear_selection();
	}
	auto key_handler = keybinds.escape_handlers.find(key);
	if (key_handler != keybinds.escape_handlers.end()) {
		(this->*(*key_handler).second)();
	}
}
void Editor::handle_arrow_up() {
	if (curr_line > 0) {
		change_line(-1);
		// handle differently sized lines
		if (col > lines[curr_line].size() + 1) {
			col = lines[curr_line].size() + 1;
		}
	}
}
void Editor::handle_arrow_down() {
	if (curr_line < lines.size() - 1) {
		change_line(1);
		if (col > lines[curr_line].size() + 1) {
			col = lines[curr_line].size() + 1;
		}
	}
}
void Editor::handle_arrow_left() {
	if (col > 1) {
		--col;
	} else if (curr_line > 0) {
		// handle moving left from the start of a line to the previous line
		change_line(-1);
		col = lines[curr_line].size() + 1;
	}
}
void Editor::handle_arrow_right() {
	if (col < lines[curr_line].size() + 1) {
		++col;
	} else if (curr_line < lines.size() - 1) {
		// handle moving right from the end of line to the following line
		change_line(1);
		col = 1;
	}
}
void Editor::handle_modifier_arrow() {
	// escape sequence is <ESC>[1;xy where x is 2,5 or 6(shift,ctrl or ctrl_shift) and y is a,b,c or d(u/d/l/r arrows)
	get_key();	// skip ;
	switch (get_key()) {
		case Key::SHIFT_ARROW_START:
			handle_shift_arrow();
			break;
		case Key::CTRL_ARROW_START:
			handle_ctrl_arrow();
			break;
		case Key::CTRL_SHIFT_ARROW_START:
			handle_ctrl_shift_arrow();
			break;
	}
}
void Editor::handle_shift_arrow() {
	start_selection();
	switch (get_key()) {
		case Key::ARROW_UP:
			handle_arrow_up();
			break;
		case Key::ARROW_DOWN:
			handle_arrow_down();
			break;
		case Key::ARROW_LEFT:
			handle_arrow_left();
			break;
		case Key::ARROW_RIGHT:
			handle_arrow_right();
			break;
	}
}
void Editor::handle_ctrl_arrow() {
	switch (get_key()) {
		case Key::ARROW_UP:
			handle_ctrl_arrow_up();
			break;
		case Key::ARROW_DOWN:
			handle_ctrl_arrow_down();
			break;
		case Key::ARROW_LEFT:
			handle_ctrl_arrow_left();
			break;
		case Key::ARROW_RIGHT:
			handle_ctrl_arrow_right();
			break;
	}
}
void Editor::handle_ctrl_arrow_up() {
	if (curr_line > 0) {
		change_line(-1);
	}
	col = 1;
}
void Editor::handle_ctrl_arrow_down() {
	if (curr_line < lines.size() - 1) {
		change_line(1);
		col = 1;
	} else {
		col = lines[curr_line].size() + 1;
	}
}
void Editor::handle_ctrl_arrow_left() {
	if (col == 1) {
		if (curr_line > 0) {
			change_line(-1);
			col = lines[curr_line].size() + 1;
		}
		return;
	}
	const std::string& line = lines[curr_line];
	const size_t pos = line.find_last_of(" \t\n()", col - 2);
	if (pos != std::string::npos) {
		col = pos + 1;
	} else {
		col = 1;
	}
}
void Editor::handle_ctrl_arrow_right() {
	const std::string& line = lines[curr_line];
	const size_t pos = line.find_first_of(" \t\n()", col);
	if (pos != std::string::npos) {
		col = pos + 1;
	} else if (col < line.size() + 1) {
		col = line.size() + 1;
	} else if (curr_line < lines.size() - 1) {
		change_line(1);
		col = 1;
	}
}
void Editor::handle_ctrl_shift_arrow() {
	start_selection();
	handle_ctrl_arrow();
}
void Editor::handle_backspace() {
	if (col == 1) {
		// handle deleting the newline
		if (curr_line > 0) {
			col = lines[curr_line - 1].size() + 1;
			change_line(-1);
			perform_action(Remove(curr_line, col, std::vector<std::string>{"", ""}));
		}
	} else {
		char removed_char = lines[curr_line][col - 2];
		perform_action(Remove(curr_line, col - 1, std::vector<std::string>{std::string(1, removed_char)}));
	}
}
void Editor::handle_enter() {
	perform_action(Add(curr_line, col, std::vector<std::string>{"", ""}));
}
void Editor::handle_key(Key key) {
	// std::cout << static_cast<int>(key) << " ";
	char chr = static_cast<char>(key);
	perform_action(Add(curr_line, col, std::vector<std::string>{std::string(1, chr)}));
}
void Editor::quit() {
	done = true;
}
void Editor::save() {
	std::ofstream output{filename};
	for (const auto& line : lines) {
		output << line << "\n";
	}
}
void Editor::cut() {
	copy();
	perform_action(Remove(selection_start.line, selection_start.col, clipboard));
}
void Editor::copy() {
	clipboard = {};
	const std::pair<Position, Position>& selection_bounds = std::minmax(selection_start, Position{curr_line, col});
	Position real_selection_start = selection_bounds.first;
	Position selection_end = selection_bounds.second;
	auto it = lines.begin() + real_selection_start.line;
	auto end = lines.begin() + selection_end.line;
	if (it == end) {
		clipboard.emplace_back(it->begin() + real_selection_start.col - 1, it->begin() + selection_end.col - 1);
	} else {
		clipboard.emplace_back(it->begin() + real_selection_start.col - 1, it->end());
		++it;
		for (; it < end - 1; ++it) {
			clipboard.push_back(*it);
		}
		clipboard.emplace_back(it->begin(), it->begin() + selection_end.col);
	}
}
void Editor::paste() {
	perform_action(Add(curr_line, col, clipboard));
}
void Editor::undo() {
	if (!actions.empty()) {
		std::shared_ptr<Action> action = actions.top();
		actions.pop();
		std::shared_ptr<Action> new_action = action->reverse();
		execute_action(*new_action);
		undos.push(new_action);
	}
}
void Editor::redo() {
	if (!undos.empty()) {
		std::shared_ptr<Action> action = undos.top();
		undos.pop();
		std::shared_ptr<Action> new_action = action->reverse();
		execute_action(*new_action);
		actions.push(new_action);
	}
}
Editor::KeyBinds::KeyBinds(std::unordered_map<Key, KeyHandler> keybinds, std::unordered_map<Key, KeyHandler> escape_handlers)
	: keybinds(std::move(keybinds)), escape_handlers(std::move(escape_handlers)) {}
const Editor::KeyBinds Editor::KeyBinds::default_binds{{{Key::CTRL_C, &Editor::copy},
														{Key::CTRL_Q, &Editor::quit},
														{Key::CTRL_V, &Editor::paste},
														{Key::CTRL_X, &Editor::cut},
														{Key::CTRL_S, &Editor::save},
														{Key::CTRL_Y, &Editor::redo},
														{Key::CTRL_Z, &Editor::undo},
														{Key::BACKSPACE, &Editor::handle_backspace},
														{Key::ENTER, &Editor::handle_enter}},
													   {{Key::ARROW_UP, &Editor::handle_arrow_up},
														{Key::ARROW_DOWN, &Editor::handle_arrow_down},
														{Key::ARROW_LEFT, &Editor::handle_arrow_left},
														{Key::ARROW_RIGHT, &Editor::handle_arrow_right},
														{Key::MODIFIER_ARROW_START, &Editor::handle_modifier_arrow}}};
void Editor::display() {
	const int tab_size = 4;	 // TODO read this from a config file
	const std::string tab_repl(tab_size, ' ');
	const std::string highlight_start = "\033[7m";
	const std::string highlight_end = "\033[0m";
	const size_t end_line = window_start + get_terminal_size().first;
	std::cout << "\033[2J\033[H";
	auto end = lines.begin() + end_line;
	if (end > lines.end()) {
		end = lines.end();
	}
	if (has_selection) {
		const std::pair<Position, Position>& selection_bounds = std::minmax(selection_start, Position{curr_line, col});
		Position real_selection_start = selection_bounds.first;
		Position selection_end = selection_bounds.second;
		std::vector<std::string>::iterator selection_start_line;
		if (selection_start.line >= window_start) {
			selection_start_line = lines.begin() + real_selection_start.line;
		} else {
			selection_start_line = lines.begin() + window_start;
		}
		std::vector<std::string>::iterator selection_end_line;
		if (selection_end.line <= end_line) {
			selection_end_line = lines.begin() + selection_end.line;
		} else {
			selection_end_line = lines.begin() + end_line;
		}
		auto it = lines.begin() + window_start;
		for (; it < end; ++it) {
			if (it != lines.begin()) {
				std::cout << '\n';
			}
			if (it != selection_start_line && it != selection_end_line) {
				std::cout << replace_all(*it, "\t", tab_repl);
			} else if (it == selection_start_line && it == selection_end_line) {
				std::string new_str{*it};
				new_str.insert(real_selection_start.col - 1, highlight_start);
				if (selection_end.col + highlight_start.length() <= new_str.length()) {
					new_str.insert(selection_end.col + highlight_start.length() - 1, highlight_end);
				} else {
					new_str.append(highlight_end);
				}
				std::cout << replace_all(new_str, "\t", tab_repl);
			} else if (it == selection_start_line) {
				std::string new_str{*it};
				new_str.insert(real_selection_start.col - 1, highlight_start);
				std::cout << replace_all(new_str, "\t", tab_repl);
			} else if (it == selection_end_line) {
				std::string new_str{*it};
				new_str.insert(selection_end.col - 1, highlight_end);
				std::cout << replace_all(new_str, "\t", tab_repl);
			}
		}
	} else {
		auto it = lines.begin() + window_start;
		for (; it < end; ++it) {
			if (it != lines.begin()) {
				std::cout << '\n';
			}
			std::cout << replace_all(*it, "\t", tab_repl);
		}
	}
	const std::string& line = lines[curr_line];
	// fix cols b/c tabs displayed as spaces in output messes up
	size_t tabs = std::count(line.begin(), line.begin() + col - 1, '\t');
	std::cout << "\033[" << curr_line - window_start + 1 << ";" << col + tabs * (tab_size - 1) << "f";
}
inline void Editor::change_line(size_t offset) {
	curr_line += offset;
	// adjust window_start if curr_line will be offscreen
	if (curr_line >= window_start + get_terminal_size().first) {
		window_start = curr_line - get_terminal_size().first + 1;
	} else if (curr_line < window_start) {
		window_start = curr_line;
	}
}
template <typename T>
inline void Editor::execute_action(T&& action) {
	Position position = action(lines);
	size_t new_line = position.line;
	col = position.col;
	change_line(new_line - curr_line);
}
template <typename T>
void Editor::perform_action(T&& action) {
	clear_selection();
	clear_undos();
	execute_action(action);
	push_action(std::make_shared<T>(action));
}
void Editor::push_action(const std::shared_ptr<Action>& action) {
	std::chrono::time_point<Clock> now = Clock::now();
	std::chrono::duration<double> elapsed = now - action_timer;
	action_timer = now;
	if (elapsed.count() < 0.5) {
		// chain actions to avoid 1-char actions
		std::shared_ptr<Action> prev = actions.top();
		std::shared_ptr<Action> new_action = Action::merge_if_adj(prev, action, lines);
		if (new_action) {
			actions.pop();
			actions.push(new_action);
			return;
		}
	}
	actions.push(action);
}
inline void Editor::clear_undos() {
	std::stack<std::shared_ptr<Action>>().swap(undos);
}
inline void Editor::start_selection() {
	if (!has_selection) {
		selection_start = {curr_line, col};
		has_selection = true;
	}
}
inline void Editor::clear_selection() {
	has_selection = false;
	selection_start = {};
}
void Editor::disable_raw_mode() {
#if defined(unix) || defined(__unix__) || defined(__unix)
	// from https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
		throw std::runtime_error{"tcsetattr returned -1"};
	}
#elif defined(_WIN32)
	HANDLE h_stdin = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(h_stdin, orig_console_mode);
#endif
}
void Editor::enable_raw_mode() {
#if defined(unix) || defined(__unix__) || defined(__unix)
	// from https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
		throw std::runtime_error{"tcgetattr returned -1"};
	}
	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	// raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
		throw std::runtime_error{"tcsetattr returned -1"};
	}
#elif defined(_WIN32)
	// from https://cpp.hotexamples.com/examples/-/-/SetConsoleMode/cpp-setconsolemode-function-examples.html and
	// https://docs.microsoft.com/en-us/windows/console/setconsolemode
	HANDLE h_stdin = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(h_stdin, (LPDWORD)&orig_console_mode);
	DWORD raw = orig_console_mode;
	raw &= ~(ENABLE_ECHO_INPUT | ENABLE_INSERT_MODE | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
	SetConsoleMode(h_stdin, raw);
#endif
}
std::pair<int, int> Editor::get_terminal_size() {
#if defined(unix) || defined(__unix__) || defined(__unix)
	// from https://stackoverflow.com/a/1022961/7941251
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return std::pair{w.ws_row, w.ws_col};
#elif defined(_Win32)
	// from https://stackoverflow.com/a/12642749/7941251
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return std::pair{columns, rows};
#endif
}
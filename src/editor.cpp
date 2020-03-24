#include "editor.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "action.hpp"
#include "key.hpp"
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
	auto key_handler = keybinds.escape_handlers.find(key);
	if (key_handler != keybinds.escape_handlers.end()) {
		(this->*(*key_handler).second)();
	}
}
void Editor::handle_arrow_up() {
	if (curr_line > 0) {
		move_up();
		if (col > lines[curr_line].size() + 1) {
			col = lines[curr_line].size() + 1;
		}
	}
}
void Editor::handle_arrow_down() {
	if (curr_line < lines.size() - 1) {
		move_down();
		if (col > lines[curr_line].size() + 1) {
			col = lines[curr_line].size() + 1;
		}
	}
}
void Editor::handle_arrow_left() {
	if (col > 1) {
		col--;
	} else if (curr_line > 0) {
		move_up();
		col = lines[curr_line].size() + 1;
	}
}
void Editor::handle_arrow_right() {
	if (col < lines[curr_line].size() + 1) {
		col++;
	} else if (curr_line < lines.size() - 1) {
		move_down();
		col = 1;
	}
}
void Editor::handle_backspace() {
	if (col == 1) {
		if (curr_line > 0) {
			std::string removed_line{lines[curr_line]};
			col = lines[curr_line - 1].size();
			/*lines[curr_line - 1].append(lines[curr_line]);
			lines.erase(lines.begin() + curr_line);
			move_up();*/
			move_up();
			perform_action(Remove(curr_line, col, std::vector<std::string>{"", ""}));
			col++;
		}
	} else {
		char removed_char = lines[curr_line][col - 2];
		// lines[curr_line].erase(col - 2, 1);
		perform_action(Remove(curr_line, col - 1, std::vector<std::string>{std::string(1, removed_char)}));
		// col--;
		// push_action(std::make_shared<Remove>(curr_line, col, std::vector<std::string>{std::string(1, removed_char)}));
	}
}
void Editor::handle_enter() {
	perform_action(Add(curr_line, col, std::vector<std::string>{"", ""}));
	/*auto& line = lines[curr_line];
	auto start = line.begin() + col - 1;
	std::string new_line = std::string(start, line.end());
	line.erase(start, line.end());
	lines.insert(lines.begin() + curr_line + 1, new_line);
	push_action(std::make_shared<Add>(curr_line, col, std::vector<std::string>{"", ""}));
	move_down();
	col = 1;*/
}
void Editor::handle_key(Key key) {
	// std::cout << static_cast<int>(key);
	char chr = static_cast<char>(key);
	perform_action(Add(curr_line, col, std::vector<std::string>{std::string(1, chr)}));

	/*if (col == 0) {
		lines[curr_line].insert(0, 1, chr);
		col++;
	} else {
		lines[curr_line].insert(col - 1, 1, chr);
		push_action(std::make_shared<Add>(curr_line, col - 1, std::vector<std::string>{std::string(1, chr)}));
	}
	col++;*/
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
void Editor::redo() {
	if (!undos.empty()) {
		std::shared_ptr<Action> action = undos.top();
		undos.pop();
		std::shared_ptr<Action> new_action = action->reverse();
		std::tie(curr_line, col) = (*new_action)(lines);
		actions.push(new_action);
	}
}
void Editor::undo() {
	if (!actions.empty()) {
		std::shared_ptr<Action> action = actions.top();
		actions.pop();
		std::shared_ptr<Action> new_action = action->reverse();
		std::tie(curr_line, col) = (*new_action)(lines);
		undos.push(new_action);
	}
}
Editor::KeyBinds::KeyBinds(std::unordered_map<Key, KeyHandler> keybinds, std::unordered_map<Key, KeyHandler> escape_handlers)
	: keybinds(std::move(keybinds)), escape_handlers(std::move(escape_handlers)) {}
const Editor::KeyBinds Editor::KeyBinds::default_binds{{{Key::CTRL_C, &Editor::quit},
														{Key::CTRL_S, &Editor::save},
														{Key::CTRL_Y, &Editor::redo},
														{Key::CTRL_Z, &Editor::undo},
														{Key::BACKSPACE, &Editor::handle_backspace},
														{Key::ENTER, &Editor::handle_enter}},
													   {{Key::ARROW_UP, &Editor::handle_arrow_up},
														{Key::ARROW_DOWN, &Editor::handle_arrow_down},
														{Key::ARROW_LEFT, &Editor::handle_arrow_left},
														{Key::ARROW_RIGHT, &Editor::handle_arrow_right}}};
void Editor::display() {
	int tab_size = 4;  // TODO read this from a config file
	std::cout << "\033[2J\033[H";
	auto end = lines.begin() + window_start + get_terminal_size().first;
	if (end > lines.end()) {
		end = lines.end();
	}
	auto it = lines.begin() + window_start;
	std::cout << replace_all(*it, "\t", std::string(tab_size, ' '));
	it++;
	for (; it < end; it++) {
		std::cout << "\n" << replace_all(*it, "\t", std::string(tab_size, ' '));
	}
	std::string line = lines[curr_line];
	// fix cols b/c tabs displayed as spaces in output messes up
	size_t tabs = std::count(line.begin(), line.begin() + col - 1, '\t');
	std::cout << "\033[" << curr_line - window_start + 1 << ";" << col + tabs * (tab_size - 1) << "f";
}
inline void Editor::move_up() {
	if (curr_line == window_start) {
		window_start--;
	}
	curr_line--;
}
inline void Editor::move_down() {
	if (curr_line == window_start + get_terminal_size().first - 1) {
		window_start++;
	}
	curr_line++;
}
template <typename T>
void Editor::perform_action(T&& action) {
	clear_undos();
	std::tie(curr_line, col) = action(lines);
	push_action(std::make_shared<T>(action));
}
void Editor::push_action(const std::shared_ptr<Action>& action) {
	/*std::clog << typeid(*action).name() << " " << actio	n->line << " " << action->col << "'";
	for (const auto& line : action->lines) {
		std::clog << line << "\n";
	}
	std::clog << "'\n";*/

	actions.push(action);
}
inline void Editor::clear_undos() {
	std::stack<std::shared_ptr<Action>>().swap(undos);
}
void Editor::disable_raw_mode() {
#if defined(unix) || defined(__unix__) || defined(__unix)
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
	HANDLE h_stdin = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(h_stdin, (LPDWORD)&orig_console_mode);
	DWORD raw = orig_console_mode;
	raw &= ~(ENABLE_ECHO_INPUT | ENABLE_INSERT_MODE | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
	SetConsoleMode(h_stdin, raw);
#endif
}
std::pair<int, int> Editor::get_terminal_size() {
#if defined(unix) || defined(__unix__) || defined(__unix)
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return std::pair{w.ws_row, w.ws_col};
#elif defined(_Win32)
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	return std::pair{columns, rows};
#endif
}
#include "editor.h"

#include <cctype>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "key.h"
#include "termios.h"
#include "unistd.h"
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
	} else if (std::isprint(static_cast<int>(key)) != 0) {
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
		curr_line--;
		if (col > lines[curr_line].size() + 1) {
			col = lines[curr_line].size() + 1;
		}
	}
}
void Editor::handle_arrow_down() {
	if (curr_line < lines.size() - 1) {
		curr_line++;
		if (col > lines[curr_line].size() + 1) {
			col = lines[curr_line].size() + 1;
		}
	}
}
void Editor::handle_arrow_left() {
	if (col > 1) {
		col--;
	} else if (curr_line > 0) {
		curr_line--;
		col = lines[curr_line].size() + 1;
	}
}
void Editor::handle_arrow_right() {
	if (col < lines[curr_line].size() + 1) {
		col++;
	} else if (curr_line < lines.size() - 1) {
		curr_line++;
		col = 1;
	}
}
void Editor::handle_backspace() {
	if (col == 1) {
		if (curr_line > 0) {
			col = lines[curr_line - 1].size() + 1;
			lines[curr_line - 1].append(lines[curr_line]);
			lines.erase(lines.begin() + curr_line);
			curr_line--;
		}
	} else {
		lines[curr_line].erase(col - 2, 1);
		col--;
	}
}
void Editor::handle_enter() {
	auto& line = lines[curr_line];
	auto start = line.begin() + col - 1;
	std::string new_line = std::string(start, line.end());
	line.erase(start, line.end());
	lines.insert(lines.begin() + curr_line + 1, new_line);
	curr_line++;
	col = 1;
}
void Editor::handle_key(Key key) {
	// std::cout << static_cast<int>(key);
	if (col == 0) {
		lines[curr_line].insert(0, 1, static_cast<char>(key));
		col++;
	} else {
		lines[curr_line].insert(col - 1, 1, static_cast<char>(key));
	}
	col++;
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
Editor::KeyBinds::KeyBinds(std::unordered_map<Key, KeyHandler> keybinds, std::unordered_map<Key, KeyHandler> escape_handlers)
	: keybinds(std::move(keybinds)), escape_handlers(std::move(escape_handlers)) {}
const Editor::KeyBinds Editor::KeyBinds::default_binds{{{Key::CTRL_C, &Editor::quit},
														{Key::CTRL_S, &Editor::save},
														{Key::BACKSPACE, &Editor::handle_backspace},
														{Key::ENTER, &Editor::handle_enter}},
													   {{Key::ARROW_UP, &Editor::handle_arrow_up},
														{Key::ARROW_DOWN, &Editor::handle_arrow_down},
														{Key::ARROW_LEFT, &Editor::handle_arrow_left},
														{Key::ARROW_RIGHT, &Editor::handle_arrow_right}}};
void Editor::display() {
	std::cout << "\033[2J\033[H";
	for (const auto& line : lines) {
		std::cout << line << "\n";
	}
	std::cout << "\033[" << curr_line + 1 << ";" << col << "f";
}

void Editor::disable_raw_mode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
		throw std::runtime_error{"tcsetattr returned -1"};
	}
}
void Editor::enable_raw_mode() {
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
}
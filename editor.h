#ifndef EDITOR_H
#define EDITOR_H
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "key.h"
#if defined(unix) || defined(__unix__) || defined(__unix)
#include "termios.h"
#elif defined(_WIN32)
#include <windows.h>
#endif
class Editor;
using KeyHandler = void (Editor::*)();
class Editor {
   public:
	Editor(const std::string& filename, const std::vector<std::string>& args);
	~Editor();
	Editor(const Editor& editor) = default;
	Editor& operator=(const Editor& editor) = default;
	Editor(Editor&& editor) = default;
	Editor& operator=(Editor&& editor) = default;
	Key get_key();
	void start();
	void update();
	void handle_escape();
	void handle_backspace();
	void handle_enter();
	void handle_arrow_up();
	void handle_arrow_down();
	void handle_arrow_left();
	void handle_arrow_right();
	void handle_key(Key key);
	void quit();
	void save();
	void display();
	void disable_raw_mode();
	void enable_raw_mode();

   private:
	size_t curr_line{0};
	size_t col{1};
	bool done{false};
	std::string filename;
	std::vector<std::string> lines{};
	struct KeyBinds {
		KeyBinds(std::unordered_map<Key, KeyHandler> keybinds, std::unordered_map<Key, KeyHandler> escape_handlers);
		std::unordered_map<Key, KeyHandler> keybinds;
		std::unordered_map<Key, KeyHandler> escape_handlers;
		const static KeyBinds default_binds;
	};
	KeyBinds keybinds{KeyBinds::default_binds};
#if defined(unix) || defined(__unix__) || defined(__unix)
	struct termios orig_termios;
#elif defined(_WIN32)
	DWORD orig_console_mode;
#endif
};
#endif
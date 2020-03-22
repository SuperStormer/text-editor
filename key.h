#ifndef KEY_H
#define KEY_H
enum class Key : int {
	CTRL_C = 3,
	CTRL_S = 19,

	ESCAPE_START = 27,
	// ansi escape codes
	ARROW_UP = 'A',
	ARROW_DOWN = 'B',
	ARROW_RIGHT = 'C',
	ARROW_LEFT = 'D',
	HOME = 'H',
	END = 'F',
	INSERT = '2',
	DELETE = '3',
	PAGE_UP = '5',
	PAGE_DOWN = '6',
	BACKSPACE = 127,
	ENTER = 13,

};
#endif
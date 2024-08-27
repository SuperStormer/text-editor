#ifndef KEY_H
#define KEY_H
enum class Key : int {
	CTRL_C = 3,
	CTRL_Q = 17,
	CTRL_S = 19,
	CTRL_V = 22,
	CTRL_X = 24,
	CTRL_Y = 25,
	CTRL_Z = 26,
	BACKSPACE = 127,
	ENTER = 13,
	ESCAPE_START = '\033',
	// ANSI escape codes
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
	MODIFIER_ARROW_START = '1',
	SHIFT_ARROW_START = '2',
	CTRL_ARROW_START = '5',
	CTRL_SHIFT_ARROW_START = '6'
};
#endif
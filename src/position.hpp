#ifndef POSITION_H
#define POSITION_H
#include <cstddef>
struct Position {
	size_t line;
	size_t col;
};
inline bool operator==(const Position& lhs, const Position& rhs) {
	return lhs.line == rhs.line && lhs.col == rhs.col;
}
inline bool operator!=(const Position& lhs, const Position& rhs) {
	return !operator==(lhs, rhs);
}
inline bool operator<(const Position& lhs, const Position& rhs) {
	return lhs.line < rhs.line || (lhs.line == rhs.line && lhs.col < rhs.col);
}
inline bool operator>(const Position& lhs, const Position& rhs) {
	return operator<(rhs, lhs);
}
inline bool operator<=(const Position& lhs, const Position& rhs) {
	return !operator>(lhs, rhs);
}
inline bool operator>=(const Position& lhs, const Position& rhs) {
	return !operator<(lhs, rhs);
}
#endif
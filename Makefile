CC = g++
CXX = g++
CXXFLAGS= -Wall -Wextra -Wno-switch -std=c++17 -pedantic -ggdb3
texteditor: texteditor.o editor.o key.h

.PHONY: clean

clean:
	rm *.o texteditor
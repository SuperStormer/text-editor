CXX = clang++
CXXFLAGS = -Wall -Wextra -Wno-switch -std=c++17 -pedantic
ifdef prod
	CXXFLAGS += -O3
else
	ifdef lint
		CXXFLAGS += -fsanitize=undefined -fsanitize=address
	else
		CXXFLAGS += -ggdb3
	endif
endif
#adapted from from https://stackoverflow.com/a/2908351/7941251
SRC_FOLDER = ./src
ifeq ($(shell uname), Linux)
	OBJ_FOLDER := ./obj_linux
else
	OBJ_FOLDER := ./obj_windows
endif

SRC_FILES = $(wildcard $(SRC_FOLDER)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_FOLDER)/%.cpp,$(OBJ_FOLDER)/%.o,$(SRC_FILES))


texteditor: $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o texteditor $(OBJ_FILES)
$(OBJ_FOLDER)/%.o: $(SRC_FOLDER)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f obj_linux/*.o obj_windows/*.o texteditor texteditor.exe

.PHONY: clean

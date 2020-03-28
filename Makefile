CXX = clang++
CXXFLAGS= -Wall -Wextra -Wno-switch -std=c++17 -pedantic -ggdb3 -O2

#adapted from from https://stackoverflow.com/a/2908351/7941251
SRC_FOLDER = ./src
ifeq ($(shell uname), Linux)
	OBJ_FOLDER = ./obj_linux
else 
	OBJ_FOLDER = ./obj_windows
endif
SRC_FILES := $(wildcard $(SRC_FOLDER)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_FOLDER)/%.cpp,$(OBJ_FOLDER)/%.o,$(SRC_FILES))
texteditor: $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_FILES)

$(OBJ_FOLDER)/%.o: $(SRC_FOLDER)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
.PHONY: clean

clean:
	rm -f obj_linux/*.o obj_windows/*.o texteditor texteditor.exe

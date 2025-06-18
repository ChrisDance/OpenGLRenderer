# Compilers
CXX = clang++
CC = clang

# Project name
TARGET = bin

# Source directories


# Source files - automatically find all .cpp and .c files
CPP_SOURCES = $(wildcard *.cpp)
C_SOURCES = $(wildcard *.c)

# Object files
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)
C_OBJECTS = $(C_SOURCES:.c=.o)
OBJECTS = $(CPP_OBJECTS) $(C_OBJECTS)

# Homebrew prefix (adjust if needed)
BREW_PREFIX = /opt/homebrew
# For Intel Macs, use: BREW_PREFIX = /usr/local

# Include directories (added Shared directory)
INCLUDES = -I$(BREW_PREFIX)/include -I$(BREW_PREFIX)/include/freetype2 -I$(SHARED_DIR)

# Library directories
LIB_DIRS = -L$(BREW_PREFIX)/lib

# Libraries to link - Added OpenGL framework and GL_SILENCE_DEPRECATION
LIBS = -lglfw -lfreetype -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lassimp

# Compiler flags - Added GL_SILENCE_DEPRECATION to prevent OpenGL deprecation conflicts
CXXFLAGS = -std=c++17 -Wall -O0 $(INCLUDES) -g -DGL_SILENCE_DEPRECATION
CFLAGS = -std=c99 -Wall -O0 $(INCLUDES) -DGL_SILENCE_DEPRECATION

# Linker flags
LDFLAGS = $(LIB_DIRS) $(LIBS)

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile C++ source files (handles both current dir and Shared dir)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile C source files (handles both current dir and Shared dir)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts (including Shared objects)
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(SHARED_DIR)/*.o

# Rebuild everything
rebuild: clean all

# Run the application
run: $(TARGET)
	./$(TARGET)

# Install dependencies (requires Homebrew)
install-deps:
	brew install glfw freetype

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Check Homebrew prefix
check-brew:
	@echo "Homebrew prefix: $(BREW_PREFIX)"
	@echo "GLFW location: $(BREW_PREFIX)/lib/libglfw.a"
	@echo "FreeType location: $(BREW_PREFIX)/lib/libfreetype.a"
	@ls -la $(BREW_PREFIX)/lib/libglfw* 2>/dev/null || echo "GLFW not found - run 'make install-deps'"
	@ls -la $(BREW_PREFIX)/lib/libfreetype* 2>/dev/null || echo "FreeType not found - run 'make install-deps'"

# Show what files will be compiled (for debugging)
show-sources:
	@echo "C++ Sources: $(CPP_SOURCES)"
	@echo "C Sources: $(C_SOURCES)"
	@echo "Objects: $(OBJECTS)"

.PHONY: all clean rebuild run install-deps debug help check-brew show-sources

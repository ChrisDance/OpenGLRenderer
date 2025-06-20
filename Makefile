# Compiler and flags
CXX = clang++
CC = clang
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
CXXFLAGS_DEBUG = -std=c++20 -Wall -Wextra -g -O0 -DDEBUG
CFLAGS = -Wall -Wextra -O2
CFLAGS_DEBUG = -Wall -Wextra -g -O0 -DDEBUG

# Include directories
# INCLUDES = -I. -Ilib -I/opt/homebrew/include
INCLUDES = -I. -Ilib -I/opt/homebrew/include -I/opt/homebrew/include/freetype2

# Library directories and libraries
LIBDIRS = -L/opt/homebrew/lib
LIBS = -lglfw -lfreetype -lassimp -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

# Target executable
TARGET = bin
TARGET_DEBUG = bin

# Source files - automatically find all .cpp and .c files
CPP_SOURCES = $(wildcard *.cpp) $(wildcard lib/*.cpp)
C_SOURCES = $(wildcard *.c) $(wildcard lib/*.c)

# Object files - all go in obj/ directory with directory prefixes to avoid conflicts
CPP_FLATTENED = $(subst /,_,$(CPP_SOURCES))
C_FLATTENED = $(subst /,_,$(C_SOURCES))
CPP_OBJECTS = $(patsubst %.cpp,obj/%.o,$(CPP_FLATTENED))
C_OBJECTS = $(patsubst %.c,obj/%.o,$(C_FLATTENED))
CPP_OBJECTS_DEBUG = $(patsubst %.cpp,obj/debug_%.o,$(CPP_FLATTENED))
C_OBJECTS_DEBUG = $(patsubst %.c,obj/debug_%.o,$(C_FLATTENED))
OBJECTS = $(CPP_OBJECTS) $(C_OBJECTS)
OBJECTS_DEBUG = $(CPP_OBJECTS_DEBUG) $(C_OBJECTS_DEBUG)

# Default target
all: $(TARGET)

# Debug target
debug: $(TARGET_DEBUG)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

# Link the debug executable
$(TARGET_DEBUG): $(OBJECTS_DEBUG)
	$(CXX) $(OBJECTS_DEBUG) $(LIBDIRS) $(LIBS) -o $(TARGET_DEBUG)

# Create obj directory
obj:
	mkdir -p obj

# Compile rules - root directory C++ sources
obj/%.o: %.cpp | obj
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile rules - root directory C sources
obj/%.o: %.c | obj
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile rules - lib directory C++ sources
obj/lib_%.o: lib/%.cpp | obj
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile rules - lib directory C sources
obj/lib_%.o: lib/%.c | obj
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Debug compile rules - root directory C++ sources
obj/debug_%.o: %.cpp | obj
	$(CXX) $(CXXFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

# Debug compile rules - root directory C sources
obj/debug_%.o: %.c | obj
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

# Debug compile rules - lib directory C++ sources
obj/debug_lib_%.o: lib/%.cpp | obj
	$(CXX) $(CXXFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

# Debug compile rules - lib directory C sources
obj/debug_lib_%.o: lib/%.c | obj
	$(CC) $(CFLAGS_DEBUG) $(INCLUDES) -c $< -o $@

# Clean target
clean:
	find obj -name "*.o" ! -name "lib_glad.o" ! -name "lib_stb_image.o" -delete 2>/dev/null || true
	rm -rf $(TARGET) $(TARGET_DEBUG)


# Rebuild target
rebuild: clean all

# Debug info
makefile_debug:
	@echo "CPP_SOURCES: $(CPP_SOURCES)"
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "CPP_FLATTENED: $(CPP_FLATTENED)"
	@echo "C_FLATTENED: $(C_FLATTENED)"
	@echo "CPP_OBJECTS: $(CPP_OBJECTS)"
	@echo "C_OBJECTS: $(C_OBJECTS)"
	@echo "OBJECTS: $(OBJECTS)"

# Phony targets
.PHONY: all debug clean rebuild makefile_debug

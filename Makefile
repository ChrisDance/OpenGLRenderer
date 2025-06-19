# Compiler and flags
CXX = clang++
CC = clang
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
CFLAGS = -Wall -Wextra -O2

# Include directories
INCLUDES = -I. -Ilib -I/opt/homebrew/include

# Library directories and libraries
LIBDIRS = -L/opt/homebrew/lib
LIBS = -lglfw -lfreetype -lassimp -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

# Target executable
TARGET = bin

# Source files - automatically find all .cpp and .c files
CPP_SOURCES = $(wildcard *.cpp) $(wildcard lib/*.cpp)
C_SOURCES = $(wildcard *.c) $(wildcard lib/*.c)

# Object files - all go in obj/ directory with directory prefixes to avoid conflicts
CPP_OBJECTS = $(patsubst %.cpp,obj/%.o,$(subst /,_,$(CPP_SOURCES)))
C_OBJECTS = $(patsubst %.c,obj/%.o,$(subst /,_,$(C_SOURCES)))
OBJECTS = $(CPP_OBJECTS) $(C_OBJECTS)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

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

# Clean target
clean:
	rm -rf obj $(TARGET)

# Rebuild target
rebuild: clean all

# Debug info
debug:
	@echo "CPP_SOURCES: $(CPP_SOURCES)"
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"

# Phony targets
.PHONY: all clean rebuild debug

# Name of the output executable
TARGET = mymalloc

# Compiler and flags
CC = gcc
CFLAGS = -g -Wall -fno-builtin

# Source files
SRCS = mymalloc.c

# Default rule
all: $(TARGET)

# Build rule
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean rule
clean:
	rm -f $(TARGET)


TARGET = yap
CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -pedantic -std=c99
LIBS = -lSDL3 -lm
SRCS = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS) $(DEPS)
	$(CC) $(CFLAGS) $(LIBS) $(SRCS) -o $@

run: $(TARGET)
	./$<

clean:
	-rm $(TARGET)

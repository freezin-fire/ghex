CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = ghex
SRC = ghex.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
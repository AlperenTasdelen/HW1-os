CC = gcc
CFLAGS = -Wall -Wextra -std=c99

SRCS = eshell.c
EXECUTABLE = eshell

.PHONY: all clean

all: clean $(EXECUTABLE)

$(EXECUTABLE): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
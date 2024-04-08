CC = g++
CFLAGS = -w -Wextra -std=c++11

SRCS = eshell.cpp
EXECUTABLE = eshell
INCLUDES = parser.c

.PHONY: all

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) -o $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)

run:
	make all
	./$(EXECUTABLE)
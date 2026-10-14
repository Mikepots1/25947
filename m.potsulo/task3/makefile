CC = gcc
CFLAGS = -O2 -Wall -Wextra
TARGET = setuid_test
SRC = 3.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
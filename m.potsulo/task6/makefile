CC = gcc
CFLAGS = -O2 -Wall -Wextra
TARGET = line_lookup_alarm
SRC = 6.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
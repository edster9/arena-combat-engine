# 3D Hardware Acceleration Test - Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lglfw -lGLEW -lGL -lm

TARGET = hw_test
SRC = main.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

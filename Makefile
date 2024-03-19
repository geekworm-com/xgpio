CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LIBS = -lgpiod

TARGET = xgpio_pwr

SRCS = xgpio_pwr.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

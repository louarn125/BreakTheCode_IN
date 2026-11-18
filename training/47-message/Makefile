CC     = gcc
CFLAGS = -Wall -Wextra -g

# Fréquence du signal en Hz
FREQ ?= 700

.PHONY: all run clean

all: morse

morse: morse.c
	$(CC) $(CFLAGS) $< -o $@ -lm

run: morse
	./morse message.wav $(FREQ)

clean:
	rm -f morse

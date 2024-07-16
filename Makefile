# Copyright Andrei-Marian Rusanescu 311CAb 2023-2024
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# define targets
TARGETS = image_editor

build: $(TARGETS)

image_editor: image_editor.c
	$(CC) $(CFLAGS) image_editor.c -lm -o image_editor

pack:
	zip -FSr 311CA_RusanescuAndreiMarian_Tema3.zip README Makefile *.c *.h

clean:
	rm -f $(TARGETS)

.PHONY: pack clean

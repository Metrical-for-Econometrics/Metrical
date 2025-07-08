#!/bin/sh
gcc `pkg-config --cflags gtk+-3.0 librsvg-2.0` -o main src/main.c src/assistant/chatbox.c src/assistant/assistant.c `pkg-config --libs gtk+-3.0 librsvg-2.0` -lfontconfig && ./main 
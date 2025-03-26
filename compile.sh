#/usr/bin/bash

gcc src/*.c\
	-Iinclude \
	$(pkg-config --cflags --libs sdl3) \
	-lreadline

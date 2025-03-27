CC = cc
SRC = $(wildcard src/*.c)
HR = $(wildcard include/*.h)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
SDL_C_FLAGS = $(shell pkg-config --cflags sdl3)
SDL_LIBS_FLAGS = $(shell pkg-config --libs sdl3)
CFLAGS = -Iinclude $(SDL_C_FLAGS) -Wall -Wextra -Werror
LDFLAGS = -lreadline $(SDL_LIBS_FLAGS)
NAME = MetallC64

all: $(NAME)
	
$(NAME): $(OBJ)
	mkdir -p programs/generated
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS)

build/%.o: src/%.c $(HR)
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf build

fclean: clean
	rm -rf programs/generated
	rm -rf $(NAME)

re: fclean all

.PHONY: clean

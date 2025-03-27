CC = cc
SRC = $(wildcard src/*.c)
HR = $(wildcard include/*.h)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
SDL_PATH = ./assets/SDL3
CFLAGS = -Iinclude
LDFLAGS = -lreadline -Llib -Wl,-rpath,lib -Wl,--enable-new-dtags -lSDL3 
NAME = MetallC64

all: dirs_set sdl $(NAME)

sdl:
	@cmake -B $(SDL_PATH)/build $(SDL_PATH)
	@cmake --build $(SDL_PATH)/build
	@cp -r $(SDL_PATH)/include/SDL3 include
	@cp -r $(SDL_PATH)/build/libSDL3* lib

dirs_set:
	@mkdir -p programs/generated
	@mkdir -p lib

dirs_rem:
	rm -rf programs/generated
	rm -rf lib

$(NAME): $(OBJ)
	@mkdir -p programs/generated
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS)

build/%.o: src/%.c $(HR)
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf build

fclean: clean dirs_rem
	rm -rf include/SDL3
	rm -rf $(SDL_PATH)/build
	rm -rf $(NAME)

re: fclean all

.PHONY: clean

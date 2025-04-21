CC = emcc 
SRC = $(wildcard src/*.c)
HR = $(wildcard include/*.h)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
SDL_PATH = ./assets/SDL3
CFLAGS = -Iinclude
LDFLAGS = -Llib -Wl,-rpath,lib -Wl,-lSDL3 
NAME = MetallC64.html

all: dirs_set sdl $(NAME)

sdl:
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
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS) -O3 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=ccall,cwrap -sEXPORTED_FUNCTIONS=_exec_clr,_exec_ldp,_exec_ram_dump,_main,_main_cycle

build/%.o: src/%.c $(HR)
	@mkdir -p $(dir $@)
	$(CC) -O3 -c $< -o $@ $(CFLAGS)

clean:
	rm -rf build

fclean: clean dirs_rem
	rm -rf include/SDL3
	rm -rf $(SDL_PATH)/build
	rm -rf $(NAME)

re: fclean all

.PHONY: clean

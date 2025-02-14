CC = cc
SRC = $(wildcard src/*.c)
HR = $(wildcard include/*.h)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
MLX_PATH = ./assets/MLX42
CFLAGS = -Iinclude -I$(MLX_PATH)/include/MLX42 -Werror -Wextra -Wall
LDFLAGS = $(MLX_PATH)/build/libmlx42.a -lreadline -lglfw
UNAME = $(shell uname)
NAME = MetallC64

ifeq ($(UNAME), Linux)
	LDFLAGS += -ldl -pthread -lm
endif
ifeq ($(UNAME), Darwin)
	READLINE_PATH = $(shell brew --prefix readline)
	GLFW_PATH = $(shell brew --prefix glfw)
	LDFLAGS += -L $(GLFW_PATH)/lib -L $(READLINE_PATH)/lib -I $(READLINE_PATH)/include -framework Cocoa -framework IOKit
endif

all: mlx $(NAME)

mlx:
	@cmake -B $(MLX_PATH)/build $(MLX_PATH)
	@cmake --build $(MLX_PATH)/build

$(NAME): $(OBJ)
	mkdir -p programs/generated
	$(CC) -o $(NAME) $(OBJ) $(LDFLAGS)

build/%.o: src/%.c $(HR)
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf build
	rm -rf $(MLX_PATH)/build

fclean: clean
	rm -rf programs/generated
	rm -rf $(NAME)

re: fclean all

.PHONY: clean

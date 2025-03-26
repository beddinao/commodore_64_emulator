CC = gcc
SRC = $(wildcard src/*.c)
HR = $(wildcard include/*.h)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
CFLAGS = -Iinclude $(shell pkg-config --cflags --libs sdl3) 
LDFLAGS = -lreadline 
UNAME = $(shell uname)
NAME = MetallC64

ifeq ($(UNAME), Darwin)
	READLINE_PATH = $(shell brew --prefix readline)
	LDFLAGS += -L $(READLINE_PATH)/lib -I $(READLINE_PATH)/include 
endif

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

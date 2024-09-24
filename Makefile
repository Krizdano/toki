CC=cc
CFLAGS=-g -Wall -Wextra -pedantic
OBJ=build/obj
BIN=build/bin/toki
SRCS=$(shell find src/ -type f -name '*.c')
OBJS=$(patsubst src/%.c, build/obj/%.o, $(SRCS))
RM=rm -rf

default: all

install: all

build: all

withflakesupport: CFLAGS=-g -Wall -Wextra -pedantic -D FLAKE_SUPPORT
withflakesupport: all

run: all
	@./$(BIN) $(filter-out $@,$(MAKECMDGOALS))

debug: all
	gdb $(BIN)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(OBJ)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) build/bin/* build/obj/*

%:
	@:

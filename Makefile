sources = main.c
bin = main
LDFLAGS += -lSDL

main: $(sources)

.PHONY: all
all: main

.PHONY: test
test: all
	./$(bin)

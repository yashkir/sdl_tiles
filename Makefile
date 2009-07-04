sources   = main.c
bin       = main

CFLAGS   += -ggdb
CPPFLAGS += -Wall
LDFLAGS  += -lSDL

main: $(sources)

.PHONY: all
all: main

.PHONY: test
test: all
	./$(bin)
.PHONY: debug
debug: all
	gdb ./$(bin)

.PHONY: clean
clean:
	@rm -f *.o

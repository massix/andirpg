# Simple Makefile, one day I will convert this to
# something more portable and complete, perhaps.

CFLAGS := -pedantic -ggdb -O0 -DMAX_INVENTORY_SIZE=20
EXTRA_CFLAGS = -std=gnu2x -xc -Wall -m64 -Isrc
CC := clang

SRCDIR = src
TESTDIR = test
OBJDIR = build
EXEDIR = rpg

BIN_SOURCES = $(wildcard $(SRCDIR)/*.c)
TEST_SOURCES = $(wildcard $(TESTDIR)/*.c)
EXE_SOURCES = $(wildcard $(EXEDIR)/*.c)

BIN_OBJECTS = $(BIN_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TEST_OBJECTS = $(TEST_SOURCES:$(TESTDIR)/%.c=$(OBJDIR)/%.o)
EXE_OBJECTS = $(EXE_SOURCES:$(EXEDIR)/%.c=$(OBJDIR)/%.o)

LIBRARIES = ncurses inih
TEST_CFLAGS = $(EXTRA_CFLAGS) $(CFLAGS) $(shell pkg-config --cflags cunit $(LIBRARIES))
MAIN_CFLAGS = $(EXTRA_CFLAGS) $(CFLAGS) $(shell pkg-config --cflags $(LIBRARIES))
TEST_LDFLAGS = $(shell pkg-config --libs cunit $(LIBRARIES))
MAIN_LDFLAGS = $(shell pkg-config --libs $(LIBRARIES))

all: build rpg.exe tests.exe
.PHONY: clean fclean

build/%.o: rpg/%.c
	$(CC) $(MAIN_CFLAGS) -c -o $@ $<

build/tests.o: test/tests.c
	$(CC) $(TEST_CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(MAIN_CFLAGS) -c -o $@ $<

$(OBJDIR)/%_tests.o: $(TESTDIR)/%_tests.c
	$(CC) $(TEST_CFLAGS) -c -o $@ $<

rpg.exe: $(EXE_OBJECTS) $(BIN_OBJECTS)
	$(CC) -o $@ $^ $(MAIN_LDFLAGS)

tests.exe: $(OBJDIR)/tests.o $(BIN_OBJECTS) $(TEST_OBJECTS)
	$(CC) -o $@ $^ $(TEST_LDFLAGS)

clean: fclean
	rm -f *.log unix_socket

fclean:
	rm -f rpg.exe tests.exe $(OBJDIR)/main.o $(OBJDIR)/tests.o $(OBJDIR)/setup.o $(BIN_OBJECTS) $(TEST_OBJECTS)


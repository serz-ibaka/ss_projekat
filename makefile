IDIR=inc
_DEPS=Assembler.hpp ParsedLine.hpp Parser.hpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

SRCDIR=src
_SRC=ParsedLine.cpp Assembler.cpp Parser.cpp main.cpp
SRC = $(patsubst %,$(SRCDIR)/%,$(_SRC))

TARGET = assembler

CC = g++
CFLAGS = -I$(IDIR)

assembler:
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

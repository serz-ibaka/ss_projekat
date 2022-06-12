IDIR=inc

_ASSEMBLER_DEPS=Assembler.hpp ParsedLine.hpp Parser.hpp
ASSEMBLER_DEPS = $(patsubst %,$(IDIR)/%,$(_ASSEMBLER_DEPS))

_LINKER_DEPS = Assembler.hpp Parser.hpp ParsedLine.hpp Linker.hpp
LINKER_DEPS = $(patsubst %,$(IDIR)/%,$(_LINKER_DEPS))

SRCDIR=src

_ASSEMBLER_SRC=ParsedLine.cpp Assembler.cpp Parser.cpp assembler_main.cpp
ASSEMBLER_SRC = $(patsubst %,$(SRCDIR)/%,$(_ASSEMBLER_SRC))

_LINKER_SRC=Linker.cpp ParsedLine.cpp Assembler.cpp Parser.cpp linker_main.cpp
LINKER_SRC = $(patsubst %,$(SRCDIR)/%,$(_LINKER_SRC))

_OBJDUMP_SRC=objdump_main.cpp
OBJDUMP_SRC = $(patsubst %,$(SRCDIR)/%,$(_OBJDUMP_SRC))

TARGET_ASSEMBLER = assembler
TARGET_LINKER = linker
TARGET_OBJDUMP = objdump

CC = g++
CFLAGS = -I$(IDIR)

all: assembler linker objdump

assembler:
	$(CC) -o $(TARGET_ASSEMBLER) $(ASSEMBLER_SRC) $(CFLAGS)

linker:
	$(CC) -o $(TARGET_LINKER) $(LINKER_SRC) $(CFLAGS)

objdump:
	$(CC) -o $(TARGET_OBJDUMP) $(OBJDUMP_SRC) $(CFLAGS)



CC = gcc
CFLAGS = -Wall -Wextra -pthread
LIBS = -lz -lncurses

SRC = src/main.c src/chunker.c src/compressor.c \
      src/writer.c src/decompressor.c src/sync.c src/gui.c

all:
	$(CC) $(CFLAGS) $(SRC) -o compressor $(LIBS)

clean:
	rm -f compressor *.o
ZIO ?= $(HOME)/zio

OBJ := libzattr.o libzio.o

libzio.a: $(OBJ)
	gcc -o "$@" $(OBJ)

%.o: %.c libzio.h
	gcc -I$(ZIO)/include -c -o "$@" "$<"
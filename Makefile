ZIO ?= $(HOME)/zio

OBJ := libzattr.o libzio.o libsysfs.o

libzio.a: $(OBJ)
	ar rcs libzio.a $(OBJ)

%.o: %.c libzio.h
	gcc -I$(ZIO)/include -ggdb -c -o "$@" "$<"

libsysfs.o: libsysfs.c libsysfs.h
	gcc -ggdb -c -o "$@" "$<"

clean:
	rm $(OBJ) libzio.a

.PHONY: clean

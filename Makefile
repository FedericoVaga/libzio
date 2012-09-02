ZIO ?= $(HOME)/zio

OBJ := libzattr.o libzio.o libsysfs.o

libzio.a: $(OBJ)
	ar rcs libzio.a $(OBJ)

%.o: %.c libzio.h
	gcc -I$(ZIO)/include -c -o "$@" "$<"
	
libsysfs.o: libsysfs.c libsysfs.h
	gcc -c -o "$@" "$<"
	
clean:
	rm $(OBJ) libzio.a

.PHONY: clean
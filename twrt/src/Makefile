CC = gcc
CFLAGS = -O2 -g -llib

OBJ_TEST = test.o string_ex.o
OBJ_TWRT = twrt.o config.o string_ex.o
twrt: $(OBJ_TWRT)
	$(CC) -o twrt $(OBJ_TWRT)
test: $(OBJ_TEST)
	$(CC) -o test $(OBJ_TEST)
test.o: test.c string_ex.h
	$(CC) $(CFLAGS) -c test.c -o test.o
string_ex.o: string_ex.c string_ex.h
	$(CC) $(CFLAGS) -c string_ex.c -o string_ex.o
config.o: config.c config.h string_ex.c string_ex.h
	$(CC) $(CFLAGS) -c config.c -o config.o
.PHONY: clean
clean: 
	rm $(OBJ_TEST)
	rm $(OBJ_TWRT)
	rm test
	rm twrt

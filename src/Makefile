CC = gcc
CFLAGS = -O2 -g -llib

OBJ_TEST = test.o string_ex.o
test: $(OBJ_TEST)
	$(CC) -o test $(OBJ_TEST)
test.o: test.c string_ex.h
	$(CC) $(CFLAGS) -c test.c -o test.o
string_ex.o: string_ex.c string_ex.h
	$(CC) $(CFLAGS) -c string_ex.c -o string_ex.o
.PHONY: clean
clean: 
	rm $(OBJ_TEST)
	rm test

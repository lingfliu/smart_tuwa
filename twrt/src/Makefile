CC = gcc
CFLAGS = -O2 -g 
LIBS = -lpthread

OBJ = test.o buffer_ex.o 

test: $(OBJ)
	$(CC) test.o buffer_ex.o -o test 
test.o: test.c
	$(CC) $(CFLAGS) $(LIBS) -c test.c -o test.o
buffer_ex.o: buffer_ex.c buffer_ex.h
	$(CC) $(CFLAGS) -c buffer_ex.c -o buffer_ex.o

.PHONY: clean
clean: 
	rm $(OBJ)
	rm test

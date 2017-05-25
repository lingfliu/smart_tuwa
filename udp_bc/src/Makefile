CC = gcc
CFLAGS = -O0 -g
LIBS = -pthread

OBJ = udp_bc.o 

udp_bc: $(OBJ)
	$(CC) $(OBJ) -o udp_bc $(LIBS)  
udp_bc.o: udp_bc.c udp_bc.h 
	$(CC) $(CFLAGS) -c udp_bc.c -o udp_bc.o $(LIBS) 

.PHONY: clean
clean: 
	rm $(OBJ)
	rm udp_bc

CC = gcc
CFLAGS = -O2 -g 
LIBS = -pthread

OBJ = twrt.o config.o string_ex.o simple_serial.o simple_inet.o proc.o sys.o buffer_ex.o 

twrt: $(OBJ)
	$(CC) $(OBJ) -o twrt $(LIBS)  
twrt.o: twrt.c twrt.h proc.c proc.h sys.c sys.h
	$(CC) $(CFLAGS) -c twrt.c -o twrt.o $(LIBS) 
config.o: config.c config.h string_ex.c string_ex.h
	$(CC) $(CFLAGS) -c config.c -o config.o
string_ex.o: string_ex.c string_ex.h
	$(CC) $(CFLAGS) -c string_ex.c -o string_ex.o
buffer_ex.o: buffer_ex.c buffer_ex.h
	$(CC) $(CFLAGS) -c buffer_ex.c -o buffer_ex.o
simple_serial.o: simple_serial.c simple_serial.h
	$(CC) $(CFLAGS) -c simple_serial.c -o simple_serial.o
simple_inet.o: simple_inet.c simple_inet.h
	$(CC) $(CFLAGS) -c simple_inet.c -o simple_inet.o
proc.o: proc.c proc.h buffer_ex.c buffer_ex.h string_ex.c string_ex.h 
	$(CC) $(CFLAGS) -c proc.c -o proc.o
sys.o: sys.c sys.h proc.c proc.h
	$(CC) $(CFLAGS) -c sys.c -o sys.o

.PHONY: clean
clean: 
	rm $(OBJ)
	rm twrt

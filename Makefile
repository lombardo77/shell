CFLAGS= -g -Wall -Wvla -fsanitize=address 

all:
	make allobj
	gcc shell.o buffer_manipulation.o -o shell

allobj: buffer_manipulation.c shell.c
	gcc buffer_manipulation.c -c 
	gcc shell.c -c 

debug:
	make allobj
	gcc $(CFLAGS) shell.o buffer_manipulation.o -o shell



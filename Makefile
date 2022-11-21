CFLAGS= -g -Wall -Wvla -fsanitize=address 

all:
	make allobj
	gcc shell.o buffer_manipulation.o joblists.o -o shell

allobj: buffer_manipulation.c shell.c joblists.c
	gcc buffer_manipulation.c -c 
	gcc shell.c -c
	gcc joblists.c -c

debug:
	make allobj
	gcc $(CFLAGS) shell.o buffer_manipulation.o -o shell



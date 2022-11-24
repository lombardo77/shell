CFLAGS= -g -Wall -Wvla -fsanitize=address 
LINKED= shell.o buffer_manipulation.o joblists.o  

all:
	make allobj
	gcc $(LINKED) -o shell

allobj: buffer_manipulation.c shell.c joblists.c
	gcc buffer_manipulation.c -c 
	gcc shell.c -c
	gcc joblists.c -c

debug: buffer_manipulation.c shell.c joblists.c
	make allobj
	gcc $(CFLAGS) $(LINKED)-o shell

clean:
	rm *.o

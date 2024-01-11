prog:	main.o htable.o
	gcc main.o htable.o -o advcalc
	
main.o:	main.c defs.h
		gcc -c main.c -o main.o

htable.o:	htable.c defs.h
		gcc -c htable.c

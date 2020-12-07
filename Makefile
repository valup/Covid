CC = gcc
FLAGS = -Wall -std=c99 -Wextra
VFLAGS = --tool=memcheck --leak-check=full -v

shell: shell.c
	$(CC) -c shell.c -o $(FLAGS) shell

ltree.o: ltree.c ltree.h
	$(CC) -c ltree.c

hashl.o: hashl.c hashl.h
	$(CC) -c hashl.c

straux.o: straux.c straux.h
	$(CC) -c straux.c

leaks: $(SHELL)
	valgrind $(VFLAGS) ./$(SHELL)

clean:
	rm *.o


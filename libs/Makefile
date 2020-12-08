CC = gcc
FLAGS = -Wall -std=c99 -Wextra
VFLAGS = --tool=memcheck --leak-check=full
EXE = shell

shell: shell.c comandos.o
	$(CC) shell.c comandos.o $(FLAGS) -o $(EXE)

comandos.o: libs/comandos.c libs/comandos.h libs/hashf.h libs/ltree.h
	$(CC) -c libs/comandos.c

ltree.o: libs/ltree.c libs/ltree.h
	$(CC) -c libs/ltree.c

hashf.o: libs/hashf.c libs/hashf.h libs/hashl.h libs/straux.h
	$(CC) -c libs/hashf.c

hashl.o: libs/hashl.c libs/hashl.h
	$(CC) -c libs/hashl.c

straux.o: libs/straux.c libs/straux.h
	$(CC) -c libs/straux.c

leaks: $(SHELL)
	valgrind $(VFLAGS) ./$(EXE)

clean:
	rm *.o

CC = gcc
FLAGS = -Wall -Wextra -Werror -std=c99 -g
VFLAGS = --tool=memcheck --leak-check=full
EXE = shell
COMANDOS = comandos.o
LTREE = ltree.o
HASHF = hasf.o
HASHL = hasl.o
STRAUX = straux.o

main: shell.c $(COMANDOS) $(LTREE) $(HASHF) $(HASHL) $(STRAUX)
	$(CC) $(FLAGS) shell.c $(COMANDOS) $(LTREE) $(HASHF) $(HASHL) $(STRAUX) -o $(EXE)

$(COMANDOS): libs/comandos.c
	$(CC) $(FLAGS) libs/comandos.c -c -o $(COMANDOS)

$(LTREE): libs/ltree.c
	$(CC) $(FLAGS) libs/ltree.c -c -o $(LTREE)

$(HASHF): libs/hashf.c
	$(CC) $(FLAGS) libs/hashf.c -c -o $(HASHF)

$(HASHL): libs/hashl.c
	$(CC) $(FLAGS) libs/hashl.c -c -o $(HASHL)

$(STRAUX): libs/straux.c
	$(CC) $(FLAGS) libs/straux.c -c -o $(STRAUX)

leaks: $(EXE)
	valgrind $(VFLAGS) ./$(EXE)

clean:
	rm *.o
	rm *.temp

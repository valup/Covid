all: shell

leaks: shell
	valgrind --tool=memcheck --leak-check=full  --show-leak-kinds=all --track-origins=yes -v ./shell

try: shell
	./shell
	rm shell
	

shell: shell.c comandos.c comandos.h straux.c straux.h ltree.c ltree.h hashf.c hashf.h hashl.c hashl.h
	gcc -Wall -std=c99 -Wextra comandos.c straux.c ltree.c hashl.c hashf.c shell.c -o shell

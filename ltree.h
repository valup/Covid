#ifndef __LTREE_H__
#define __LTREE_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

typedef void (*FuncionVisitante) (char* lugar);

typedef struct _LTNodo {
  wchar_t* lugar;
  struct _LTNodo *left;
  struct _LTNodo *right;
} LTNodo;

typedef LTNodo *LTree;

/* Solo se exportan las funciones necesarias para el programa */

/* Inicializa un arbol */
LTree ltree_crear();

/* Recibe un arbol y un puntero a string,
si no halla el string en el arbol lo inserta,
sino libera el string y lo reemplaza por el preexistente en el puntero */
LTree ltree_insertar(LTree lt, wchar_t** lugar);

/* Destruye un arbol y todos sus strings recursivamente */
void ltree_destruir(LTree lt);

#endif

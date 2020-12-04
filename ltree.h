#ifndef __LTREE_H__
#define __LTREE_H__

typedef void (*FuncionVisitante) (char* lugar);

typedef struct _LTNodo {
  char* lugar;
  struct _LTNodo *left;
  struct _LTNodo *right;
} LTNodo;

typedef LTNodo *LTree;

/* Solo se exportan las funciones necesarias para el programa */

LTree ltree_crear();

LTree ltree_insertar(LTree lt, char** lugar);

void ltree_destruir(LTree lt);

#endif

#ifndef __LTREE_H__
#define __LTREE_H__

typedef void (*FuncionVisitante) (char* lugar);

typedef struct _LTNodo {
  char* lugar;
  struct _LTNodo *left;
  struct _LTNodo *right;
} LTNodo;

typedef LTNodo *LTree;

LTree ltree_crear();

void ltree_print(LTree lt);

LTree ltree_insertar(LTree lt, char** lugar);

LTree ltree_eliminar(LTree lt, char* lugar);

int ltree_contiene(LTree lt, char* lugar);

int ltree_nelementos(LTree lt);

int ltree_altura(LTree lt);

void ltree_recorrer(LTree lt, FuncionVisitante visit);

void ltree_imprimir(LTree lt);

char* ltree_minimo(LTree lt);

char* ltree_acceder(LTree lt, int indice);

void ltree_destruir(LTree lt);

#endif

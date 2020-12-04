#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ltree.h"

#define CONT 10

/* Inicializa un arbol */
LTree ltree_crear() {
  return NULL;
}

/* Funcion de maximo */
int max(int a, int b) {
  return (a > b)? a : b;
}

/* Calcula recursivamente la altura del arbol */
int ltree_altura(LTree lt) {
  if (lt == NULL)
    return -1;
  return 1 + max(ltree_altura(lt->left), ltree_altura(lt->right));
}

/* Devuelve el minimo string en el arbol
(el mas a la izquierda) */
char* ltree_minimo(LTree lt) {
  /* Si es arbol vacio no hay minimo */
  if (!lt)
    return NULL;

  if (!lt->left)
    return lt->lugar;

  return ltree_minimo(lt->left);
}

/* Funcion de rotacion derecha para balanceo */
LTree ltree_rotar_der(LTree lt) {
  LTree left = lt->left;
  LTree lr = left->right;

  left->right = lt;
  lt->left = lr;

  return left;
}

/* Funcion de rotacion izquierda para balanceo */
LTree ltree_rotar_izq(LTree lt) {
  LTree right = lt->right;
  LTree rl = right->left;

  right->left = lt;
  lt->right = rl;

  return right;
}

/* Recibe un arbol y un puntero a string,
si no halla el string en el arbol lo inserta,
sino libera el string y lo reemplaza por el preexistente en el puntero */
LTree ltree_insertar(LTree lt, char** lugar) {
  /* Si el arbol es vacio inserta el string en el primer nodo */
  if (!lt) {
    LTree nodo = malloc(sizeof(LTNodo));
    nodo->lugar = *lugar;
    nodo->left = NULL;
    nodo->right = NULL;
    return nodo;
  }

  /* Si el string es alfabeticamente menor al del nodo,
  sigue buscando a la izquierda */
  if (strcmp(*lugar, lt->lugar) < 0)
    lt->left = ltree_insertar(lt->left, lugar);
  /* Si es mayor, a la derecha */
  else if (strcmp(*lugar, lt->lugar) > 0)
    lt->right = ltree_insertar(lt->right, lugar);
  /* Sino es el mismo, asi que se lo reemplaza */
  else {
    free(*lugar);
    *lugar = lt->lugar;
    return lt;
  }

  /* Se calcula el balance del arbol */
  int dif = ltree_altura(lt->left) - ltree_altura(lt->right);
  /* Si es necesario se balancea */
  if (dif > 1) {
    if (strcmp(*lugar, lt->left->lugar) > 0)
      lt->left = ltree_rotar_izq(lt->left);
    return ltree_rotar_der(lt);

  } else if (dif < -1) {
    if (strcmp(*lugar, lt->right->lugar) < 0)
      lt->right = ltree_rotar_der(lt->right);
    return ltree_rotar_izq(lt);
  }

  return lt;
}

/* Destruye un arbol y todos sus strings recursivamente */
void ltree_destruir(LTree lt) {
  if (!lt)
    return;

  //printf("Elimino %s\n", lt->lugar);
  free(lt->lugar);
  ltree_destruir(lt->left);
  ltree_destruir(lt->right);
  free(lt);
}

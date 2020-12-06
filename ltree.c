#include "ltree.h"

LTree ltree_crear() {
  return NULL;
}

/* Funcion de maximo */
int max(int a, int b) {
  return (a > b)? a : b;
}

/* Calcula recursivamente la altura del arbol */
int ltree_altura(LTree lt) {
  if (!lt)
    return -1;

  return 1 + max(ltree_altura(lt->left), ltree_altura(lt->right));
}

/* Devuelve el minimo string en el arbol
(el mas a la izquierda) */
wchar_t* ltree_minimo(LTree lt) {

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

LTree ltree_insertar(LTree lt, wchar_t** lugar) {

  if (!lt) {
    LTree nodo = malloc(sizeof(LTNodo));
    nodo->lugar = *lugar;
    nodo->left = NULL;
    nodo->right = NULL;
    return nodo;
  }

  if (wcscoll(*lugar, lt->lugar) < 0)
    lt->left = ltree_insertar(lt->left, lugar);

  else if (wcscoll(*lugar, lt->lugar) > 0)
    lt->right = ltree_insertar(lt->right, lugar);

  /* Si encuentra el mismo lugar lo reemplaza por el preexistente */
  else {
    free(*lugar);
    *lugar = lt->lugar;
    return lt;
  }

  /* Se calcula el balance del arbol */
  int dif = ltree_altura(lt->left) - ltree_altura(lt->right);

  if (dif > 1) {
    if (wcscoll(*lugar, lt->left->lugar) > 0)
      lt->left = ltree_rotar_izq(lt->left);
    return ltree_rotar_der(lt);

  } else if (dif < -1) {
    if (wcscoll(*lugar, lt->right->lugar) < 0)
      lt->right = ltree_rotar_der(lt->right);
    return ltree_rotar_izq(lt);
  }

  return lt;
}

void ltree_destruir(LTree lt) {
  if (!lt)
    return;

  free(lt->lugar);
  ltree_destruir(lt->left);
  ltree_destruir(lt->right);
  free(lt);
}

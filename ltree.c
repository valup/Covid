#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ltree.h"

#define CONT 10

LTree ltree_crear() {
  return NULL;
}

int max(int a, int b) {
  return (a > b)? a : b;
}

int ltree_altura(LTree lt) {
  if (lt == NULL)
    return -1;
  return 1 + max(ltree_altura(lt->left), ltree_altura(lt->right));
}

void ltree_print_aux(LTree lt, int espacio) {
    if (lt == NULL)
        return;
    // Increase distance between levels
    espacio += CONT;

    ltree_print_aux(lt->right, espacio);

    // Print current node after space
    // count
    printf("\n");
    for (int i = CONT; i < espacio; i++)
        printf(" ");
    printf("%s\n", lt->lugar);

    ltree_print_aux(lt->left, espacio);
}

void ltree_print(LTree lt) {
   // Pass initial space count as 0
   ltree_print_aux(lt, 0);
}

char* ltree_minimo(LTree lt) {
  if (lt == NULL) {
    printf("ERROR: arbol vacio\n");
    return NULL;
  }
  if (lt->left == NULL)
    return lt->lugar;
  return ltree_minimo(lt->left);
}

LTree ltree_rotar_der(LTree lt) {
  LTree left = lt->left;
  LTree lr = left->right;

  left->right = lt;
  lt->left = lr;

  return left;
}

LTree ltree_rotar_izq(LTree lt) {
  LTree right = lt->right;
  LTree rl = right->left;

  right->left = lt;
  lt->right = rl;

  return right;
}

LTree ltree_insertar(LTree lt, char** lugar) {
  if (!lt) {
    LTree nodo = malloc(sizeof(LTNodo));
    nodo->lugar = *lugar;
    nodo->left = NULL;
    nodo->right = NULL;
    return nodo;
  }

  if (strcmp(*lugar, lt->lugar) < 0)
    lt->left = ltree_insertar(lt->left, lugar);
  else if (strcmp(*lugar, lt->lugar) > 0)
    lt->right = ltree_insertar(lt->right, lugar);
  else {
    free(*lugar);
    *lugar = lt->lugar;
    return lt;
  }

  int dif = ltree_altura(lt->left) - ltree_altura(lt->right);

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

LTree ltree_eliminar(LTree lt, char* lugar) {
  if (lt == NULL)
    return lt;

  if (strcmp(lugar, lt->lugar) < 0) {
    lt->left = ltree_eliminar(lt->left, lugar);
  } else if (strcmp(lugar, lt->lugar) > 0) {
    lt->right = ltree_eliminar(lt->right, lugar);
  } else {
    if (lt->right == NULL) {
      LTree temp = lt;
      lt = lt->left;
      free(temp);
    } else {
      char* nuevoDato = ltree_minimo(lt->right);
      lt->lugar = nuevoDato;
      lt->right = ltree_eliminar(lt->right, nuevoDato);
    }
  }

  if (lt == NULL)
    return lt;

  int dif = ltree_altura(lt->left) - ltree_altura(lt->right);

  if (dif > 1) {
    if (ltree_altura(lt->left->left) - ltree_altura(lt->left->right) < 0)
      lt->left = ltree_rotar_izq(lt->left);
    return ltree_rotar_der(lt);
  } else if (dif < -1) {
    if (ltree_altura(lt->right->left) - ltree_altura(lt->right->right) > 0)
      lt->right = ltree_rotar_der(lt->right);
    return ltree_rotar_izq(lt);
  }
  return lt;
}

int ltree_contiene(LTree lt, char* lugar) {
  if (lt == NULL)
    return 0;
  if (!strcmp(lugar, lt->lugar))
    return 1;
  if (strcmp(lugar, lt->lugar) < 0) {
    return ltree_contiene(lt->left, lugar);
  } else {
    return ltree_contiene(lt->right, lugar);
  }
}

int ltree_nelementos(LTree lt) {
  if (lt == NULL)
    return 0;
  return 1 + ltree_nelementos(lt->left) + ltree_nelementos(lt->right);
}

void ltree_recorrer(LTree lt, FuncionVisitante visit) {
  if (lt == NULL)
    return;
  ltree_recorrer(lt->left, visit);
  visit(lt->lugar);
  ltree_recorrer(lt->right, visit);
}

void imprimir_string(char* data) {
  printf("%s ", data);
}

void ltree_imprimir(LTree lt) {
  ltree_recorrer(lt, imprimir_string);
}

char* ltree_acceder_aux(LTree lt, int indice, int* actual) {
  char* lugar;
  if (lt->left != NULL)
    lugar = ltree_acceder_aux(lt->left, indice, actual);
  (*actual) = (*actual) + 1;
  if (indice < *actual) {
    return lugar;
  } else if (indice == *actual) {
    return lt->lugar;
  } else if (lt->right != NULL) {
    return ltree_acceder_aux(lt->right, indice, actual);
  }
  return NULL;
}

char* ltree_acceder(LTree lt, int indice) {
  if (lt == NULL) {
    printf("ERROR: arbol vacio\n");
    return NULL;
  }

  int actual = -1;
  return ltree_acceder_aux(lt, indice, &actual);
}

void ltree_destruir(LTree lt) {
  if (lt == NULL)
    return;

  //printf("Elimino %s\n", lt->lugar);
  free(lt->lugar);
  ltree_destruir(lt->left);
  ltree_destruir(lt->right);
  free(lt);
}

#include "hashl.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Recibe una capacidad y funciones para hash, paso y comparacion
y crea una tabla de conjuntos con esa informacion */
Lugares* lugares_crear(unsigned capacidad, FuncionHash hash, FuncionPaso paso) {
  Lugares* tabla = malloc(sizeof(Lugares));
  tabla->hash = hash;
  tabla->paso = paso;
  tabla->numElems = 0;
  tabla->capacidad = capacidad;
  tabla->lugares = malloc(sizeof(CasillaHash) * capacidad);

  /* se inicializan las casillas con datos nulos */
  for (unsigned idx = 0; idx < capacidad; ++idx) {
    tabla->lugares[idx].lugar = NULL;
    tabla->lugares[idx].notifs = NULL;
  }

  return tabla;
}

/* Recibe una tabla de conjuntos, una lugar y un nuevo conjunto
e inserta el conjunto en la tabla, asociado a la lugar dada */
void lugares_insertar(Lugares* tabla, char* lugar, int* notifs) {
  /* si el factor de carga supera el limite establecido
  se redimensiona la tabla para duplicar la capacidad
  esto significa que la tabla nunca se llena */
  if ((float) tabla->numElems / (float) tabla->capacidad > LIM)
    lugares_redimensionar(tabla);

  /* se calcula la posición de la lugar dada de acuerdo a la función hash */
  unsigned idx = tabla->hash(lugar) % tabla->capacidad;
  //printf("idx = %d\n", idx);

  /* se recorre la tabla hasta hallar una casilla vacia
  y gracias a la redimension siempre habra lugar */
  while (tabla->lugares[idx].notifs) {
    if (!strcmp(tabla->lugares[idx].lugar, lugar)) {
      //free(lugar);
      free(tabla->lugares[idx].notifs);
      tabla->lugares[idx].notifs = notifs;
      return;
    }
    idx = (idx + tabla->paso(lugar)) % tabla->capacidad;
  }
  /* si se inserta el conjunto en una casilla nueva
  aumenta el numero total de elementos */
  tabla->numElems++;
  tabla->lugares[idx].lugar = lugar;
  tabla->lugares[idx].notifs = notifs;
  //printf("%s - ", tabla->lugares[idx].lugar);
  //printf("%d %d %d - ", tabla->lugares[idx].notifs[0], tabla->lugares[idx].notifs[1], tabla->lugares[idx].notifs[2]);
}

/* Recibe una tabla de conjuntos y una lugar
busca el conjunto asociado en la tabla y retorna un puntero al mismo
o en caso de no hallarlo retorna un puntero nulo */
int* lugares_buscar(Lugares* tabla, char* lugar) {
  /* se calcula la posición de la lugar dada de acuerdo a la función hash */
  unsigned idx = tabla->hash(lugar) % tabla->capacidad;
  /* se recorren las posiciones posibles en la tabla
  con la funcion paso para buscar el conjunto
  el valor de la funcion de paso no debe ser divisor del tam
  de la tabla para poder recorrerla toda y no volver al principio */
  while (tabla->lugares[idx].lugar) {
    if (!strcmp(tabla->lugares[idx].lugar, lugar))
      return tabla->lugares[idx].notifs;
    idx = (idx + tabla->paso(lugar)) % tabla->capacidad;
  }
  return NULL;
}

void lugares_eliminar(Lugares* tabla, char* lugar) {
  /* se calcula la posición de la clave dada de acuerdo a la función hash */
  unsigned idx = tabla->hash(lugar) % tabla->capacidad;
  /* se recorren las posiciones posibles en la tabla
  con la funcion paso para buscar el conjunto
  el valor de la funcion de paso no debe ser divisor del tam
  de la tabla para poder recorrerla toda y no volver al principio */
  while (tabla->lugares[idx].notifs) {
    if (!strcmp(tabla->lugares[idx].lugar, lugar)) {
      free(tabla->lugares[idx].notifs);
    }
    idx = (idx + tabla->paso(lugar)) % tabla->capacidad;
  }
}

/* Recibe una tabla de conjuntos y duplica su capacidad
luego rehashea todos los elementos para insertarlos
en la nueva tabla */
void lugares_redimensionar(Lugares* tabla) {
  tabla->capacidad *= 2;
  /* se crea e inicializa una tabla nueva
  con el doble de tam de la anterior */
  CasillaHash* tablaNueva = malloc(sizeof(CasillaHash) * tabla->capacidad);
  for (size_t i = 0; i < tabla->capacidad; i++) {
    tablaNueva[i].lugar = NULL;
    tablaNueva[i].notifs = NULL;
  }

  unsigned idx;
  /* se recorre la tabla original en orden
  hasta haber movido la cantidad de elementos que tenia */
  for (size_t i = 0, j = 0; j < tabla->numElems; i++) {
    if (tabla->lugares[i].lugar) {
      j++;
      idx = tabla->hash(tabla->lugares[i].lugar) % tabla->capacidad;
      while (tablaNueva[idx].lugar)
        idx = (idx + tabla->paso(tabla->lugares[i].lugar)) % tabla->capacidad;
      tablaNueva[idx].lugar = tabla->lugares[i].lugar;
      tablaNueva[idx].notifs = tabla->lugares[i].notifs;
    }
  }

  CasillaHash* temp = tabla->lugares;
  tabla->lugares = tablaNueva;
  free(temp);
}

/* Destruye la tabla y todos sus conjuntos */
void lugares_destruir(Lugares* tabla) {
  /* se recorre la tabla en orden hasta eliminar
  la cantidad de conjuntos que tenia */
  for (size_t i = 0, j = 0; j < tabla->numElems; i++) {
    if (tabla->lugares[i].lugar) {
      j++;
      free(tabla->lugares[i].notifs);
    }
  }
  free(tabla->lugares);
  free(tabla);
}

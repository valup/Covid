#include "hashl.h"

Lugares* lugares_crear(unsigned capacidad, FuncionHash hash, FuncionPaso paso) {

  Lugares* tabla = malloc(sizeof(Lugares));
  tabla->hash = hash;
  tabla->paso = paso;
  tabla->numElems = 0;
  tabla->capacidad = capacidad;
  tabla->lugares = malloc(sizeof(CasillaLugar) * capacidad);

  for (unsigned idx = 0; idx < capacidad; ++idx) {
    tabla->lugares[idx].lugar = NULL;
    tabla->lugares[idx].notifs = NULL;
  }

  return tabla;
}

void lugares_insertar(Lugares* tabla, wchar_t* lugar, int* notifs) {

  /* si el factor de carga supera el limite establecido
  se redimensiona la tabla para duplicar la capacidad,
  esto significa que la tabla nunca se llena */
  if ((float) tabla->numElems / (float) tabla->capacidad > LIM)
    lugares_redimensionar(tabla);

  unsigned idx = tabla->hash(lugar) % tabla->capacidad;

  for (;tabla->lugares[idx].lugar;
    idx = (idx + tabla->paso(lugar)) % tabla->capacidad) {

    if (!wcscoll(tabla->lugares[idx].lugar, lugar)) {
      free(tabla->lugares[idx].notifs);
      /* No se libera el lugar porque primero se lo busca en el arbol
      y si ya existia se remplaza por el string preexistente,
      asi que ambos strings apuntan al mismo espacio de memoria */
      tabla->lugares[idx].notifs = notifs;
      return;
    }
  }

  tabla->numElems++;
  tabla->lugares[idx].lugar = lugar;
  tabla->lugares[idx].notifs = notifs;
}

int* lugares_buscar(Lugares* tabla, wchar_t* lugar) {

  unsigned idx = tabla->hash(lugar) % tabla->capacidad;

  /* se recorren las posiciones posibles en la tabla
  con la funcion paso para buscar las notificaciones,
  el valor de la funcion de paso no debe ser divisor del tam
  de la tabla para poder recorrerla toda antes de volver al principio */
  for (;tabla->lugares[idx].lugar;
    idx = (idx + tabla->paso(lugar)) % tabla->capacidad) {
    /* Se compara por lugar porque las casillas eliminadas
    tienen puntero nulo a notificaciones pero mantienen el lugar */
    if (!wcscoll(tabla->lugares[idx].lugar, lugar))
      return tabla->lugares[idx].notifs;
  }
  return NULL;
}

void lugares_eliminar(Lugares* tabla, wchar_t* lugar) {

  unsigned idx = tabla->hash(lugar) % tabla->capacidad;

  for (;tabla->lugares[idx].lugar;
    idx = (idx + tabla->paso(lugar)) % tabla->capacidad) {
    /* Se compara por lugar porque las casillas eliminadas
    tienen puntero nulo a notificaciones pero mantienen el lugar */
    if (!wcscoll(tabla->lugares[idx].lugar, lugar)
      && tabla->lugares[idx].notifs) {

      tabla->numElems--;
      free(tabla->lugares[idx].notifs);
      tabla->lugares[idx].notifs = NULL;
      /* Se mantiene el string de lugar como marca de casilla eliminada
      para no detener la busqueda futura de otros lugares */
    }
  }
}

void lugares_redimensionar(Lugares* tabla) {

  tabla->capacidad *= 2;
  /* se crea e inicializa una tabla nueva
  con el doble de tam de la anterior */
  CasillaLugar* tablaNueva = malloc(sizeof(CasillaLugar) * tabla->capacidad);
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

  CasillaLugar* temp = tabla->lugares;
  tabla->lugares = tablaNueva;
  free(temp);
}

void lugares_destruir(Lugares* tabla) {

  /* se recorre la tabla en orden hasta eliminar
  la cantidad de entradas que tenia */
  for (size_t i = 0, j = 0; j < tabla->numElems; i++) {

    if (tabla->lugares[i].lugar) {
      j++;
      free(tabla->lugares[i].notifs);
      /* No se libera el lugar porque el string apunta al mismo
      espacio de memoria que el arbol de lugares asi que se libera
      cuando se destruya el arbol */
    }
  }
  free(tabla->lugares);
  free(tabla);
}

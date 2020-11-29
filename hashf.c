#include "hashf.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "straux.h"

/* Funcion de hasheo de forma rolling hash polinomial
explicada en el informe */
unsigned hash(char* str) {
  unsigned p = 53, pow = 1, len = strlen(str), total = 0;
  for (long i = 0; i < len; i++) {
    //printf("%ud ^ %ud = %f\n", p, i, pow(p, i));
    //printf("%f + %f = %f\n", total, (float) str[i] * pow(p, i), total + str[i] * pow(p, i));
    total += (unsigned) str[i] * pow;
    //printf("total = %f, %f, %f\n", total, (float) str[i], pow(p, i));
    //printf("total = %d\n", total);
    pow *= p;
  }
  return (unsigned) total;
}

/* Funcion de paso para hashing doble
que retorna el valor ASCII de la primera letra
de la clave */
unsigned paso(char* str) {
  return (unsigned) str[0];
}

void actualizar_fecha(struct tm* dest, struct tm* fuente) {
  dest->tm_year = fuente->tm_year;
  dest->tm_mon = fuente->tm_mon;
  dest->tm_mday = fuente->tm_mday;
}

void agregar_dias(struct tm* fecha, int dias) {
    time_t segundos = mktime(fecha) + (dias * 24 * 60 * 60);

    *fecha = *localtime(&segundos);
}

int igual_fecha(struct tm* f1, struct tm* f2) {
  return (f1->tm_year == f2->tm_year
    && f1->tm_mon == f2->tm_mon
    && f1->tm_mday == f2->tm_mday);
}

int dias(struct tm* f1, struct tm* f2) {
  return difftime(mktime(f1), mktime(f2))/(24.0*60*60);
}

/* Recibe una capacidad y funcion para calcular distancia en dias
y crea una tabla de fechas con esa informacion */
Fechas* fechas_crear(int capacidad) {
  Fechas* tabla = malloc(sizeof(Fechas));
  tabla->capacidad = capacidad;
  tabla->numElems = 0;
  tabla->ref = NULL;
  tabla->fechas = malloc(sizeof(CasillaFecha) * capacidad);

  /* se inicializan las casillas con datos nulos */
  for (int idx = 0; idx < capacidad; ++idx) {
    tabla->fechas[idx].fecha = NULL;
    tabla->fechas[idx].tabla = NULL;
    tabla->fechas[idx].sig = NULL;
  }

  return tabla;
}

/* Recibe una tabla de conjuntos, una clave y un nuevo conjunto
e inserta el conjunto en la tabla, asociado a la clave dada */
void fechas_insertar(Fechas* tabla, char* lugar, struct tm* fecha, int* notifs) {
  if (tabla->numElems == 0) {
    tabla->ref = malloc(sizeof(struct tm));
    memset(tabla->ref, 0, sizeof(struct tm));
    //printf("%d-%d-%d\n", tabla->ref->tm_year+1900, tabla->ref->tm_mon+1, tabla->ref->tm_mday);
    actualizar_fecha(tabla->ref, fecha);
  }

  int idx = abs(dias(fecha, tabla->ref));

  if (idx >= tabla->capacidad) {
    tabla->capacidad *= 2;
    tabla->fechas = realloc(tabla->fechas, tabla->capacidad);
  }

  CasillaFecha* casilla = &tabla->fechas[idx];

  if (casilla->fecha) {
    if (igual_fecha(casilla->fecha, fecha)) {
      free(casilla->fecha);
      casilla->fecha = fecha;
      if (!casilla->tabla)
        casilla->tabla = lugares_crear(LUGARES_INI, hash, paso);
      lugares_insertar(casilla->tabla, lugar, notifs);
      //printf("%d-%d-%d\n", casilla->fecha->tm_year+1900, casilla->fecha->tm_mon+1, casilla->fecha->tm_mday);
      return;
    }
    if (casilla->sig) {
      casilla = casilla->sig;
      free(casilla->fecha);
      casilla->fecha = fecha;
      if (!casilla->tabla)
        casilla->tabla = lugares_crear(LUGARES_INI, hash, paso);
      lugares_insertar(casilla->tabla, lugar, notifs);
      //printf("%d-%d-%d\n", casilla->fecha->tm_year+1900, casilla->fecha->tm_mon+1, casilla->fecha->tm_mday);
      return;
    }
    casilla->sig = malloc(sizeof(CasillaFecha));
    casilla = casilla->sig;
    casilla->sig = NULL;
  }

  tabla->numElems++;
  casilla->fecha = fecha;
  casilla->tabla = lugares_crear(LUGARES_INI, hash, paso);
  lugares_insertar(casilla->tabla, lugar, notifs);
  //printf("%d-%d-%d\n", casilla->fecha->tm_year+1900, casilla->fecha->tm_mon+1, casilla->fecha->tm_mday);
}

/* Recibe una tabla de conjuntos y una clave
busca el conjunto asociado en la tabla y retorna un puntero al mismo
o en caso de no hallarlo retorna un puntero nulo */
Lugares* fechas_buscar(Fechas* tabla, struct tm* fecha) {
  /* se calcula la posición de la clave dada de acuerdo a la función hash */
  int idx = abs(dias(fecha, tabla->ref));

  if (idx >= tabla->capacidad)
    return NULL;

  if (igual_fecha(tabla->fechas[idx].fecha, fecha))
    return tabla->fechas[idx].tabla;

  if (tabla->fechas[idx].sig)
    return tabla->fechas[idx].sig->tabla;

  return NULL;
}

/* Destruye la tabla y todos sus conjuntos */
void fechas_destruir(Fechas* tabla) {
  /* se recorre la tabla en orden hasta eliminar
  la cantidad de conjuntos que tenia */
  CasillaFecha* aux = NULL;
  for (int i = 0, j = 0; j < tabla->numElems; i++) {
    aux = &tabla->fechas[i];
    if (aux->tabla) {
      j++;
      if (aux->sig) {
        j++;
        lugares_destruir(aux->sig->tabla);

        free(aux->sig->fecha);
        free(aux->sig);
      }
      free(aux->fecha);
      lugares_destruir(aux->tabla);
    }
  }
  free(tabla->ref);
  free(tabla->fechas);
  free(tabla);
}

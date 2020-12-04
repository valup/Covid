#include "hashf.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "straux.h"

/* Funcion de hasheo de forma rolling hash polinomial
explicada en el informe
usada para la creacion de tablas de lugar */
unsigned hash(wchar_t* str) {
  unsigned p = 53, pow = 1, len = wcslen(str), total = 0;
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
que retorna el valor ASCII de la primera letra de la clave
usada para la creacion de tablas de lugar */
unsigned paso(wchar_t* str) {
  return (unsigned) str[0];
}

/* Recibe dos fechas y copia la segunda sobre la primera */
void actualizar_fecha(struct tm* dest, struct tm* fuente) {
  dest->tm_year = fuente->tm_year;
  dest->tm_mon = fuente->tm_mon;
  dest->tm_mday = fuente->tm_mday;
}

/* Recibe una fecha y un entero
y le agrega/quita dias segun el entero */
void agregar_dias(struct tm* fecha, int dias) {
    /* Se cacula convirtiendo la fecha en segundos totales
    y sumando/restando la cantidad de segundos en tantos dias */
    time_t segundos = mktime(fecha) + (dias * 24 * 60 * 60);
    /* luego se convierte el resultado en fecha y se la actualiza */
    *fecha = *localtime(&segundos);
}

/* Recibe dos fechas y las compara */
int igual_fecha(struct tm* f1, struct tm* f2) {
  return (f1->tm_year == f2->tm_year
    && f1->tm_mon == f2->tm_mon
    && f1->tm_mday == f2->tm_mday);
}

/* Recibe una capacidad y crea una tabla de fechas */
Fechas* fechas_crear(int capacidad) {
  Fechas* tabla = malloc(sizeof(Fechas));
  tabla->capacidad = capacidad;
  tabla->numElems = 0;
  tabla->fechas = malloc(sizeof(CasillaFecha) * capacidad);

  /* se inicializan las casillas con datos nulos */
  for (int idx = 0; idx < capacidad; ++idx) {
    tabla->fechas[idx].fecha = NULL;
    tabla->fechas[idx].tabla = NULL;
    tabla->fechas[idx].sig = NULL;
  }

  return tabla;
}

/* Recibe una tabla de fechas, una fecha, un lugar y notificaciones de la fecha
e inserta las ultimas dos en una tabla asociada a la fecha dada */
void fechas_insertar(Fechas* tabla, wchar_t* lugar, struct tm* fecha, int* notifs) {
  /* Si es la primera fecha que se ingresa se ubica al principio
  para luego usarla de referencia para el hasheo */
  if (!tabla->numElems) {
    tabla->numElems++;
    tabla->fechas[0].fecha = fecha;
    tabla->fechas[0].tabla = lugares_crear(LUGARES_INI, hash, paso);
    lugares_insertar(tabla->fechas[0].tabla, lugar, notifs);
    return;
  }
  /* La posicion se calcula como el modulo de la distancia
  en dias respecto a la fecha guardada en la primera posicion */
  int idx = abs(dias(fecha, tabla->fechas[0].fecha));
  /* Si la posicion buscada supera el limite de la tabla, se la realoca
  (no es necesario rehashear porque la distancia respecto
  a la fecha de referencia se mantiene) */
  if (idx >= tabla->capacidad) {
    tabla->capacidad *= 2;
    tabla->fechas = realloc(tabla->fechas, tabla->capacidad);
  }
  /* Se guarda un puntero a la casilla por simplicidad */
  CasillaFecha* casilla = &tabla->fechas[idx];
  /* Si hay fecha guardada en la casilla puede ser la misma
  o la de distancia simetrica respecto a la diferencia
  (si la fecha a insertar es X dias despues, la guardada
  puede ser X dias antes) */
  if (casilla->fecha) {
    if (igual_fecha(casilla->fecha, fecha)) {
      /* Si es la misma fecha se libera la guardada y se la reemplaza
      para seguir utilizando la fecha a insertar luego de la funcion */
      free(casilla->fecha);
      casilla->fecha = fecha;
      /* Se puede asumir que si habia fecha en la casilla tambien
      se creo una tabla para lugares, ya que aun si se eliminan
      todos los lugares de la fecha uno por uno, no se elimina la tabla */
      lugares_insertar(casilla->tabla, lugar, notifs);
      return;
    }
    /* Si hay casilla siguiente en la lista significa que la otra fecha
    posible se inserto, y solo puede ser la misma fecha a insertar */
    if (casilla->sig) {
      /* El puntero pasa a apuntar a la otra casilla y el resto es igual */
      casilla = casilla->sig;
      free(casilla->fecha);
      casilla->fecha = fecha;
      lugares_insertar(casilla->tabla, lugar, notifs);
      return;
    }
    /* Sino hay que crear la nueva casilla para insertar la fecha */
    casilla->sig = malloc(sizeof(CasillaFecha));
    casilla = casilla->sig;
    casilla->sig = NULL;
  }
  /* Si la fecha no estaba hay que contar una fecha nueva
  y crear una tabla de lugares para los registros */
  tabla->numElems++;
  casilla->fecha = fecha;
  casilla->tabla = lugares_crear(LUGARES_INI, hash, paso);
  lugares_insertar(casilla->tabla, lugar, notifs);
}

/* Recibe una tabla de fechas y una fecha, la busca en la tabla
y retorna un puntero a la tabla de lugares que guarda,
o en caso de no hallar retorna un puntero nulo */
Lugares* fechas_buscar(Fechas* tabla, struct tm* fecha) {
  /* Si no hay elementos en la tabla se ahorra la busqueda */
  if (!tabla->numElems)
    return NULL;
  /* Se calcula el modulo de distancia en dias respecto a la primera fecha */
  int idx = abs(dias(fecha, tabla->fechas[0].fecha));
  /* Si supera el limite de la tabla no hay registros para la fecha */
  if (idx >= tabla->capacidad)
    return NULL;
  /* Si se encuentra la fecha se devuelve su tabla */
  if (igual_fecha(tabla->fechas[idx].fecha, fecha))
    return tabla->fechas[idx].tabla;
  /* Si no es la misma fecha pero hay otra casilla en la lista,
  es la casilla de la fecha y se devuelve su tabla */
  if (tabla->fechas[idx].sig)
    return tabla->fechas[idx].sig->tabla;
  /* Si tampoco hay otra casilla la fecha no esta */
  return NULL;
}

/* Destruye la tabla y todas sus fechas y tablas */
void fechas_destruir(Fechas* tabla) {
  /* se recorre la tabla en orden hasta eliminar
  la cantidad de fechas que se habian contado */
  CasillaFecha* aux = NULL;
  for (int i = 0, j = 0; j < tabla->numElems; i++) {
    /* Se guarda un puntero temporal a la casilla por simplicidad */
    aux = &tabla->fechas[i];
    /* Si hay tabla en la casilla se cuenta y se elimina */
    if (aux->tabla) {
      j++;
      /* Si hay otra casilla en la lista se cuenta y se elimina primero */
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
  free(tabla->fechas);
  free(tabla);
}

#include "hashf.h"

/* Funcion de hasheo de forma rolling hash polinomial
explicada en el informe
usada para la creacion de tablas de lugar */
size_t hash(wchar_t* str) {

  const size_t p = 53;
  size_t pow = 1, len = wcslen(str), total = 0;

  for (size_t i = 0; i < len; i++) {
    total += (size_t) str[i] * pow;
    pow *= p;
  }
  return total;
}

/* Funcion de paso para hashing doble
que retorna el valor ASCII de la primera letra de la clave
usada para la creacion de tablas de lugar */
size_t paso(wchar_t* str) {

  return (size_t) str[0];
}

void actualizar_fecha(struct tm* dest, struct tm* fuente) {

  dest->tm_year = fuente->tm_year;
  dest->tm_mon = fuente->tm_mon;
  dest->tm_mday = fuente->tm_mday;
}

void agregar_dias(struct tm* fecha, int dias) {

  /* Se cacula convirtiendo la fecha en segundos totales
  y sumando/restando la cantidad de segundos en tantos dias */
  time_t segundos = mktime(fecha) + (dias * 24 * 60 * 60);
  /* luego se convierte el resultado en fecha y se la actualiza */
  *fecha = *localtime(&segundos);
}

int igual_fecha(struct tm* f1, struct tm* f2) {

  return (f1->tm_year == f2->tm_year
    && f1->tm_mon == f2->tm_mon
    && f1->tm_mday == f2->tm_mday);
}

Fechas* fechas_crear(int capacidad) {

  Fechas* tabla = malloc(sizeof(Fechas));
  tabla->capacidad = capacidad;
  tabla->numElems = 0;
  tabla->fechas = malloc(sizeof(CasillaFecha) * capacidad);

  tabla->ref = malloc(sizeof(struct tm));
  memset(tabla->ref, 0, sizeof(struct tm));

  for (int idx = 0; idx < capacidad; ++idx) {
    tabla->fechas[idx].fecha = NULL;
    tabla->fechas[idx].tabla = NULL;
    tabla->fechas[idx].sig = NULL;
  }

  return tabla;
}

void fechas_insertar(Fechas* tabla, wchar_t* lugar, struct tm* fecha, int* notifs) {

  /* Si es la primera fecha que se ingresa se ubica al principio
  para luego usarla de referencia para el hasheo */
  if (!tabla->numElems) {
    tabla->numElems++;
    tabla->fechas[0].fecha = fecha;
    tabla->fechas[0].tabla = lugares_crear(LUGARES_INI, hash, paso);
    actualizar_fecha(tabla->ref, fecha);
    lugares_insertar(tabla->fechas[0].tabla, lugar, notifs);
    return;
  }

  /* La posicion se calcula como el modulo de la distancia
  en dias respecto a la fecha guardada en la primera posicion */
  int idx = abs(dias(fecha, tabla->ref));

  /* Si la posicion buscada supera el limite de la tabla, se la realoca
  (no es necesario rehashear porque la distancia respecto
  a la fecha de referencia se mantiene) */
  if (idx >= tabla->capacidad) {
    tabla->capacidad *= 2;
    tabla->fechas = realloc(tabla->fechas, tabla->capacidad);
  }

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

      if (!casilla->tabla)
        casilla->tabla = lugares_crear(LUGARES_INI, hash, paso);
      lugares_insertar(casilla->tabla, lugar, notifs);
      return;
    }

    /* Si hay casilla siguiente en la lista significa que la otra fecha
    posible se inserto, y solo puede ser la misma fecha a insertar */
    if (casilla->sig) {
      casilla = casilla->sig;
      free(casilla->fecha);
      casilla->fecha = fecha;

      if (!casilla->tabla)
        casilla->tabla = lugares_crear(LUGARES_INI, hash, paso);
      lugares_insertar(casilla->tabla, lugar, notifs);
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
}

void fechas_eliminar(Fechas* tabla, struct tm* fecha) {

  if (tabla && tabla->numElems) {

    int idx = abs(dias(fecha, tabla->ref));

    if (idx < tabla->capacidad) {

      CasillaFecha* casilla = &tabla->fechas[idx];
      if (casilla->fecha) {
        if (igual_fecha(casilla->fecha, fecha)) {
          tabla->numElems--;
          free(casilla->fecha);
          lugares_destruir(casilla->tabla);

          /* Si habia otra casilla en la lista reemplaza los datos
          y la elimina */
          if (casilla->sig) {
            casilla->fecha = casilla->sig->fecha;
            casilla->tabla = casilla->sig->tabla;
            free(casilla->sig);
            casilla->sig = NULL;
          }

        /* Si no era la misma fecha pero hay otra casilla
        debe ser la de la fecha asi que la elimina,
        pero no busca si hay otra casilla mas porque no pueden
        haber mas de dos fechas en una posicion */
        } else if (casilla->sig) {
          tabla->numElems--;
          free(casilla->sig->fecha);
          lugares_destruir(casilla->sig->tabla);
          free(casilla->sig);
          casilla->sig = NULL;
        }
      }
    }
  }
}

Lugares* fechas_buscar(Fechas* tabla, struct tm* fecha) {

  if (tabla->numElems) {

    int idx = abs(dias(fecha, tabla->ref));
    if (idx < tabla->capacidad) {

      if (igual_fecha(tabla->fechas[idx].fecha, fecha))
        return tabla->fechas[idx].tabla;
      if (tabla->fechas[idx].sig)
        return tabla->fechas[idx].sig->tabla;
    }
  }

  return NULL;
}

void fechas_vaciar(Fechas* tabla) {

  CasillaFecha* aux = NULL;

  /* Recorre la tabla en orden hasta eliminar
  la cantidad de fechas que se habian contado */
  for (int i = 0, j = 0; j < tabla->numElems; i++) {

    aux = &tabla->fechas[i];
    if (aux->tabla) {
      j++;

      /* Si hay otra casilla en la lista la elimina primero */
      if (aux->sig) {
        j++;
        lugares_destruir(aux->sig->tabla);
        free(aux->sig->fecha);
        free(aux->sig);
      }

      lugares_destruir(aux->tabla);
      free(aux->fecha);
      aux->tabla = NULL;
      aux->fecha = NULL;
    }
  }

  tabla->numElems = 0;
  memset(tabla->ref, 0, sizeof(struct tm));
}

void fechas_destruir(Fechas* tabla) {

  fechas_vaciar(tabla);
  free(tabla->ref);
  free(tabla->fechas);
  free(tabla);
}

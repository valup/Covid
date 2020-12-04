#ifndef __HASHF_H__
#define __HASHF_H__

#include "hashl.h"

#define FECHAS_INI 365

/* CasillaFecha es una casilla de hash
con una fecha (puntero struct tm) como clave
y tabla de lugares (puntero a Lugares) como dato,
que resuelve colisiones con una lista */
typedef struct _CasillaFecha {
  struct tm* fecha;
  Lugares *tabla;
  struct _CasillaFecha *sig;
} CasillaFecha;

/* Fechas es una tabla hash hecha para guardar
estructuras CasillaFecha */
/* Para hash y comparacion se definen luego las funciones
dias y igual_fecha, pero se considero innecesario
incluirlas en la estructura para este programa */
typedef struct {
  CasillaFecha* fechas;
  int capacidad;
  int numElems;
} Fechas;

void actualizar_fecha(struct tm* dest, struct tm* fuente);

void agregar_dias(struct tm* fecha, int dias);

int igual_fecha(struct tm* f1, struct tm* f2);

Fechas* fechas_crear(int capacidad);

void fechas_insertar(Fechas* tabla, wchar_t* lugar, struct tm* fecha, int* notifs);

Lugares* fechas_buscar(Fechas* tabla, struct tm* fecha);

void fechas_destruir(Fechas* tabla);

#endif

#ifndef __HASHF_H__
#define __HASHF_H__

#include "hashl.h"

#define FECHAS_INI 365

typedef struct _CasillaFecha {
  struct tm* fecha;
  Lugares *tabla;
  struct _CasillaFecha *sig;
} CasillaFecha;

typedef struct {
  CasillaFecha* fechas;
  int capacidad;
  int numElems;
  struct tm* ref;
} Fechas;

void actualizar_fecha(struct tm* dest, struct tm* fuente);

void agregar_dias(struct tm* fecha, int dias);

int igual_fecha(struct tm* f1, struct tm* f2);

Fechas* fechas_crear(int capacidad);

void fechas_insertar(Fechas* tabla, char* lugar, struct tm* fecha, int* notifs);

//void fechas_eliminar(Fechas* tabla, char* lugar, char* fecha);

Lugares* fechas_buscar(Fechas* tabla, struct tm* fecha);

void fechas_destruir(Fechas* tabla);

#endif

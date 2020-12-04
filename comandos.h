#ifndef __COMANDOS_H__
#define __COMANDOS_H__

#include "hashf.h"
#include "ltree.h"
#include <time.h>

LTree cargar_dataset(Fechas* tabla, LTree lt, char* archivo, struct tm** lims);

void imprimir_dataset(Fechas* tabla, LTree lt, char* archivo, struct tm** lims);

LTree agregar_registro(Fechas* tabla, LTree lt, char** args, struct tm** lims);

void eliminar_registro(Fechas* tabla, struct tm* fecha, char* lugar);

void buscar_pico(Fechas* tabla, char* lugar, struct tm** lims);

void casos_acumulados(Fechas* tabla, struct tm* fecha, char* lugar);

void tiempo_duplicacion(Fechas* tabla, struct tm* fecha, char* lugar, struct tm* prim);

void graficar(Fechas* tabla, struct tm** fechas, char* lugar, struct tm** lims);

#endif

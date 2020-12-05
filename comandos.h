#ifndef __COMANDOS_H__
#define __COMANDOS_H__

#include "hashf.h"
#include "ltree.h"
#include <time.h>
#include <wchar.h>

LTree cargar_dataset(Fechas* tabla, LTree lt, wchar_t* archivo, struct tm** lims);

void imprimir_dataset(Fechas* tabla, LTree lt, wchar_t* archivo, struct tm** lims);

LTree agregar_registro(Fechas* tabla, LTree lt, wchar_t** args, struct tm** lims);

void eliminar_registro(Fechas* tabla, struct tm* fecha, wchar_t* lugar, struct tm** lims);

void buscar_pico(Fechas* tabla, wchar_t* lugar, struct tm** lims);

void casos_acumulados(Fechas* tabla, struct tm* fecha, wchar_t* lugar);

void tiempo_duplicacion(Fechas* tabla, struct tm* fecha, wchar_t* lugar, struct tm* prim);

void graficar(Fechas* tabla, struct tm** fechas, wchar_t* lugar, struct tm** lims);

#endif

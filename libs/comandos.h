#ifndef __COMANDOS_H__
#define __COMANDOS_H__

#define _GNU_SOURCE /* Para popen en graficar */
#include "hashf.h"
#include "ltree.h"

/* Recibe una tabla de fechas, un arbol de lugares, un nombre de archivo
y un puntero a estructuras de tiempo (dos, una para la primera fecha y
otra para la ultima) e intenta cargar toda la infrmacion del archivo
(asumiendo el formato
YYYY-MM-DDT00:00-03:00-Depto,LOCALIDAD,confirmados,descartados,estudio,total)
a la tabla y el arbol, guardando la primera y ultima fecha en la estructura
lims (asumiendo que el archivo esta ordenado por fecha) y retornando el arbol */
LTree cargar_dataset(Fechas* tabla, LTree lt, wchar_t* archivo, struct tm** lims);

/* Recibe una tabla de fechas, un arbol de lugares, un nombre de archivo,
y una estructura de fechas limite, y se imprimen todos los registros
entre las fechas limite en el archivo, con formato
YYYY-MM-DDT00:00:00-03:00,lugar,confirmados,descartados,estudio,total
ordenados primero por fecha y luego por lugar */
void imprimir_dataset(Fechas* tabla, LTree lt, wchar_t* archivo, struct tm** lims);

/* Recibe una tabla de fechas, un arbol de lugares, un puntero a strings
de argumento (una fecha, un lugar y notificaciones) y un puntero a fechas limite
y agrega el registro a la tabla, y de ser necesario agrega el lugar al arbol
y actualiza los limites */
LTree agregar_registro(Fechas* tabla, LTree lt, wchar_t** args, struct tm** lims);

/* Recibe una tabla de fechas una estructura de fecha y un string de lugar,
si hay registros de ese lugar en esa fecha los elimina,
y si no quedan mas registros de la fecha la elimina de la tabla */
void eliminar_registro(Fechas* tabla, struct tm* fecha, wchar_t* lugar, struct tm** lims);

/* Recibe una tabla de fechas, un string de lugar y una estructura
de fechas limite, y busca todos los registros del lugar
entre las fechas limite para hallar el pico */
void buscar_pico(Fechas* tabla, wchar_t* lugar, struct tm** lims);

/* Recibe una tabla de fechas, una estructura de fecha y un string de lugar,
busca registro del lugar en la fecha y si lo halla imprime los casos
confirmados acumulados */
void casos_acumulados(Fechas* tabla, struct tm* fecha, wchar_t* lugar);

/* Recibe una tabla de fechas, una estructura de fecha, un string de lugar
y una estructura de fecha con el limite inferior de fechas, e intenta
buscar la cantidad de dias que tardaron en duplicarse los casos confirmados
acumulados hasta llegar a la fecha ingresada */
void tiempo_duplicacion(Fechas* tabla, struct tm* fecha, wchar_t* lugar, struct tm* prim);

/* Recibe una tabla de fechas, un puntero a estructuras de fecha,
un string de lugar y un puntero a estructuras de fechas limite
e intenta graficar la evolucion de casos confirmados diarios y acumulados
del lugar entre las fechas */
void graficar(Fechas* tabla, struct tm** fechas, wchar_t* lugar, struct tm** lims);

#endif

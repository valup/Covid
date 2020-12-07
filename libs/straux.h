#ifndef __STRAUX_H__
#define __STRAUX_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <wchar.h>

/* Recibe un wide string nulo para buffer y un stream de donde leer,
y lee una linea completa del stream hasta nueva linea
sin limite de caracteres */
wchar_t* wgetline(wchar_t* buf, FILE* stream);

/* Recibe tres strings y devuelve uno nuevo con los dos primeros
separados por el tercero */
wchar_t* unir(wchar_t* str1, wchar_t* str2, wchar_t* sep);

/* Recibe un string e intenta convertirlo en int
inicialmente convirtiendolo en long y chequeando que el resultado sea valido
en cuyo caso se guarda en un puntero y se retorna
sino se retorna NULL */
int* string_to_int(wchar_t* str);

/* Calcula la diferencia en dias entre dos fechas,
positiva si la primera fecha es posterior a la segunda
y negativa en caso contrario */
int dias(struct tm* f1, struct tm* f2);

/* Toma un string que asume como fecha en formato AAAA-MM-DD
e intenta crear una estructura de tiempo struct tm* con esa informacion */
struct tm* string_fecha(wchar_t* fecha);

/* Toma un comando en buffer con argumentos separados por espacio,
pone una separador | en el primer espacio luego de la fecha
(asumiendo que esta al principio) y retorna un puntero
al primer caracter del comando luego de ese separador */
wchar_t* marcar_fecha(wchar_t* buf);

/* Toma un comando en buffer con argumentos separados por espacio
(excepto la fecha que se separa primero con |),
e intenta poner un separador | en el espacio antes de cada notificacion
(asumiendo que son tres al final), retorna 1 si pudo o 0 sino */
int marcar_notifs(wchar_t* buf);

/* Toma un comando en buffer con departamento y localidad separados con espacio
y un caracter con el cual detener la lectura,
e intenta separar las partes por coma (asumiendo que estan al principio, que
ambas pueden tener espacios adentro y que se diferencian porque el departamento
tiene minusculas y la localidad no), retorna 1 si pudo o 0 sino */
int marcar_lugar(wchar_t* buf, wchar_t hasta);

#endif

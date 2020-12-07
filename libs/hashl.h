#ifndef __HASHL_H__
#define __HASHL_H__

#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LIM 0.7
#define LUGARES_INI 997

typedef size_t (*FuncionHash)(wchar_t* clave);

typedef size_t (*FuncionPaso)(wchar_t* clave);

/* CasillaLugar es una casilla de hash
con string (lugar) como clave y notificaciones de casos
(puntero a int para guardar 3 int) como dato */
typedef struct _CasillaLugar {
  wchar_t* lugar;
  int* notifs;
} CasillaLugar;

/* Lugares es una tabla hash hecha para guardar
estructuras CasillaLugar */
/* Para comparacion se usa !strcmp
y se considero innecesario incluirla como funcion
en la estructura para este programa */
typedef struct {
  CasillaLugar* lugares;
  size_t numElems;
  size_t capacidad;
  FuncionHash hash;
  FuncionPaso paso;
} Lugares;

/* Recibe una capacidad y funciones para hash y paso
y crea una tabla de lugares con esa informacion */
Lugares* lugares_crear(size_t capacidad, FuncionHash hash, FuncionPaso paso);

/* Recibe una tabla de lugares, un lugar y un puntero a notificaciones
e inserta el puntero en la tabla, asociado al lugar dado */
void lugares_insertar(Lugares* tabla, wchar_t* lugar, int* notifs);

/* Recibe una tabla de lugares y un lugar
busca las notificaciones asociadas en la tabla y retorna un puntero a ellas
o en caso de no hallarlas retorna un puntero nulo */
int* lugares_buscar(Lugares* tabla, wchar_t* lugar);

/* Recibe una tabla de lugares y un lugar,
busca la entrada asociada en la tabla y si halla la elimina */
void lugares_eliminar(Lugares* tabla, wchar_t* lugar);

/* Recibe una tabla de lugares y duplica su capacidad,
luego rehashea todos los elementos para insertarlos en la nueva tabla */
void lugares_redimensionar(Lugares* tabla);

/* Destruye la tabla y todas las notificaciones guardadas */
void lugares_destruir(Lugares* tabla);

#endif

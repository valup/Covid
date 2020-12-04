#ifndef __HASHL_H__
#define __HASHL_H__

#include <wchar.h>

#define LIM 0.7
#define LUGARES_INI 997

typedef unsigned (*FuncionHash)(wchar_t* clave);

typedef unsigned (*FuncionPaso)(wchar_t* clave);

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
  unsigned numElems;
  unsigned capacidad;
  FuncionHash hash;
  FuncionPaso paso;
} Lugares;

Lugares* lugares_crear(unsigned capacidad, FuncionHash hash, FuncionPaso paso);

void lugares_insertar(Lugares* tabla, wchar_t* lugar, int* notifs);

int* lugares_buscar(Lugares* tabla, wchar_t* lugar);

void lugares_eliminar(Lugares* tabla, wchar_t* lugar);

void lugares_redimensionar(Lugares* tabla);

void lugares_destruir(Lugares* tabla);

#endif

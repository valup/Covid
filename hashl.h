#ifndef __HASHL_H__
#define __HASHL_H__

#define LIM 0.7
#define LUGARES_INI 997

typedef unsigned (*FuncionHash)(char* clave);

typedef unsigned (*FuncionPaso)(char* clave);

/* CasillaLugar es una casilla de hash
con string (lugar) como clave y notificaciones de casos
(puntero a int para guardar 3 int) como dato */
typedef struct _CasillaLugar {
  char* lugar;
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

void lugares_insertar(Lugares* tabla, char* lugar, int* notifs);

int* lugares_buscar(Lugares* tabla, char* lugar);

void lugares_eliminar(Lugares* tabla, char* lugar);

void lugares_redimensionar(Lugares* tabla);

void lugares_destruir(Lugares* tabla);

#endif

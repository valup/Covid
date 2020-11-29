#ifndef __HASHL_H__
#define __HASHL_H__

#define LIM 0.7
#define LUGARES_INI 997

typedef unsigned (*FuncionHash)(char* clave);

typedef unsigned (*FuncionPaso)(char* clave);

typedef struct _CasillaHash {
  char* lugar;
  int* notifs;
} CasillaHash;

typedef struct {
  CasillaHash* lugares;
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

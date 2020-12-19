#ifndef __HASHF_H__
#define __HASHF_H__

#include "hashl.h"
#include "straux.h"

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
  /* Estructura para guardar la fecha de referencia
  para insertar otras fechas */
  struct tm* ref;
  int capacidad;
  int numElems;
} Fechas;

/* Recibe dos fechas y copia la segunda sobre la primera */
void actualizar_fecha(struct tm* dest, struct tm* fuente);

/* Recibe una fecha y un entero
y le agrega/quita dias segun el entero */
void agregar_dias(struct tm* fecha, int dias);

/* Recibe dos fechas y las compara */
int igual_fecha(struct tm* f1, struct tm* f2);

/* Recibe una capacidad y crea una tabla de fechas */
Fechas* fechas_crear(int capacidad);

/* Recibe una tabla de fechas, una fecha, un lugar y notificaciones de la fecha
e inserta las ultimas dos en una tabla asociada a la fecha dada */
void fechas_insertar(Fechas* tabla, wchar_t* lugar,
                    struct tm* fecha, int* notifs);

/* Recibe una tabla de fechas y una fecha
y elimina la casilla de la fecha si existe */
void fechas_eliminar(Fechas* tabla, struct tm* fecha);

/* Recibe una tabla de fechas y una fecha, la busca en la tabla
y retorna un puntero a la tabla de lugares que guarda,
o en caso de no hallar retorna un puntero nulo */
Lugares* fechas_buscar(Fechas* tabla, struct tm* fecha);

/* Recibe una tabla y libera todos los datos que guarda
para devolverla a su estado original.
Se usa para destruirla o para reiniciarla luego de un error de carga */
void fechas_vaciar(Fechas* tabla);

/* Destruye la tabla y todas sus fechas y tablas */
void fechas_destruir(Fechas* tabla);

#endif

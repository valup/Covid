#define  _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "comandos.h"
#include "straux.h"

LTree cargar_dataset(Fechas* tabla, LTree lt, wchar_t* archivo, struct tm** lims) {
  /* Convierte el nombre de archivo a char* para poder abrir el archivo */
  char* file = malloc(sizeof(char) * (wcslen(archivo) + 2));
  swscanf(archivo, L"%s", file);

  FILE* fp; /* Puntero para el archivo */
  struct tm* f = NULL; /* Puntero para crear cada estructura de fecha */

  if ((fp = fopen(file, "r"))) {

    free(file);

    wchar_t* buf = NULL;
    wchar_t* pt; /* Puntero para wcstok */

    /* Se lee la primera linea del archivo, que deberia indicar
    los nombres de las columnas */
    buf = wgetline(buf, fp);

    if (feof(fp)) {
      printf("\nERROR: Archivo vacio.\n");
      printf("Ingrese help para mas informacion\n\n");
      free(buf);
      fclose(fp);
      return lt;
    }

    free(buf);

    int lim = 0; /* Flag para indicar si se guardo el primer limite */

    buf = wgetline(buf, fp);

    while (!feof(fp)) {

      /* Hasta la T es fecha */
      wchar_t* fecha = wcstok(buf, L"T", &pt);
      f = string_fecha(fecha);
      if (!f) {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        fclose(fp);
        return lt;
      }

      wchar_t* depto = wcstok(NULL, L",", &pt);
      /* Se repite para descartar la parte de 00:00:00-03:00 */
      depto = wcstok(NULL, L",", &pt);
      if (!depto) {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        free(f);
        fclose(fp);
        return lt;
      }

      wchar_t* local = wcstok(NULL, L",", &pt);
      if (!local) {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        free(f);
        fclose(fp);
        return lt;
      }

      wchar_t* cuenta = wcstok(NULL, L"\n", &pt);
      int* notifs = malloc(sizeof(int) * 3);
      int read = swscanf(cuenta, L"%d,%d,%d", notifs, &notifs[1], &notifs[2]);
      if (read < 3) {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(notifs);
        free(buf);
        free(f);
        fclose(fp);
        return lt;
      }

      /* Si no se guardo el primer limite se actualiza con f */
      if (!lim) {
        actualizar_fecha(lims[0], f);
        lim = 1;
      }

      /* Se unen el departamento y localidad en un string con coma */
      wchar_t* lugar = unir(depto, local, L",");

      /* Se intenta insertar el lugar primero en el arbol,
      si ya estaba se libera el string y se lo reemplaza por el que estaba */
      lt = ltree_insertar(lt, &lugar);
      fechas_insertar(tabla, lugar, f, notifs);

      free(buf);
      buf = wgetline(buf, fp);
    }

    /* Si se termino de leer el archivo y quedo fecha leida
    (no sucede si el archivo no tenia registros) se guarda el otro limite */
    if (f)
      actualizar_fecha(lims[1], f);

    free(buf);
    fclose(fp);

  } else {
    printf("\nERROR: No se puede abrir el archivo.\n\n");
  }

  return lt;
}

/* Funcion auxiliar que recibe un arbol de lugares, una tabla de lugares,
un puntero a archivo y una fecha (la asociada a la tabla)
e imprime todos los registros de la tabla segun el orden del arbol */
void imprimir_dataset_aux(LTree lt, Lugares* t, FILE* fp, struct tm* f) {

  if (!lt)
    return;

  imprimir_dataset_aux(lt->left, t, fp, f);

  int* not = lugares_buscar(t, lt->lugar);

  if (not) {
    fprintf(fp, "%d-%d-%dT00:00:00-03:00,%ls,%d,%d,%d,%d\n",
      f->tm_year+1900, f->tm_mon+1, f->tm_mday,
      lt->lugar, not[0], not[1], not[2], not[0]+not[1]+not[2]);
  }

  imprimir_dataset_aux(lt->right, t, fp, f);
}

void imprimir_dataset(Fechas* tabla, LTree lt, wchar_t* arch, struct tm** lims) {

  char* file = malloc(sizeof(char) * (wcslen(arch) + 2));
  swscanf(arch, L"%s", file);
  FILE* fp = fopen(file, "w");
  free(file);
  fprintf(fp,
  "Fecha,Departamento,Localidad,Confirmados,Descartados,En estudio,Notificaciones\n"
  );

  if (!tabla->numElems) {
    fclose(fp);
    return;
  }

  /* Se inicializa una estructura de fecha para buscar en la tabla */
  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, lims[0]);

  int orden = 0; /* Indicador del orden de las fechas */

  /* Si la segunda fecha es posterior a la primera es orden ascendente */
  if (dias(lims[1], lims[0]) > 0)
    orden = 1;
  else
    orden = -1;

  int fin = 0; /* Flag para indicar si se llego al ultimo limite */
  CasillaFecha* casilla; /* Puntero para simplificar luego el codigo */

  int idx = abs(dias(lims[0], tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {

    if (igual_fecha(f, lims[1]))
      fin = 1;

    casilla = &tabla->fechas[idx];
    if (casilla->tabla) {

      /* Si no es la misma fecha pero hay otra casilla en la lista,
      esa tiene que ser la de la fecha */
      if (!igual_fecha(f, casilla->fecha) && casilla->sig)
        casilla = casilla->sig;

      imprimir_dataset_aux(lt, casilla->tabla, fp, casilla->fecha);
    }

    /* Se busca el siguiente dia a imprimir segun el orden */
    agregar_dias(f, orden);
  }

  fclose(fp);
  free(f);
}

LTree agregar_registro(Fechas* tabla, LTree lt, wchar_t** args, struct tm** lims) {

  struct tm* f = string_fecha(args[0]);
  if (!f) {
    printf("\nERROR: Fecha invalida.\n\n");
    printf("Ingrese help para mas informacion.\n\n");
    return lt;
  }

  int* notifs = malloc(sizeof(int) * 3);
  int read = swscanf(args[2], L"%d|%d|%d", notifs, &notifs[1], &notifs[2]);
  if (read < 3) {
    printf("\nERROR: Numero invalido.\n");
    printf("Ingrese help para mas informacion.\n\n");
    free(notifs);
    free(f);
    return lt;
  }

  /* Compara la fecha con los limites segun el orden */
  if (dias(lims[1], lims[0]) >= 0) {
    /* Si es ascendente y la fecha es posterior a la ultima,
    actualiza la ultima */
    if (dias(f, lims[1]) > 0) {
      actualizar_fecha(lims[1], f);
    /* Si es anterior a la primera, actualiza la primera */
    } else if (dias(f, lims[0]) < 0) {
      actualizar_fecha(lims[0], f);
    }

  } else {
    /* Si es descendente y la fecha es anterior a la ultima,
    actualiza la ultima */
    if (dias(f, lims[1]) < 0) {
      actualizar_fecha(lims[1], f);
    /* Si es posterior a la primera, actualiza la primera */
    } else if (dias(f, lims[0]) > 0) {
      actualizar_fecha(lims[0], f);
    }
  }

  /* Aloca memoria para poder guardar el lugar */
  wchar_t* lugar = malloc(sizeof(wchar_t) * (wcslen(args[1]) + 1));
  wcscpy(lugar, args[1]);

  /* Inserta en la tabla pero primero
  intenta insertar el lugar en el arbol, y si ya estaba
  se libera y se reemplaza por el lugar del arbol */
  lt = ltree_insertar(lt, &lugar);
  fechas_insertar(tabla, lugar, f, notifs);

  return lt;
}

void eliminar_registro(Fechas* tabla, struct tm* fecha, wchar_t* lugar, struct tm** lims) {

  Lugares* lugares = fechas_buscar(tabla, fecha);
  if (lugares) {

    lugares_eliminar(lugares, lugar);

    /* Si no quedan mas lugares en la fecha la elimina */
    if (!lugares->numElems) {

      fechas_eliminar(tabla, fecha);

      /* Si era la primera fecha limite cambia el limite,
      a menos que los limites sean iguales */
      if (igual_fecha(fecha, lims[0])) {
        /* Calcula orden de fechas */
        int d = dias(lims[1], lims[0]);
        /* Si es ascendente le agrega 1 dia */
        if (d > 0) {
          agregar_dias(lims[0], 1);
        /* Si es descendente le quita 1 dia */
        } else if (d < 0) {
          agregar_dias(lims[0], -1);
        }

      } else if (igual_fecha(fecha, lims[1])) {
        int d = dias(lims[1], lims[0]);
        if (d > 0) {
          agregar_dias(lims[1], -1);
        } else if (d < 0) {
          agregar_dias(lims[1], 1);
        }
      }
    }
  }
}

void buscar_pico(Fechas* tabla, wchar_t* lugar, struct tm** lims) {

  if (!tabla->numElems) {
    printf("\nERROR: No hay registros cargados.\n\n");
    return;
  }

  /* Calcula el orden de las fechas para buscar desde la anterior.
  1 = ascendente, 0 = descendente, entonces lims[1 - orden]
  sera la fecha anterior y lims[orden] la posterior */
  int orden = (dias(lims[1], lims[0]) > 0);

  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, lims[1 - orden]);

  int acum = 0; /* Variable para guardar cantidad acumulada de confirmados */
  int pico = -1; /* Variable para guardar pico de confirmados */

  /* Inicializa una estructura de fecha para la fecha del pico */
  struct tm* fpico = malloc(sizeof(struct tm));
  memset(fpico, 0, sizeof(struct tm));

  int fin = 0; /* Flag para indicar si se llego a la ultima fecha */
  CasillaFecha* casilla; /* Puntero para simplificar luego el codigo */

  int idx = abs(dias(lims[1 - orden], tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {

    if (igual_fecha(f, lims[orden]))
      fin = 1;

    casilla = &tabla->fechas[idx];
    if (casilla->tabla) {

      if (!igual_fecha(f, casilla->fecha) && casilla->sig)
        casilla = casilla->sig;

      int* notifs = lugares_buscar(casilla->tabla, lugar);
      if (notifs) {
        /* Si hay calcula los confirmados diarios
        como la diferencia entre los acumulados nuevos y los anteriores */
        int diarios = notifs[0] - acum;

        if (diarios < 0) {
          printf("\nERROR: Datos incorrectos en %d-%d-%d.\n\n",
            f->tm_year+1900, f->tm_mon+1, f->tm_mday);
          free(f);
          free(fpico);
          return;

        } else {
          acum = notifs[0];

          if (diarios > pico) {
            pico = diarios;
            actualizar_fecha(fpico, casilla->fecha);
          }
        }
      }
    }

    agregar_dias(f, 1);
  }

  /* Si el pico queda en -1 significa que no se encontraron registros */
  if (pico > -1) {
    printf("\nPico de %ls: %d en %d-%d-%d.\n\n", lugar, pico,
      fpico->tm_year+1900, fpico->tm_mon+1, fpico->tm_mday);
  } else {
    printf("\nERROR: No hay registros en %ls.\n\n", lugar);
  }

  free(f);
  free(fpico);
}

void casos_acumulados(Fechas* tabla, struct tm* fecha, wchar_t* lugar) {

  Lugares* l = fechas_buscar(tabla, fecha);
  if (l) {

    int* notifs = lugares_buscar(l, lugar);
    if (notifs) {
      printf("\nCasos acumulados en %ls hasta %d-%d-%d: %d.\n\n",
      lugar, fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday, *notifs);
    } else {
      printf("\nERROR: No hay registro de %d-%d-%d en %ls.\n\n",
      fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday, lugar);
    }

  } else {
    printf("\nERROR: No hay registros de %d-%d-%d.\n\n",
    fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday);
  }
}

void tiempo_duplicacion(Fechas* tabla, struct tm* fecha, wchar_t* lugar, struct tm* prim) {

  if (!tabla->numElems) {
    printf("\nERROR: No hay registros cargados.\n\n");
    return;
  }

  Lugares* l = fechas_buscar(tabla, fecha);
  if (l) {

    int* og = lugares_buscar(l, lugar); /* Valor original de casos acumulados */
    if (og) {

      printf("\nCasos acumulados en %ls hasta %d-%d-%d: %d\n",
      lugar, fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday, *og);

      int obj = *og/2; /* Maximo de casos acumulados que quiere hallar */

      struct tm* f = malloc(sizeof(struct tm));
      memset(f, 0, sizeof(struct tm));
      actualizar_fecha(f, fecha);

      int fin = 0; /* Flag que indica si se llego a la primera fecha */
      CasillaFecha* casilla;

      int idx = abs(dias(fecha, tabla->ref));
      for(; !fin; idx = abs(dias(f, tabla->ref))) {

        if (igual_fecha(f, prim))
          fin = 1;

        casilla = &tabla->fechas[idx];
        if (casilla->tabla) {

          if (!igual_fecha(f, casilla->fecha) && casilla->sig)
            casilla = casilla->sig;

          int* notifs = lugares_buscar(casilla->tabla, lugar);

          /* Si hay registro y la cantidad acumulada de confirmados
          es menor o igual al valor buscado, imprime la diferencia en dias */
          if (notifs && *notifs <= obj) {

            printf("Casos acumulados en %ls hasta %d-%d-%d: %d\n",
            lugar, casilla->fecha->tm_year+1900, casilla->fecha->tm_mon+1,
            casilla->fecha->tm_mday, *notifs);

            printf("\nTiempo de duplicacion: %d dias\n\n",
            dias(fecha, casilla->fecha));

            free(f);
            return;
          }
        }

        agregar_dias(f, -1);
      }

      printf(
      "\nERROR: No hay registros en %ls con %d o menos casos acumulados.\n\n",
      lugar, obj);

      free(f);
    } else
      printf("\nERROR: No hay registro de %d-%d-%d en %ls.\n\n",
      fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday, lugar);

  } else
    printf("\nERROR: No hay registros de %d-%d-%d.\n\n",
    fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday);
}

/* Funcion auxiliar que recibe un puntero a gnuplot y un puntero a
estructuras de fecha e imprime comandos a gnuplot para crear dos graficos
de datos entre las fechas */
void comandos_gnuplot(FILE* gnuplotPipe, struct tm** fechas) {
  /* Tipo de datos en eje x */
  fprintf(gnuplotPipe, "set xdata time\n");
  /* Formato de tiempo a leer */
  fprintf(gnuplotPipe, "%s\n", "set timefmt \"%Y-%m-%d\"");
  /* Formato de tiempo a escribir en x */
  fprintf(gnuplotPipe, "%s\n", "set format x \"%d\\n%m\\n%Y\"");
  /* Rango de fechas para el eje */
  fprintf(gnuplotPipe, "set xrange [\"%d-%d-%d\":\"%d-%d-%d\"]\n",
    fechas[0]->tm_year+1900, fechas[0]->tm_mon+1, fechas[0]->tm_mday,
    fechas[1]->tm_year+1900, fechas[1]->tm_mon+1, fechas[1]->tm_mday);
  /* Presentacion de dos graficos uno sobre el otro */
  fprintf(gnuplotPipe, "set multiplot layout 2,1\n");
  /* Titulo del primer grafico */
  fprintf(gnuplotPipe, "set title \"Diarios\"\n");
  /* Grafico con lineas sin leyenda */
  fprintf(gnuplotPipe, "plot \"diarios.temp\" u 1:2 w lines notitle\n");
  /* Titulo del segundo grafico */
  fprintf(gnuplotPipe, "set title \"Acumulados\"\n");
  /* Grafico con lineas sin leyenda */
  fprintf(gnuplotPipe, "plot \"acum.temp\" u 1:2 w lines notitle\n");
  fflush(gnuplotPipe); /* Ejecuta todo */
}

void graficar(Fechas* tabla, struct tm** fechas, wchar_t* lugar, struct tm** lims) {

  if (!tabla->numElems) {
    printf("\nERROR: No hay registros cargados.\n\n");
    return;
  }

  /* Calcula que pueda haber registros entre las fechas ingresadas
  buscando interseccion con las fechas limite */
  int orden = (dias(lims[1], lims[0]) > 0);
  if (dias(fechas[0], lims[orden]) > 0 || dias(lims[1 - orden], fechas[1]) > 0) {
    printf("\nERROR: No hay registros entre %d-%d-%d y %d-%d-%d.\n\n",
    fechas[0]->tm_year+1900, fechas[0]->tm_mon+1, fechas[0]->tm_mday,
    fechas[1]->tm_year+1900, fechas[1]->tm_mon+1, fechas[1]->tm_mday);
    return;
  }

  /* Si hay interseccion pero alguna de las fechas esta fuera de los limites,
  la reemplaza por el limite */
  if (dias(fechas[1], lims[orden]) > 0)
    actualizar_fecha(fechas[1], lims[orden]);
  if (dias(lims[1 - orden], fechas[0]) > 0)
    actualizar_fecha(fechas[0], lims[1 - orden]);

  /* Abre los archivos donde imprimir los datos a graficar */
  FILE* fp[2];
  fp[0] = fopen("diarios.temp", "w");
  fp[1] = fopen("acum.temp", "w");

  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, fechas[0]);

  int acum = 0; /* Variable de casos acumulados hasta la fecha */
  int fin = 0; /* Flag que indica si se llego a la ultima fecha */
  CasillaFecha* casilla;

  int idx = abs(dias(fechas[0], tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {

    if (igual_fecha(f, fechas[1]))
      fin = 1;

    casilla = &tabla->fechas[idx];
    if (casilla->tabla) {

      if (!igual_fecha(f, casilla->fecha) && casilla->sig)
        casilla = casilla->sig;

      int* notifs = lugares_buscar(casilla->tabla, lugar);
      if (notifs && *notifs - acum > 0) {

        /* Los casos diarios son la diferencia entre los acumulados del dia
        y los acumulados de antes */
        fprintf(fp[0], "%d-%d-%d %d\n",
          f->tm_year+1900, f->tm_mon+1, f->tm_mday, *notifs - acum);
        fprintf(fp[1], "%d-%d-%d %d\n",
          f->tm_year+1900, f->tm_mon+1, f->tm_mday, *notifs);

        acum = *notifs;
      }
    }

    agregar_dias(f, 1);
  }
  fclose(fp[0]);
  fclose(fp[1]);

  FILE* gnuplotPipe = popen("gnuplot -persistent", "w");
  comandos_gnuplot(gnuplotPipe, fechas);
  fclose(gnuplotPipe);

  free(f);
}

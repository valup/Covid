#include "comandos.h"

/* Recibe un puntero a estructuras de fechas limite, una fecha
y una cantidad de elementos y actualiza los limites con la fecha */
void actualizar_lims(struct tm** lims, struct tm* f, int elems) {
  
  if (!elems) {
    actualizar_fecha(lims[0], f);
    actualizar_fecha(lims[1], f);

  /* Compara la fecha con los limites segun el orden */
  } else if (dias(lims[1], lims[0]) >= 0) {
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
}

LTree cargar_dataset(Fechas* tabla, LTree lt,
                    wchar_t* archivo, struct tm** lims) {

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
    int error = 0; /* Flag para indicar que hubo problema con la lectura */

    buf = wgetline(buf, fp);

    while (!feof(fp) && buf[0] != L'\n') {

      /* Hasta la T es fecha */
      wchar_t* fecha = wcstok(buf, L"T", &pt);
      f = string_fecha(fecha);
      if (f) {

        wchar_t* depto;
        /* Se lee dos veces para descartar la parte de 00:00:00-03:00 */
        for (int i = 0; i < 2 && !error; i++) {
          depto = wcstok(NULL, L",", &pt);
          if (!depto)
            error = 1;
        }

        if (!error) {

          wchar_t* local = wcstok(NULL, L",", &pt);
          if (local) {

            wchar_t* cuenta = wcstok(NULL, L"\n", &pt);
            int* notifs = malloc(sizeof(int) * 3);

            int read = swscanf(cuenta, L"%d,%d,%d", notifs,
                              &notifs[1], &notifs[2]);
            if (read == 3) {

              /* Si no se guardo el primer limite se guarda f,
              sino se intenta actualizar */
              if (!lim) {
                if (!lims[0]->tm_year) {
                  actualizar_fecha(lims[0], f);
                } else
                  actualizar_lims(lims, f, tabla->numElems);

                lim = 1;
              }

              /* Se unen el departamento y localidad en un string con coma */
              wchar_t* lugar = unir(depto, local, L",");

              /* Se intenta insertar el lugar primero en el arbol,
              si ya estaba se libera el string
              y se lo reemplaza por el que estaba */
              lt = ltree_insertar(lt, &lugar);
              fechas_insertar(tabla, lugar, f, notifs);
            } else {
              free(notifs);
              free(f);
              error = 1;
            }
          } else {
            free(f);
            error = 1;
          }
        } else {
          free(f);
          error = 1;
        }
      }

      if (error) {
        printf("\nERROR: Hubo un problema leyendo los datos.\n\n");
        fechas_vaciar(tabla);
        ltree_destruir(lt);
        return NULL;
      }

      free(buf);
      buf = wgetline(buf, fp);
    }

    /* Si se termino de leer el archivo y quedo fecha leida
    (no sucede si el archivo no tenia registros) se guarda el otro limite */
    if (f) {
      if (lims[1]->tm_year == 0)
        actualizar_fecha(lims[1], f);
      else
        actualizar_lims(lims, f, tabla->numElems);

      lim = 1;
    }

    free(buf);
    fclose(fp);
    printf("\nDataset cargado correctamente desde %ls.\n\n", archivo);

  } else {
    printf("\nERROR: No se puede abrir el archivo.\n\n");
    free(file);
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
    fprintf(fp, "%d-%02d-%02dT00:00:00-03:00,%ls,%d,%d,%d,%d\n",
      f->tm_year+1900, f->tm_mon+1, f->tm_mday,
      lt->lugar, not[0], not[1], not[2], not[0]+not[1]+not[2]);
  }

  imprimir_dataset_aux(lt->right, t, fp, f);
}

void imprimir_dataset(Fechas* tabla, LTree lt,
                      wchar_t* arch, struct tm** lims) {

  char* file = malloc(sizeof(char) * (wcslen(arch) + 2));
  swscanf(arch, L"%s", file);
  FILE* fp = fopen(file, "w");
  free(file);
  fprintf(fp,
  "Fecha,Departamento,Localidad,Confirmados,Descartados,En estudio,Notificaciones\n"
  );

  if (!tabla->numElems) {
    printf("\nDataset impreso correctamente en %ls.\n\n", arch);
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
  for(;!fin; idx = abs(dias(f, tabla->ref))) {

    if (igual_fecha(f, lims[1]))
      fin = 1;

    casilla = &tabla->fechas[idx];
    if (casilla->tabla) {

      if (igual_fecha(f, casilla->fecha)) {
        imprimir_dataset_aux(lt, casilla->tabla, fp, casilla->fecha);

      /* Si no es la misma fecha pero hay otra casilla en la lista,
      esa tiene que ser la de la fecha */
      } else if (casilla->sig) {
        casilla = casilla->sig;
        imprimir_dataset_aux(lt, casilla->tabla, fp, casilla->fecha);
      }
    }

    /* Se busca el siguiente dia a imprimir segun el orden */
    agregar_dias(f, orden);
  }

  printf("\nDataset impreso correctamente en %ls.\n\n", arch);
  fclose(fp);
  free(f);
}

LTree agregar_registro(Fechas* tabla, LTree lt,
                      wchar_t** args, struct tm** lims) {

  struct tm* f = string_fecha(args[0]);
  if (!f)
    return lt;

  int* notifs = malloc(sizeof(int) * 3);
  int read = swscanf(args[2], L"%d|%d|%d", notifs, &notifs[1], &notifs[2]);
  if (read < 3) {
    printf("\nERROR: Numero invalido.\n");
    printf("Ingrese help para mas informacion.\n\n");
    free(notifs);
    free(f);
    return lt;
  }

  actualizar_lims(lims, f, tabla->numElems);

  /* Aloca memoria para poder guardar el lugar */
  wchar_t* lugar = malloc(sizeof(wchar_t) * (wcslen(args[1]) + 1));
  wcscpy(lugar, args[1]);

  /* Inserta en la tabla pero primero
  intenta insertar el lugar en el arbol, y si ya estaba
  se libera y se reemplaza por el lugar del arbol */
  lt = ltree_insertar(lt, &lugar);
  fechas_insertar(tabla, lugar, f, notifs);

  printf("\nRegistro cargado correctamente.\n\n");

  return lt;
}

void eliminar_registro(Fechas* tabla, struct tm* fecha,
                      wchar_t* lugar, struct tm** lims) {

  Lugares* lugares = fechas_buscar(tabla, fecha);
  if (lugares) {

    lugares_eliminar(lugares, lugar);

    printf("\nRegistro eliminado correctamente.\n\n");

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

/* Funcion auxiliar que recibe una tabla de lugares, un string de lugar
y punteros a enteros de pico y cantidad acumulada
para buscar registro del lugar en la tabla y comparar con el pico guardado,
si es nuevo pico retorna 1, si hay disminucion de acumulados retorna -1
y sino 0 */
int buscar_pico_aux(Lugares* tabla, wchar_t* lugar, int* acum, int* pico) {

  int* notifs = lugares_buscar(tabla, lugar);
  if (notifs) {

    /* Si hay calcula los confirmados diarios
    como la diferencia entre los acumulados nuevos y los anteriores */
    int diarios = *notifs - *acum;
    *acum = *notifs;

    if (diarios < 0) {
      return -1;

    } else if (diarios > *pico) {
      *pico = diarios;
      return 1;
    }
  }

  return 0;
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
  int p = 0; /* Retorno de funcion auxiliar */
  CasillaFecha* casilla; /* Puntero para simplificar luego el codigo */

  int idx = abs(dias(lims[1 - orden], tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {

    if (igual_fecha(f, lims[orden]))
      fin = 1;

    casilla = &tabla->fechas[idx];
    if (casilla->tabla) {

      if (igual_fecha(f, casilla->fecha))
        p = buscar_pico_aux(casilla->tabla, lugar, &acum, &pico);

      else if (casilla->sig)
        p = buscar_pico_aux(casilla->sig->tabla, lugar, &acum, &pico);

      if (p == 1) {
        actualizar_fecha(fpico, f);

      } else if (p == -1) {
        printf("\nWARNING: Error de datos en %d-%02d-%02d.\n",
        f->tm_year+1900, f->tm_mon+1, f->tm_mday);
        printf("Cantidad acumulada de confirmados menor a la anterior.\n\n");
      }

      p = 0;
    }

    agregar_dias(f, 1);
  }

  /* Si el pico queda en -1 significa que no se encontraron registros */
  if (pico > -1) {
    printf("\nPico de %ls: %d en %d-%02d-%02d.\n\n", lugar, pico,
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
      printf("\nCasos acumulados en %ls hasta %d-%02d-%02d: %d.\n\n",
      lugar, fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday, *notifs);

    } else {
      printf("\nERROR: No hay registro de %d-%02d-%02d en %ls.\n\n",
      fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday, lugar);
    }

  } else {
    printf("\nERROR: No hay registros de %d-%02d-%02d.\n\n",
    fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday);
  }
}

/* Funcion auxiliar que recibe una tabla de lugares, un string de lugar
y la cantidad maxima de casos que se buscan, busca el numero de casos del lugar
si hay y si no supera el objetivo lo retorna, sino retorna 0 */
int tiempo_duplicacion_aux(Lugares* tabla, wchar_t* lugar, int obj) {
  int* notifs = lugares_buscar(tabla, lugar);

  /* Si hay registro y la cantidad acumulada de confirmados
  es menor o igual al valor buscado, imprime la diferencia en dias */
  if (notifs && *notifs <= obj)
    return *notifs;

  return 0;
}

void tiempo_duplicacion(Fechas* tabla, struct tm* fecha,
                        wchar_t* lugar, struct tm* prim) {

  if (!tabla->numElems) {
    printf("\nERROR: No hay registros cargados.\n\n");
    return;
  }

  Lugares* l = fechas_buscar(tabla, fecha);
  if (l) {

    int* og = lugares_buscar(l, lugar); /* Valor original de acumulados */
    if (og) {

      printf("\nCasos acumulados en %ls hasta %d-%02d-%02d: %d\n",
      lugar, fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday, *og);

      struct tm* f = malloc(sizeof(struct tm));
      memset(f, 0, sizeof(struct tm));
      actualizar_fecha(f, fecha);

      int obj = *og/2; /* Maximo de casos acumulados que quiere hallar */
      int notifs = 0; /* 0 o primer valor menor o igual a obj */
      int fin = 0; /* Flag que indica si se llego a la primera fecha */
      CasillaFecha* casilla;

      int idx = abs(dias(fecha, tabla->ref));
      for(; !fin; idx = abs(dias(f, tabla->ref))) {

        if (igual_fecha(f, prim))
          fin = 1;

        casilla = &tabla->fechas[idx];
        if (casilla->tabla) {

          if (igual_fecha(f, casilla->fecha))
            notifs = tiempo_duplicacion_aux(casilla->tabla, lugar, obj);
          else if (casilla->sig)
            notifs = tiempo_duplicacion_aux(casilla->sig->tabla, lugar, obj);

          if (notifs) {
            printf("Casos acumulados en %ls hasta %d-%02d-%02d: %d\n",
            lugar, f->tm_year+1900, f->tm_mon+1, f->tm_mday, notifs);

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
      printf("\nERROR: No hay registro de %d-%02d-%02d en %ls.\n\n",
      fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday, lugar);

  } else
    printf("\nERROR: No hay registros de %d-%02d-%02d.\n\n",
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
  fprintf(gnuplotPipe, "set title \"DIARIOS\"\n");
  /* Grafico con lineas sin leyenda */
  fprintf(gnuplotPipe, "plot \"diarios.temp\" u 1:2 w lines notitle\n");
  /* Titulo del segundo grafico */
  fprintf(gnuplotPipe, "set title \"ACUMULADOS\"\n");
  /* Grafico con lineas sin leyenda */
  fprintf(gnuplotPipe, "plot \"acum.temp\" u 1:2 w lines notitle\n");
  fflush(gnuplotPipe); /* Ejecuta todo */
}

/* Funcion auxiliar que recibe una tabla de lugares, un string de lugar
y un puntero a entero de casos acumulados y busca el numero de casos del lugar,
si hay lo retorna y sino retorna 0 */
int graficar_aux(Lugares* tabla, wchar_t* lugar, int* acum) {
  int* notifs = lugares_buscar(tabla, lugar);

  if (notifs) {
    int diarios = *notifs - *acum;
    *acum = *notifs;

    return diarios;
  }

  return 0;
}

void graficar(Fechas* tabla, struct tm** fechas,
              wchar_t* lugar, struct tm** lims) {

  if (!tabla->numElems) {
    printf("\nERROR: No hay registros cargados.\n\n");
    return;
  }

  /* Calcula que pueda haber registros entre las fechas ingresadas
  buscando interseccion con las fechas limite */
  int orden = (dias(lims[1], lims[0]) > 0);

  if (dias(fechas[0], lims[orden]) > 0
      || dias(lims[1 - orden], fechas[1]) > 0) {

    printf("\nERROR: No hay registros entre %d-%02d-%02d y %d-%02d-%02d.\n\n",
    fechas[0]->tm_year+1900, fechas[0]->tm_mon+1, fechas[0]->tm_mday,
    fechas[1]->tm_year+1900, fechas[1]->tm_mon+1, fechas[1]->tm_mday);
    return;
  }

  /* Si hay interseccion pero alguna de las fechas esta fuera de los limites,
  la reemplaza por el limite */
  if (dias(fechas[1], lims[orden]) > 0) {
    printf("\nWARNING: Solo hay registros hasta %d-%02d-%02d\n",
    lims[orden]->tm_year+1900, lims[orden]->tm_mon+1, lims[orden]->tm_mday);

    actualizar_fecha(fechas[1], lims[orden]);
  }
  if (dias(lims[1 - orden], fechas[0]) > 0) {
    printf("\nWARNING: Solo hay registros desde %d-%02d-%02d\n",
    lims[1-orden]->tm_year+1900, lims[1-orden]->tm_mon+1,
    lims[1-orden]->tm_mday);

    actualizar_fecha(fechas[0], lims[1 - orden]);
  }

  /* Abre los archivos donde imprimir los datos a graficar */
  FILE* fp[2];
  fp[0] = fopen("diarios.temp", "w");
  fp[1] = fopen("acum.temp", "w");

  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, fechas[0]);

  int acum = 0; /* Casos acumulados hasta la fecha */
  int fin = 0; /* Flag que indica si se llego a la ultima fecha */
  int diarios = 0; /* Retorno de funcion auxiliar */
  CasillaFecha* casilla;

  int idx = abs(dias(fechas[0], tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {

    if (igual_fecha(f, fechas[1]))
      fin = 1;

    casilla = &tabla->fechas[idx];
    if (casilla->tabla) {

      if (igual_fecha(f, casilla->fecha))
        diarios = graficar_aux(casilla->tabla, lugar, &acum);
      else if (casilla->sig)
        diarios = graficar_aux(casilla->sig->tabla, lugar, &acum);

      if (diarios < 0) {
        printf("\nWARNING: Error de datos en %d-%02d-%02d.\n",
        f->tm_year+1900, f->tm_mon+1, f->tm_mday);
        printf("Cantidad acumulada de confirmados menor a la anterior.\n\n");
        diarios = 0;
      }
    }

    fprintf(fp[0], "%d-%d-%d %d\n",
      f->tm_year+1900, f->tm_mon+1, f->tm_mday, diarios);
    fprintf(fp[1], "%d-%d-%d %d\n",
      f->tm_year+1900, f->tm_mon+1, f->tm_mday, acum);

    diarios = 0;
    agregar_dias(f, 1);
  }
  fclose(fp[0]);
  fclose(fp[1]);

  FILE* gnuplotPipe = popen("gnuplot -persistent", "w");
  comandos_gnuplot(gnuplotPipe, fechas);
  fclose(gnuplotPipe);

  free(f);
}

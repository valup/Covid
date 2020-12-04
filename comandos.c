#define  _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "comandos.h"
#include "straux.h"

/* Recibe una tabla de fechas, un arbol de lugares, un nombre de archivo
y un puntero a estructuras de tiempo (dos, una para la primera fecha y
otra para la ultima) e intenta cargar toda la infrmacion del archivo
(asumiendo el formato
YYYY-MM-DDT00:00-03:00-Depto,LOCALIDAD,confirmados,descartados,estudio,total)
a la tabla y el arbol, guardando la primera y ultima fecha en la estructura
lims (asumiendo que el archivo esta ordenado por fecha) y retornando el arbol */
LTree cargar_dataset(Fechas* tabla, LTree lt, char* archivo, struct tm** lims) {
  FILE* fp; /* Puntero para el archivo */
  struct tm* f = NULL; /* Puntero para crear cada estructura de fecha */
  /* Primero se asegura que se pueda abrir el archivo */
  if ((fp = fopen(archivo, "r"))) {
    char* buf = NULL;
    size_t s = 1; /* size_t cualquiera para pasar a getline */
    /* Se lee la primera linea del archivo, que deberia indicar
    los nombres de las columnas */
    getline(&buf, &s, fp);
    /* Si se leyo solamente el end of file, el archivo es vacio */
    if (feof(fp)) {
      printf("\nERROR: Archivo vacio.\n");
      printf("Ingrese help para mas informacion\n\n");
      free(buf);
      fclose(fp);
      return lt;
    }

    int lim = 0; /* Flag para indicar si se guardo el primer limite */
    /* Se lee la primera linea de datos */
    getline(&buf, &s, fp);
    /* En cada vuelta se confirma que no haya end of file */
    while (!feof(fp)) {
      /* Hasta la T es fecha */
      char* fecha = strtok(buf, "T");
      /* Se intenta convertir a struct tm* */
      f = string_fecha(strim(fecha));
      /* Si no se pudo hay un error en los datos y se detiene todo */
      if (!f) {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        fclose(fp);
        return lt;
      }
      /* Se lee hasta la proxima coma */
      char* depto = strtok(NULL, ",");
      /* Se repite para descartar la parte de 00:00:00-03:00 */
      depto = strtok(NULL, ",");
      /* Si no hay departamento antes de la coma es error y se detiene */
      if (strim(depto)[0] == '\0') {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        free(f);
        fclose(fp);
        return lt;
      }
      /* Se lee hasta la proxima coma para la localidad */
      char* local = strtok(NULL, ",");
      /* Si no hay localidad antes de la coma es error y se detiene */
      if (strim(local)[0] == '\0') {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        free(f);
        fclose(fp);
        return lt;
      }
      /* Se lee el resto para las notificaciones */
      char* cuenta = strtok(NULL, "\n");
      /* Se guarda espacio para ellas */
      int* notifs = malloc(sizeof(int) * 3);
      /* Se escanea para extraer los valores */
      int read = sscanf(cuenta, "%d,%d,%d", notifs, &notifs[1], &notifs[2]);
      /* Si no se escanearon los 3 es error y se detiene todo */
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
      char* lugar = unir(depto, local, ",");
      /* Se intenta insertar el lugar en el arbol,
      si ya estaba se libera el string y se lo reemplaza por el que estaba */
      lt = ltree_insertar(lt, &lugar);
      /* Se inserta toda la informacion del registro en la tabla */
      fechas_insertar(tabla, lugar, f, notifs);
      /* Se libera el buffer y se lee la proxima linea */
      free(buf);
      buf = NULL;
      getline(&buf, &s, fp);
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
  /* Se confirma que el arbol no sea nulo antes de acceder a sus campos */
  if (!lt)
    return;
  /* Se imprimen primero los lugares de la izquierda */
  imprimir_dataset_aux(lt->left, t, fp, f);
  /* Se busca registro del lugar de la raiz */
  int* not = lugares_buscar(t, lt->lugar);
  /* Si se hallan notificaciones se imprimen */
  if (not) {
    fprintf(fp, "%d-%d-%dT00:00:00-03:00,%s,%d,%d,%d,%d\n",
      f->tm_year+1900, f->tm_mon+1, f->tm_mday,
      lt->lugar, not[0], not[1], not[2], not[0]+not[1]+not[2]);
  }
  /* Se sigue con los lugares de la derecha */
  imprimir_dataset_aux(lt->right, t, fp, f);
}

/* Recibe una tabla de fechas, un arbol de lugares, un nombre de archivo,
y una estructura de fechas limite, y se imprimen todos los registros
entre las fechas limite en el archivo, con formato
YYYY-MM-DDT00:00:00-03:00,lugar,confirmados,descartados,estudio,total
ordenados primero por fecha y luego por lugar */
void imprimir_dataset(Fechas* tabla, LTree lt, char* arch, struct tm** lims) {
  FILE* fp = fp = fopen(arch, "w");
  fprintf(fp,
  "Fecha,Departamento,Localidad,Confirmados,Descartados,En estudio,Notificaciones\n"
  );
  /* Si no hay elementos en la tabla no hay nada que imprimir */
  if (!tabla->numElems) {
    fclose(fp);
    return;
  }
  /* Se inicializa una estructura de fecha para buscar en la tabla */
  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  /* Se guarda el primer limite */
  actualizar_fecha(f, lims[0]);
  int orden = 0; /* Indicador del orden de las fechas */
  /* Si la segunda fecha es posterior a la primera es orden ascendente */
  if (dias(lims[1], lims[0]) > 0)
    orden = 1;
  /* Sino descendente */
  else
    orden = -1;
  int fin = 0; /* Flag para indicar si se llego al ultimo limite */
  CasillaFecha* casilla; /* Puntero para simplificar luego el codigo */
  /* Se calcula el primer indice y se empieza a buscar */
  int idx = abs(dias(lims[0], tabla->fechas[0].fecha));
  for(; !fin; idx = abs(dias(f, tabla->fechas[0].fecha))) {
    /* Si se llego a la ultima fecha se setea el fin */
    if (igual_fecha(f, lims[1]))
      fin = 1;
    /* Guarda puntero a la casilla */
    casilla = &tabla->fechas[idx];
    /* Si hay registros en la casilla compara las fechas */
    if (casilla->tabla) {
      /* Si es la misma imprime todos los registros de
      la tabla de lugares de la fecha segun el orden del arbol */
      if (igual_fecha(f, casilla->fecha) && casilla->sig)
        casilla = casilla->sig;
      imprimir_dataset_aux(lt, casilla->tabla, fp, f);
    }
    /* Se busca el siguiente dia a imprimir segun el orden */
    agregar_dias(f, orden);
  }

  fclose(fp);
  free(f);
}

/* Recibe una tabla de fechas, un arbol de lugares, un puntero a strings
de argumento (una fecha, un lugar y notificaciones) y un puntero a fechas limite
y agrega el registro a la tabla, y de ser necesario agrega el lugar al arbol
y actualiza los limites */
LTree agregar_registro(Fechas* tabla, LTree lt, char** args, struct tm** lims) {
  /* Intenta convertir el primer argumento a estructura de fecha,
  si no puede retorna */
  struct tm* f = string_fecha(args[0]);
  if (!f) {
    printf("\nERROR: Fecha invalida.\n\n");
    printf("Ingrese help para mas informacion.\n\n");
    return lt;
  }
  /* Intenta convertir el ultimo argumento a notificaciones,
  si no puede retorna */
  int* notifs = malloc(sizeof(int) * 3);
  int read = sscanf(args[2], "%d|%d|%d", notifs, &notifs[1], &notifs[2]);
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
  /* Guarda espacio para el lugar y lo copia */
  char* lugar = malloc(sizeof(char) * (strlen(args[1]) + 1));
  strcpy(lugar, args[1]);
  /* Inserta en la tabla pero primero
  intenta insertar el lugar en el arbol, y si ya estaba
  se libera y se reemplaza por el lugar del arbol */
  lt = ltree_insertar(lt, &lugar);
  fechas_insertar(tabla, lugar, f, notifs);

  return lt;
}

//agregar_registro 2020-11-23 9 de Julio ANTONIO PINI 5 0 0
//agregar_registro 2020-11-21 9 de Julio ANTONIO PINI 2 0 0
//agregar_registro 2020-11-22 9 de Julio ANTONIO PINI 4 0 0

/* Recibe una tabla de fechas una estructura de fecha y un string de lugar,
y si hay registros de ese lugar en esa fecha los elimina */
void eliminar_registro(Fechas* tabla, struct tm* fecha, char* lugar) {
  Lugares* lugares = fechas_buscar(tabla, fecha);
  if (lugares)
    lugares_eliminar(lugares, lugar);
}

/* Recibe una tabla de fechas, un string de lugar y una estructura
de fechas limite, y busca todos los registros del lugar
entre las fechas limite para hallar el pico */
void buscar_pico(Fechas* tabla, char* lugar, struct tm** lims) {
  /* Si no hay elementos no puede buscar */
  if (!tabla->numElems) {
    printf("\nERROR: No hay registros cargados\n\n");
    return;
  }
  /* Calcula el orden de las fechas para buscar desde la anterior.
  1 = ascendente, 0 = descendente, entonces lims[1 - orden]
  sera la fecha anterior y lims[orden] la posterior */
  int orden = (dias(lims[1], lims[0]) > 0);

  /* Inicializa una estructura de fecha para recorrer la tabla */
  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  /* Guarda la primera fecha en la estructura */
  actualizar_fecha(f, lims[1 - orden]);

  int acum = 0; /* Variable para guardar cantidad acumulada de confirmados */
  int pico = -1; /* Variable para guardar pico de confirmados */
  /* Inicializa una estructura de fecha para la fecha del pico */
  struct tm* fpico = malloc(sizeof(struct tm));
  memset(fpico, 0, sizeof(struct tm));

  int fin = 0; /* Flag para indicar si se llego a la ultima fecha */
  CasillaFecha* casilla; /* Puntero para simplificar luego el codigo */

  /* Calcula la primera posicion y se empieza a buscar */
  int idx = abs(dias(lims[1 - orden], tabla->fechas[0].fecha));
  for(; !fin; idx = abs(dias(f, tabla->fechas[0].fecha))) {
    /* Si se llega a la ultima fecha se setea el fin */
    if (igual_fecha(f, lims[orden]))
      fin = 1;
    /* Guarda puntero a la casilla por simplicidad */
    casilla = &tabla->fechas[idx];
    /* Si hay tabla en la casilla compara las fechas */
    if (casilla->tabla) {
      /* Si no es la misma fecha pero hay otra casilla en la lista,
      esa tiene que ser la de la fecha */
      if (!igual_fecha(f, casilla->fecha) && casilla->sig)
        casilla = casilla->sig;
      /* Busca el registro del lugar */
      int* notifs = lugares_buscar(casilla->tabla, lugar);
      if (notifs) {
        /* Si hay calcula los confirmados diarios
        como la diferencia entre los acumulados nuevos y los anteriores */
        int diarios = notifs[0] - acum;
        /* Si es valor negativo hay error en los datos y detiene todo */
        if (diarios < 0) {
          printf("\nERROR: Datos incorrectos en %d-%d-%d\n\n",
            f->tm_year+1900, f->tm_mon+1, f->tm_mday);
          free(f);
          free(fpico);
          return;
        /* Sino actualiza los acumulados */
        } else {
          acum = notifs[0];
          /* Si es un nuevo pico actualiza la informacion del pico */
          if (diarios > pico) {
            pico = diarios;
            actualizar_fecha(fpico, casilla->fecha);
          }
        }
      }
    }
    /* Se sigue al proximo dia */
    agregar_dias(f, 1);
  }
  /* Si se hallo un pico se imprime el valor y la fecha correspondiente */
  if (pico > -1) {
    printf("\nPico de %s: %d en %d-%d-%d\n\n", lugar, pico,
      fpico->tm_year+1900, fpico->tm_mon+1, fpico->tm_mday);
  }

  free(f);
  free(fpico);
}

/* Recibe una tabla de fechas, una estructura de fecha y un string de lugar,
busca registro del lugar en la fecha y si lo halla imprime los casos
confirmados acumulados */
void casos_acumulados(Fechas* tabla, struct tm* fecha, char* lugar) {
  /* Busca la fecha en la tabla */
  Lugares* l = fechas_buscar(tabla, fecha);
  /* Si hay registros para la fecha busca el lugar */
  if (l) {
    int* notifs = lugares_buscar(l, lugar);
    /* Si halla registro imprime los casos confirmados acumulados */
    if (notifs) {
      printf("\n%d casos acumulados en %s hasta %d-%d-%d\n\n",
      notifs[0], lugar, fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday);
    } else {
      printf("\nERROR: No hay registro del lugar en la fecha\n\n");
    }
  } else {
    printf("\nERROR: No hay registros para la fecha\n\n");
  }
}

void tiempo_duplicacion(Fechas* tabla, struct tm* fecha, char* lugar, struct tm* prim) {
  if (!tabla->numElems) {
    printf("\nERROR: no hay dataset cargado\n\n");
    return;
  }

  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, fecha);

  int* og; /* Variable para guardar el valor original de casos acumulados */
  /* Empieza buscando la fecha en la tabla */
  Lugares* l = fechas_buscar(tabla, fecha);
  /* Si hay registros para la fecha busca el lugar */
  if (l) {
    og = lugares_buscar(l, lugar);
    if (!og) {
      printf("\nERROR: No hay registro del lugar en la fecha\n\n");
      free(f);
      return;
    }
  } else {
    printf("\nERROR: No hay registros para la fecha\n\n");
  }
  int obj = *og/2;

  int fin = 0;
  CasillaFecha* casilla;

  int idx = abs(dias(fecha, tabla->fechas[0].fecha));
  for(; !fin; idx = abs(dias(f, tabla->fechas[0].fecha))) {
    if (igual_fecha(f, prim))
      fin = 1;
    /* Guarda un puntero a la casilla por simplicidad */
    casilla = &tabla->fechas[idx];
    if (casilla->tabla) {
      if (!igual_fecha(f, casilla->fecha) && casilla->sig)
        casilla = casilla->sig;
      int* notifs = lugares_buscar(casilla->tabla, lugar);
      if (notifs && *notifs <= obj) {
        obj = dias(fecha, casilla->fecha);
        printf("\nTiempo de duplicacion: %d dias\n\n", obj);
        free(f);
        return;
      }
    }
    agregar_dias(f, -1);
  }
  printf("\nERROR: No hay registros de dias con la mitad o menos de casos acumulados\n\n");

  free(f);
}

void comandos_gnuplot(FILE* gnuplotPipe, struct tm** fechas) {
  fprintf(gnuplotPipe, "set xdata time\n");
  fprintf(gnuplotPipe, "%s\n", "set timefmt \"%Y-%m-%d\"");
  fprintf(gnuplotPipe, "%s\n", "set format x \"%d\\n%m\\n%Y\"");
  fprintf(gnuplotPipe, "set xrange [\"%d-%d-%d\":\"%d-%d-%d\"]\n",
    fechas[0]->tm_year+1900, fechas[0]->tm_mon+1, fechas[0]->tm_mday,
    fechas[1]->tm_year+1900, fechas[1]->tm_mon+1, fechas[1]->tm_mday);
  fprintf(gnuplotPipe, "set multiplot layout 2,1\n");
  fprintf(gnuplotPipe, "set title \"Diarios\"\n");
  fprintf(gnuplotPipe, "plot \"diarios.temp\" u 1:2 w lines notitle\n");
  fprintf(gnuplotPipe, "set title \"Acumulados\"\n");
  fprintf(gnuplotPipe, "plot \"acum.temp\" u 1:2 w lines notitle\n");
  fflush(gnuplotPipe);
}

void graficar(Fechas* tabla, struct tm** fechas, char* lugar, struct tm** lims) {
  if (!tabla->numElems) {
    printf("\nERROR: no hay dataset cargado\n\n");
    return;
  }

  int orden = (dias(lims[1], lims[0]) > 0);
  if (dias(fechas[0], lims[orden]) > 0 || dias(lims[1 - orden], fechas[1]) > 0) {
    printf("\nERROR: No hay registros entre las fechas\n\n");
    return;
  }
  if (dias(fechas[1], lims[orden]) > 0)
    actualizar_fecha(fechas[1], lims[orden]);
  if (dias(lims[1 - orden], fechas[0]) > 0)
    actualizar_fecha(fechas[0], lims[1 - orden]);

  FILE* fp[2];
  fp[0] = fopen("diarios.temp", "w");
  fp[1] = fopen("acum.temp", "w");

  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, fechas[0]);

  int acum = 0;
  int fin = 0;
  CasillaFecha* casilla;

  int idx = abs(dias(fechas[0], tabla->fechas[0].fecha));
  for(; !fin; idx = abs(dias(f, tabla->fechas[0].fecha))) {
    if (igual_fecha(f, fechas[1]))
      fin = 1;
    /* Guarda un puntero a la casilla por simplicidad */
    casilla = &tabla->fechas[idx];
    if (casilla->tabla) {
      if (igual_fecha(f, casilla->fecha) && casilla->sig)
        casilla = casilla->sig;
      int* notifs = lugares_buscar(casilla->tabla, lugar);
      if (notifs && *notifs - acum > 0) {
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
//graficar 2020-03-24 2020-11-22 Rosario ROSARIO

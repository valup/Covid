#define  _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "comandos.h"
#include "straux.h"

LTree cargar_dataset(Fechas* tabla, LTree lt, char* archivo, struct tm** lims) {
  FILE* fp;
  struct tm* f;
  fp = fopen(archivo, "a");
  fprintf(fp, "\n");
  fclose(fp);

  if ((fp = fopen(archivo, "r"))) {
    char* buf = NULL;
    size_t s = 1;
    if (!(getline(&buf, &s, fp))) {
      printf("\nERROR: Archivo vacio.\n");
      printf("Ingrese help para mas informacion\n\n");
      return lt;
    }
    int lim = 0, orden = 0;

    getline(&buf, &s, fp);
    while (buf[0] != '\n') {
      char* fecha = strtok(buf, "T");
      f = string_fecha(strim(fecha));
      if (!f) {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        return lt;
      }

      char* depto = strtok(NULL, ",");
      depto = strtok(NULL, ",");
      if (strim(depto)[0] == '\0') {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        return lt;
      }
      char* local = strtok(NULL, ",");
      if (strim(local)[0] == '\0') {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(buf);
        return lt;
      }
      char* cuenta = strtok(NULL, "\n");
      int* notifs = malloc(sizeof(int) * 3);
      int read = sscanf(cuenta, "%d,%d,%d", notifs, &notifs[1], &notifs[2]);

      if (read < 3) {
        printf("\nERROR: Error de datos. No se pudo cargar toda la tabla.\n\n");
        free(notifs);
        free(buf);
        return lt;
      }

      if (!lim) {
        actualizar_fecha(lims[1], f);
        actualizar_fecha(lims[0], f);
        lim = 1;
      } else if (!orden) {
        if (dias(f, lims[1]) > 0) {
          orden = 1;
        } else if (dias(f, lims[0]) < 0) {
          orden = -1;
        }
      }

      char* lugar = unir(depto, local, ",");

      lt = ltree_insertar(lt, &lugar);
      fechas_insertar(tabla, lugar, f, notifs);

      getline(&buf, &s, fp);
    }

    if (orden == -1 && dias(lims[0], f) > 0) {
      actualizar_fecha(lims[0], f);
    } else if (orden == 1 && dias(f, lims[1]) > 0) {
      actualizar_fecha(lims[1], f);
    }
    free(buf);
    fclose(fp);
  } else {
    printf("\nERROR: No se puede abrir el archivo.\n\n");
  }

  return lt;
}

void imprimir_dataset_aux(LTree lt, Lugares* t, FILE* fp, struct tm* f) {
  if (lt == NULL)
    return;
  imprimir_dataset_aux(lt->left, t, fp, f);
  int* not = lugares_buscar(t, lt->lugar);
  if (not) {
    fprintf(fp, "%d-%d-%dT00:00:00-03:00,", f->tm_year+1900, f->tm_mon+1, f->tm_mday);
    fprintf(fp, "%s,%d,%d,%d,%d\n", lt->lugar, not[0], not[1], not[2], not[0]+not[1]+not[2]);
  }
  imprimir_dataset_aux(lt->right, t, fp, f);
}

void imprimir_dataset(Fechas* tabla, LTree lt, char* archivo, struct tm** lims) {
  FILE* fp = fp = fopen(archivo, "w");
  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, lims[1]);

  fprintf(fp, "Fecha,Departamento,Localidad,Confirmados,Descartados,En estudio,Notificaciones\n");

  int fin = 0;
  int idx = abs(dias(lims[1], tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {
    if (igual_fecha(f, lims[0]))
      fin = 1;
    if (tabla->fechas[idx].tabla) {
      if (igual_fecha(f, tabla->fechas[idx].fecha)) {
        imprimir_dataset_aux(lt, tabla->fechas[idx].tabla, fp, f);
      } else if (tabla->fechas[idx].sig) {
        imprimir_dataset_aux(lt, tabla->fechas[idx].sig->tabla, fp, f);
      }
    }
    agregar_dias(f, -1);
  }
  fprintf(fp, "\n");
  fclose(fp);

  free(f);
}

LTree agregar_registro(Fechas* tabla, LTree lt, char** args, int* notifs, struct tm* lims[2]) {
  struct tm* f = string_fecha(args[0]);
  if (!f) {
    printf("\nERROR: Fecha invalida.\n\n");
    return lt;
  }

  if (dias(lims[1], f) < 0) {
    actualizar_fecha(lims[1], f);
  } else if (dias(lims[0], f) > 0) {
    actualizar_fecha(lims[0], f);
  }

  char* lugar = malloc(sizeof(char) * (strlen(args[1]) + 1));
  strcpy(lugar, args[1]);

  lt = ltree_insertar(lt, &lugar);
  fechas_insertar(tabla, lugar, f, notifs);

  return lt;
}

//agregar_registro 2020-11-23 9 de Julio ANTONIO PINI 5 0 0
//agregar_registro 2020-11-21 9 de Julio ANTONIO PINI 2 0 0
//agregar_registro 2020-11-22 9 de Julio ANTONIO PINI 4 0 0


void eliminar_registro(Fechas* tabla, struct tm* fecha, char* lugar) {
  Lugares* lugares = fechas_buscar(tabla, fecha);
  if (lugares)
    lugares_eliminar(lugares, lugar);
}

int buscar_pico_aux(Lugares* tabla, char* lugar, int* pico, int* acum) {
  int* notifs = lugares_buscar(tabla, lugar);
  if (notifs) {
    int diarios = notifs[0] - *acum;
    if (diarios < 0) {
      return -1;
    } else {
      *acum = notifs[0];
      if (diarios > *pico) {
        *pico = diarios;
        return 1;
      }
    }
  }
  return 0;
}

void buscar_pico(Fechas* tabla, char* lugar, struct tm** lims) {
  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, lims[0]);

  int acum = 0;
  int pico = -1;
  struct tm* fpico = malloc(sizeof(struct tm));
  memset(fpico, 0, sizeof(struct tm));
  int fin = 0;
  int idx = abs(dias(lims[0], tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {
    if (igual_fecha(f, lims[1]))
      fin = 1;
    if (tabla->fechas[idx].tabla) {
      int p;
      if (igual_fecha(f, tabla->fechas[idx].fecha)) {
        p = buscar_pico_aux(tabla->fechas[idx].tabla, lugar, &pico, &acum);
        if (p == -1) {
          printf("\nERROR: Datos incorrectos en ");
          printf("%d-%d-%d\n\n", f->tm_year+1900, f->tm_mon+1, f->tm_mday);
          return;
        } else if (p == 1) {
          actualizar_fecha(fpico, f);
        }
      } else if (tabla->fechas[idx].sig->tabla) {
        if (p == -1) {
          p = buscar_pico_aux(tabla->fechas[idx].sig->tabla, lugar, &pico, &acum);
          printf("\nERROR: Datos incorrectos en ");
          printf("%d-%d-%d\n\n", f->tm_year+1900, f->tm_mon+1, f->tm_mday);
          return;
        } else if (p == 1) {
          actualizar_fecha(fpico, f);
        }
      }
    }
    agregar_dias(f, 1);
  }
  if (pico > -1) {
    printf("\nPico de %s: %d en ", lugar, pico);
    printf("%d-%d-%d\n\n", fpico->tm_year+1900, fpico->tm_mon+1, fpico->tm_mday);
  }

  free(f);
  free(fpico);
}

int* casos_acumulados_aux(Fechas* tabla, struct tm* fecha, char* lugar) {
  Lugares* l = fechas_buscar(tabla, fecha);
  if (l) {
    return lugares_buscar(l, lugar);
  } else {
    return NULL;
  }
}

void casos_acumulados(Fechas* tabla, struct tm* fecha, char* lugar) {
  int* notifs = casos_acumulados_aux(tabla, fecha, lugar);
  if (notifs) {
    printf("\n%d casos acumulados en %s hasta ", notifs[0], lugar);
    printf("%d-%d-%d\n\n", fecha->tm_year+1900, fecha->tm_mon+1, fecha->tm_mday);
  } else {
    printf("\nERROR: No hay registros para la fecha\n\n");
  }
}

int tiempo_duplicacion_aux(Lugares* tabla, char* lugar, int obj) {
  int* notifs = lugares_buscar(tabla, lugar);
  if (notifs && *notifs <= obj)
    return 1;
  return 0;
}

void tiempo_duplicacion(Fechas* tabla, struct tm* fecha, char* lugar, struct tm* prim) {
  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, fecha);

  int* og = casos_acumulados_aux(tabla, fecha, lugar);
  if (!og) {
    printf("\nERROR: No hay registros para la fecha\n\n");
    return;
  }
  int obj = *og/2;

  int fin = 0;
  int idx = abs(dias(fecha, tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {
    if (igual_fecha(f, prim))
      fin = 1;
    if (tabla->fechas[idx].tabla) {
      int p;
      if (igual_fecha(f, tabla->fechas[idx].fecha)) {
        p = tiempo_duplicacion_aux(tabla->fechas[idx].tabla, lugar, obj);
        if (p == 1) {
          obj = dias(fecha, tabla->fechas[idx].fecha);
          printf("\nTiempo de duplicacion: %d dias\n\n", obj);
          return;
        }
      } else if (tabla->fechas[idx].sig->tabla) {
        p = tiempo_duplicacion_aux(tabla->fechas[idx].sig->tabla, lugar, obj);
        if (p == 1) {
          obj = dias(fecha, tabla->fechas[idx].sig->fecha);
          printf("Tiempo de duplicacion: %d dias\n", obj);
          return;
        }
      }
    }
    agregar_dias(f, -1);
  }
  printf("\nERROR: No hay registros de dias con la mitad o menos de casos acumulados\n\n");

  free(f);
}

void graficar_aux(Lugares* t, FILE** fp, char* lugar, struct tm* f, int* acum) {
  int* not = lugares_buscar(t, lugar);
  if (not && *not - *acum > 0) {
    fprintf(fp[0], "%d-%d-%d ", f->tm_year+1900, f->tm_mon+1, f->tm_mday);
    fprintf(fp[0], "%d\n", *not - *acum);

    fprintf(fp[1], "%d-%d-%d ", f->tm_year+1900, f->tm_mon+1, f->tm_mday);
    fprintf(fp[1], "%d\n", not[0]);
    *acum = *not;
  }
}

void comandos_gnuplot(FILE* gnuplotPipe, struct tm** fechas) {
  fprintf(gnuplotPipe, "%s\n", "set xdata time");
  fprintf(gnuplotPipe, "%s\n", "set timefmt \"%Y-%m-%d\"");
  fprintf(gnuplotPipe, "%s\n", "set format x \"%d\\n%m\\n%Y\"");
  fprintf(gnuplotPipe, "%s", "set xrange [\"");
  fprintf(gnuplotPipe, "%d-%d", fechas[0]->tm_year+1900, fechas[0]->tm_mon+1);
  fprintf(gnuplotPipe, "-%d\":\"%d", fechas[0]->tm_mday, fechas[1]->tm_year+1900);
  fprintf(gnuplotPipe, "-%d-%d\"]\n", fechas[1]->tm_mon+1, fechas[1]->tm_mday);
  fprintf(gnuplotPipe, "%s\n", "set multiplot layout 2,1");
  fprintf(gnuplotPipe, "%s\n", "set title \"Diarios\"");
  fprintf(gnuplotPipe, "%s\n", "plot \"diarios.temp\" u 1:2 w lines notitle");
  fprintf(gnuplotPipe, "%s\n", "set title \"Acumulados\"");
  fprintf(gnuplotPipe, "%s\n", "plot \"acum.temp\" u 1:2 w lines notitle");
  fflush(gnuplotPipe);
}

void graficar(Fechas* tabla, struct tm** fechas, char* lugar, struct tm** lims) {
  if (dias(fechas[0], lims[1]) > 0 || dias(lims[0], fechas[1]) > 0) {
    printf("\nERROR: No hay registros entre las fechas\n\n");
    return;
  }
  if (dias(fechas[1], lims[1]) > 0)
    actualizar_fecha(fechas[1], lims[1]);
  if (dias(lims[0], fechas[0]) > 0)
    actualizar_fecha(fechas[0], lims[0]);

  FILE* fp[2];
  fp[0] = fopen("diarios.temp", "w");
  fp[1] = fopen("acum.temp", "w");

  struct tm* f = malloc(sizeof(struct tm));
  memset(f, 0, sizeof(struct tm));
  actualizar_fecha(f, fechas[0]);

  int acum = 0;
  int fin = 0;
  int idx = abs(dias(fechas[0], tabla->ref));
  for(; !fin; idx = abs(dias(f, tabla->ref))) {
    if (igual_fecha(f, fechas[1]))
      fin = 1;
    if (tabla->fechas[idx].tabla) {
      if (igual_fecha(f, tabla->fechas[idx].fecha)) {
        graficar_aux(tabla->fechas[idx].tabla, fp, lugar, f, &acum);
      } else if (tabla->fechas[idx].sig) {
        graficar_aux(tabla->fechas[idx].sig->tabla, fp, lugar, f, &acum);
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
//graficar 2020-07-08 2020-11-22 Rosario ROSARIO

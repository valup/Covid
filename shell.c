#define  _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hashf.h"
#include "ltree.h"
#include "comandos.h"
#include "straux.h"

//cargar_dataset d.csv

void help() {
  printf("TO DO\n");
}

LTree procesar(Fechas* tabla, LTree lt, char* buf, struct tm** lims) {

  if (!strcmp(buf, "help"))
    help();

  char* com = strtok(buf, " ");
  char* arg = strim(strtok(NULL, "\n"));
  if (strim(arg)[0] == '\0') {
    printf("\nERROR: Faltan argumentos.\n");
    printf("Ingrese help para mas informacion.\n\n");
    return lt;
  }

  if (!strcmp(com, "cargar_dataset")) {
    lt = cargar_dataset(tabla, lt, arg, lims);

  } else if (!strcmp(com, "imprimir_dataset")) {
    imprimir_dataset(tabla, lt, arg, lims);

  } else if (!strcmp(com, "agregar_registro")) {
    char* resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_notifs(resto)) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, '-')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    char* args[3];
    args[0] = strtok(arg, "|");
    args[1] = strtok(NULL, "|");
    args[2] = strtok(NULL, "\n");

    int* notifs = malloc(sizeof(int) * 3);
    int read = sscanf(args[2], "%d|%d|%d", notifs, &notifs[1], &notifs[2]);
    if (read < 3) {
      printf("\nERROR: Numero invalido.\n");
      printf("Ingrese help para mas informacion.\n\n");
      free(notifs);
      return lt;
    }

    lt = agregar_registro(tabla, lt, args, notifs, lims);

  } else if (!strcmp(com, "eliminar_registro")) {
    char* resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, '\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    char* args[2];
    args[0] = strtok(arg, "|");
    struct tm* tm = string_fecha(args[0]);
    if (!tm) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    args[1] = strtok(NULL, "|");

    eliminar_registro(tabla, tm, args[1]);
    free(tm);

  } else if (!strcmp(com, "buscar_pico")) {
    if (!marcar_lugar(arg, '\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
    }

    buscar_pico(tabla, arg, lims);

  } else if (!strcmp(com, "casos_acumulados")) {
    char* resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, '\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    char* args[2];
    args[0] = strtok(arg, "|");
    struct tm* tm = string_fecha(args[0]);
    if (!tm) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    } else if (dias(tm, lims[1]) > 0 || dias(lims[0], tm) > 0) {
      printf("\nERROR: No hay registros para la fecha.\n");
      free(tm);
      return lt;
    }

    args[1] = strtok(NULL, "|");

    casos_acumulados(tabla, tm, args[1]);
    free(tm);

  } else if (!strcmp(com, "tiempo_duplicacion")) {
    char* resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, '\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    char* args[2];
    args[0] = strtok(arg, "|");
    struct tm* tm = string_fecha(args[0]);
    if (!tm) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    } else if (dias(tm, lims[1]) > 0 || dias(lims[0], tm) > 0) {
      printf("\nERROR: No hay registros para la fecha.\n");
      free(tm);
      return lt;
    }

    args[1] = strtok(NULL, "|");

    tiempo_duplicacion(tabla, tm, args[1], lims[0]);
    free(tm);

  } else if (!strcmp(com, "graficar")) {
    char* resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, '\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    char* args[3];
    struct tm* tm[2];
    for (int i = 0; i < 2; i++) {
      if (i == 0) {
        args[i] = strtok(arg, "|");
      } else
        args[i] = strtok(NULL, "|");
      tm[i] = string_fecha(args[i]);
      if (!tm[i]) {
        printf("\nERROR: Faltan argumentos.\n");
        printf("Ingrese help para mas informacion.\n\n");
        if (i == 1)
          free(tm[0]);
        return lt;
      }
    }

    args[2] = strtok(NULL, "\n");

    graficar(tabla, tm, args[2], lims);
    free(tm[0]);
    free(tm[1]);

  } else {
    printf("\nERROR: Comando incorrecto.\n\n");
  }

  return lt;
}

int main() {
  printf("\nIngrese 'help' para informacion sobre los comandos\n\n");

  Fechas* tabla = fechas_crear(LUGARES_INI);
  LTree lt = ltree_crear();
  struct tm* lims[2];
  lims[0] = malloc(sizeof(struct tm));
  memset(lims[0], 0, sizeof(struct tm));
  lims[1] = malloc(sizeof(struct tm));
  memset(lims[1], 0, sizeof(struct tm));

  char* buf = NULL;
  size_t s = 1;

  while (1) {
    printf(">> ");
    getline(&buf, &s, stdin);

    if (!strcmp(buf, "salir\n")) {
      fechas_destruir(tabla);
      ltree_destruir(lt);
      free(buf);
      free(lims[0]);
      free(lims[1]);
      return 0;
    }

    char* b = strim_izq(buf);
    if (b[0] != '\0')
      lt = procesar(tabla, lt, b, lims);
  }

  return 0;
}

#define  _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include "hashf.h"
#include "ltree.h"
#include "comandos.h"
#include "straux.h"

//cargar_dataset d.csv

/* Imprime informacion sobre el programa */
void help() {
  printf("\nComandos:\n\n");
  printf("  cargar_dataset nombreDeArchivo:\n");
  printf("    Carga los datos del archivo al programa.\n");
  printf("    Se asume que la primera linea tiene los nombres de columna\n");
  printf("    y que las lineas de datos tienen el siguiente formato:\n");
  printf("    AAAA-MM-DDT00:00:00-03:00,Departamento,LOCALIDAD,");
  printf("confirmados,descartados,enEstudio,total\n\n");
  printf("  imprimir_dataset nombreDeArchivo:\n");
  printf("    Imprime los datos cargados al programa en el archivo\n");
  printf("    con el formato anterior.\n\n");
  printf("  agregar_registro AAAA-MM-DD Departamento LOCALIDAD ");
  printf("confirmados descartados enEstudio:\n");
  printf("    Agrega un registro a la tabla o si ya existe lo actualiza\n\n");
  printf("  eliminar_registro AAAA-MM-DD Departamento LOCALIDAD:\n");
  printf("    Elimina un registro del programa si existe.\n\n");
  printf("  buscar_pico Departamento LOCALIDAD:\n");
  printf("    Busca la mayor cantidad de casos confirmados diarios\n");
  printf("    registrados para la localidad.\n\n");
  printf("  casos_acumulados AAAA-MM-DD Departamento LOCALIDAD:\n");
  printf("    Busca los casos confirmados acumulados registrados\n");
  printf("    en la fecha en la localidad.\n\n");
  printf("  tiempo_duplicacion AAAA-MM-DD Departamento LOCALIDAD:\n");
  printf("    Calcula a cantidad de dias que tardaron en duplicarse\n");
  printf("    los casos de la localidad hasta llegar a los de la fecha.\n\n");
  printf("  graficar AAAA-MM-DD AAAA-MM-DD Departamento LOCALIDAD:\n");
  printf("    Grafica la evolucion de los casos diarios y acumulados\n");
  printf("    en la localidad entre las fechas.\n\n");
  printf("  salir: Cierra el programa.\n\n");
  printf("Nota: La localidad se debe escribir toda en mayusculas, ");
  printf("el departamento solo la primera letra.\n\n");
}

/* Recibe una tabla de fechas, un arbol de lugares, un buffer con comando
y un puntero a estructuras de fechas limite, y procesa el comando
realizando las operaciones necesarias */
LTree procesar(Fechas* tabla, LTree lt, wchar_t* buf, struct tm** lims) {
  /* Si el comando es help llama a la funcion */
  if (!wcscoll(buf, L"help\n")) {
    help();
    return lt;
  }

  wchar_t* pt; /* Puntero para wcstok */
  wchar_t* com = wcstok(buf, L" ", &pt); /* Toma el comando */
  wchar_t* arg = wcstok(NULL, L"\n", &pt); /* Toma los otros argumentos */
  /* Si no hay es error */
  if (!arg) {
    printf("\nERROR: Comando incorrecto.\n");
    printf("Ingrese help para mas informacion.\n\n");
    return lt;
  }
  /* Si es cargar_dataset el argumento es archivo y lo pasa a la funcion */
  if (!wcscoll(com, L"cargar_dataset")) {
    double comienzo = clock();
    lt = cargar_dataset(tabla, lt, arg, lims);
    double final = clock();
    printf("%f segundos\n", (final - comienzo) / CLOCKS_PER_SEC);
  /* Si es imprimir_dataset el argumento es archivo y lo pasa a la funcion */
  } else if (!wcscoll(com, L"imprimir_dataset")) {
    double comienzo = clock();
    imprimir_dataset(tabla, lt, arg, lims);
    double final = clock();
    printf("%f segundos\n", (final - comienzo) / CLOCKS_PER_SEC);
  /* Si es agregar_registro separa los argumentos y los pasa a la funcion */
  } else if (!wcscoll(com, L"agregar_registro")) {
    /* Separa con | los argumentos
    (fecha departamento localidad confirmados descartados estudio)
    y une departamento y localidad con coma */
    wchar_t* resto = marcar_fecha(arg);
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
    if (!marcar_lugar(resto, L'|')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    /* Lee los argumentos por partes */
    wchar_t* args[3];
    args[0] = wcstok(arg, L"|", &pt);
    args[1] = wcstok(NULL, L"|", &pt);
    args[2] = wcstok(NULL, L"\n", &pt);

    lt = agregar_registro(tabla, lt, args, lims);
  /* Si es eliminar_registro separa los argumentos y los pasa a la funcion */
  } else if (!wcscoll(com, L"eliminar_registro")) {
    /* Separa con | los argumentos (fecha departamento localidad)
    y une departamento y localidad con coma */
    wchar_t* resto = marcar_fecha(arg);
    if (!resto) {
    printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, L'\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    /* Toma los argumentos por partes */
    wchar_t* args[2];
    args[0] = wcstok(arg, L"|", &pt);
    /* Intenta convertir el primero a fecha valida */
    struct tm* tm = string_fecha(args[0]);
    if (!tm) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    args[1] = wcstok(NULL, L"|", &pt);

    eliminar_registro(tabla, tm, args[1], lims);
    free(tm);
  /* Si es buscar_pico une departamento y localidad con coma y los pasa */
  } else if (!wcscoll(com, L"buscar_pico")) {
    if (!marcar_lugar(arg, L'\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
    }

    buscar_pico(tabla, arg, lims);
  /* Si es casos_acumulados separa los argumentos y los pasa a la funcion */
  } else if (!wcscoll(com, L"casos_acumulados")) {
    /* Separa con | los argumentos (fecha departamento localidad)
    y une departamento y localidad con coma */
    wchar_t* resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, L'\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    wchar_t* args[2];
    args[0] = wcstok(arg, L"|", &pt);

    struct tm* tm = string_fecha(args[0]);
    if (!tm) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    } else {
      int orden = (dias(lims[1], lims[0]) > 0);
      if (dias(tm, lims[orden]) > 0 || dias(lims[1 - orden], tm) > 0) {
        printf("\nERROR: No hay registros para la fecha.\n\n");
        free(tm);
        return lt;
      }
    }
    args[1] = wcstok(NULL, L"|", &pt);

    casos_acumulados(tabla, tm, args[1]);
    free(tm);
  /* Si es tiempo_duplicacion separa los argumentos y los pasa a la funcion */
  } else if (!wcscoll(com, L"tiempo_duplicacion")) {
    /* Separa con | los argumentos (fecha departamento localidad)
    y une departamento y localidad con coma */
    wchar_t* resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, L'\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    wchar_t* args[2];
    args[0] = wcstok(arg, L"|", &pt);
    struct tm* tm = string_fecha(args[0]);
    if (!tm) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    } else {
      int orden = (dias(lims[1], lims[0]) > 0);
      if (dias(tm, lims[orden]) > 0 || dias(lims[1 - orden], tm) > 0) {
        printf("\nERROR: No hay registros para la fecha.\n\n");
        free(tm);
        return lt;
      }
    }

    args[1] = wcstok(NULL, L"|", &pt);

    int orden = (dias(lims[1], lims[0]) < 0);
    tiempo_duplicacion(tabla, tm, args[1], lims[orden]);
    free(tm);
  /* Si es graficar separa los argumentos y los pasa a la funcion */
  } else if (!wcscoll(com, L"graficar")) {
    /* Separa con | los argumentos (fecha1 fecha2 departamento localidad)
    y une departamento y localidad con coma */
    wchar_t* resto = marcar_fecha(arg);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    resto = marcar_fecha(resto);
    if (!resto) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }
    if (!marcar_lugar(resto, L'\0')) {
      printf("\nERROR: Faltan argumentos.\n");
      printf("Ingrese help para mas informacion.\n\n");
      return lt;
    }

    wchar_t* args[3];
    struct tm* tm[2];
    /* Intenta convertir los primeros 2 argumentos a fechas validas */
    for (int i = 0; i < 2; i++) {
    if (i == 0) {
        args[i] = wcstok(arg, L"|", &pt);
      } else
        args[i] = wcstok(NULL, L"|", &pt);
      tm[i] = string_fecha(args[i]);
      if (!tm[i]) {
        printf("\nERROR: Faltan argumentos.\n");
        printf("Ingrese help para mas informacion.\n\n");
        if (i == 1)
          free(tm[0]);
        return lt;
      }
    }
    /* Confirma que la primera fecha sea anterior */
    if (dias(tm[1], tm[0]) > 0) {
      args[2] = wcstok(NULL, L"\n", &pt);
      graficar(tabla, tm, args[2], lims);
    } else
      printf("\nERROR: Orden incorrecto de fechas.\n\n");

    free(tm[0]);
    free(tm[1]);

  } else {
    printf("\nERROR: Comando incorrecto.\n");
    printf("Ingrese help para mas informacion.\n\n");
  }

  return lt;
}

int main() {
  setlocale(LC_ALL, ""); /* Define el orden alfabetico */

  printf("\nIngrese 'help' para informacion sobre los comandos\n\n");

  Fechas* tabla = fechas_crear(LUGARES_INI); /* Tabla de fechas */
  LTree lt = ltree_crear(); /* Arbol de lugares */
  struct tm* lims[2]; /* Estructuras de fechas limite */
  /* Inicializa las estructuras */
  lims[0] = malloc(sizeof(struct tm));
  memset(lims[0], 0, sizeof(struct tm));
  lims[1] = malloc(sizeof(struct tm));
  memset(lims[1], 0, sizeof(struct tm));

  wchar_t* buf = NULL; /* Buffer para comandos */

  while (1) {
    printf(">> ");
    /* Lee toda la entrada como wchar_t* */
    buf = wgetline(buf, stdin);
    /* Sigue si el string contiene algo mas que nueva linea */
    if (buf[0] != L'\n') {
      /* Si pide salir se libera todo */
      if (!wcscoll(buf, L"salir\n")) {
        fechas_destruir(tabla);
        ltree_destruir(lt);
        free(buf);
        free(lims[0]);
        free(lims[1]);
        return 0;
      }
      /* Sino procesa el comando */
      lt = procesar(tabla, lt, buf, lims);
    }
    /* Libera el buffer para volver a leer */
    free(buf);
  }

  return 0;
}

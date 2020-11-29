#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include "straux.h"

char* unir(char* str1, char* str2, char* sep) {
  char* str = malloc(sizeof(char) * (strlen(str1) + strlen(str2) + strlen(sep) + 1));
  strcpy(str, str1);
  strcat(str, sep);
  strcat(str, str2);
  return str;
}

/* Recibe un string y lo devuelve un puntero
al primer caracter no espacio */
char* strim_izq(char* str) {
  if (!str)
    return NULL;
  for (;str[0] == ' '; str++);
  return str;
}

/* Recibe un string y elimina los espacios luego del ultimo
caracter no espacio */
char* strim_der(char* str) {
  if (!str)
    return NULL;
  size_t i = strlen(str);
  for (;i > 0 && str[i-1] == ' '; i--);
  str[i] = '\0';
  return str;
}

// Recibe un string y remueve los espacios de ambos extremos
char* strim(char* str) {
  return strim_der(strim_izq(str));
}

int dias(struct tm* f1, struct tm* f2) {
  return difftime(mktime(f1), mktime(f2))/(24.0*60*60);
}

struct tm* string_fecha(char* fecha) {
  int partes[3];
  int n = sscanf(fecha, "%d-%d-%d", partes, &partes[1], &partes[2]);

  if (n != 3)
    return NULL;
  if (partes[0] < 2020) {
    printf("\nERROR: Fechas deben ser a partir de 2020.\n\n");
    return NULL;
  }

  partes[0] -= 1900;
  partes[1] -= 1;

  struct tm* tm = malloc(sizeof(struct tm));
  memset(tm, 0, sizeof(struct tm));

  tm->tm_year = partes[0];
  tm->tm_mon = partes[1];
  tm->tm_mday = partes[2];

  if (!mktime(tm) || tm->tm_year != partes[0] || tm->tm_mon != partes[1] || tm->tm_mday != partes[2]) {
    free(tm);
    return NULL;
  }

  time_t ahora;
  time(&ahora);
  struct tm *hoy = localtime(&ahora);

  if (dias(tm, hoy) > 0) {
    printf("\nERROR: Fecha futura.\n\n");
    return NULL;
  }

  return tm;
}

char* marcar_fecha(char* buf) {
  char* p = strchr(buf, ' ');
  if (!p || *(p + 1) == '\n' || *(p + 1) == '\0')
    return NULL;

  *p = '|';

  return p + 1;
}

int marcar_notifs(char* buf) {
  char* p;
  for (int i = 0; i < 3; i++) {
    p = strrchr(buf, ' ');
    if (!p || p == buf)
      return 0;

    *p = '|';
  }
  return 1;
}

int marcar_lugar(char* buf, char hasta) {
  int i = 0;
  for (; buf[i] != ' ' && buf[i] != hasta; i++) {
    if ((buf[i] >= 'a' && buf[i] <= 'z') || (buf[i] >= '0' && buf[i] <= '9'))
      break;
  }
  if (buf[i] == hasta || (buf[i] == ' ' && i > 1))
    return 0;

  for (; buf[i] != ' ' && buf[i] != hasta; i++);
  if (buf[i] == hasta)
    return 0;

  for (int j = i + 1; buf[j] != hasta; j++) {
    int min = 0;
    for (; buf[j] != ' ' && buf[j] != hasta; j++) {
      if (buf[j] >= 'a' && buf[j] <= 'z') {
        min = 1;
        for (; buf[j] != ' ' && buf[j] != hasta; j++);
        break;
      }
    }
    if (!min) {
      buf[i] = ',';
      break;
    } else if (buf[j] == hasta)
      return 0;
    i = j;
  }

  return 1;
}


/* Recibe un string e intenta convertirlo en int
inicialmente convirtiendolo en long y chequeando que el resultado sea valido
en cuyo caso se guarda en un puntero y se retorna
sino se retorna NULL */
int* string_to_int(char* str) {
  // el puntero endp indica hasta donde se pudo convertir str
  char* endp = NULL;
  // se convierte en base 10
  long n = strtol(str, &endp, 10);
  /* si el puntero sigue al principio o no llego a /0
  (es decir que no se pudo convertir todo el string)
  o si el resultado esta fuera del rango representable por enteros
  no es valido y se retorna NULL */
  if (endp == str || *endp != '\0' || n < INT_MIN || n > INT_MAX)
    return NULL;
  int* m = malloc(sizeof(int));
  *m = (int) n;
  return m;
}

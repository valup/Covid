#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include "straux.h"

/* Recibe tres strings y devuelve uno nuevo con los dos primeros
separados por el tercero */
char* unir(char* str1, char* str2, char* sep) {
  /* Guarda espacio para el nuevo string */
  char* str = malloc(sizeof(char)
              * (strlen(str1) + strlen(str2) + strlen(sep) + 1));
  /* Copia el primer string */
  strcpy(str, str1);
  /* Concatena el separador */
  strcat(str, sep);
  /* Concatena el segundo string */
  strcat(str, str2);
  return str;
}

/* Recibe un string y devuelve un puntero al primer caracter no espacio */
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

/* Recibe un string y remueve los espacios de ambos extremos */
char* strim(char* str) {
  return strim_der(strim_izq(str));
}

/* Calcula la diferencia en dias entre dos fechas,
positiva si la primera fecha es posterior a la segunda
y negativa en caso contrario */
int dias(struct tm* f1, struct tm* f2) {
  /* mktime convierte las la informacion de struct tm* a valores time_t,
  difftime calcula la diferencia en segundos entre los time_t,
  y se divide el resultado por cantidad de segundos por dia*/
  return difftime(mktime(f1), mktime(f2))/(24.0*60*60);
}

/* Toma un string que asume como fecha en formato YYYY-MM-DD
e intenta crear una estructura de tiempo struct tm* con esa informacion */
struct tm* string_fecha(char* fecha) {
  int partes[3];
  int n = sscanf(fecha, "%d-%d-%d", partes, &partes[1], &partes[2]);
  /* Si no pudo escanear tres enteros entonces hay un error en el string
  y no se puede convertir */
  if (n != 3)
    return NULL;
  /* Como la pandemia se comenzo a esparcir en 2020
  el año no deberia ser anterior a eso */
  if (partes[0] < 2020) {
    printf("\nERROR: Fechas deben ser a partir de 2020.\n\n");
    return NULL;
  }
  /* En struct tm* los años se guardan como cantidad de años desde el 1900
  y los meses como cantidad de meses desde Enero, asi que se resta
  a los valores lo necesario para guardarlos en la estructura */
  partes[0] -= 1900;
  partes[1] -= 1;
  /* Se inicializa la estructura con todos los campos en 0 */
  struct tm* tm = malloc(sizeof(struct tm));
  memset(tm, 0, sizeof(struct tm));
  /* Se guardan los valores escaneados */
  tm->tm_year = partes[0];
  tm->tm_mon = partes[1];
  tm->tm_mday = partes[2];
  /* Se realiza mktime para confirmar la validez de la fecha.
  mktime retorna una estructura de tiempo time_t que si la fecha no es valida
  puede devolver -1, o tambien actualiza la estructura de argumento
  a una fecha valida */
  if (mktime(tm) == -1 || tm->tm_year != partes[0]
      || tm->tm_mon != partes[1] || tm->tm_mday != partes[2]) {
    free(tm);
    return NULL;
  }

  /* Se cacula la fecha actual */
  time_t ahora;
  time(&ahora);
  struct tm *hoy = localtime(&ahora);

  /* Si la fecha creada es posterior a la fecha actual es un error,
  porque no pueden haber registros de una fecha futura */
  if (dias(tm, hoy) > 0) {
    printf("\nERROR: Fecha futura.\n\n");
    return NULL;
  }

  return tm;
}

/* Toma un comando en buffer con argumentos separados por espacio,
pone una separador | en el primer espacio luego de la fecha
(asumiendo que esta al principio) y retorna un puntero
al primer caracter del comando luego de ese separador */
char* marcar_fecha(char* buf) {
  char* p = strchr(buf, ' ');
  /* Si no hay espacios o es el final del comando se retorna NULL sin marcar */
  if (!p || *(p + 1) == '\n' || *(p + 1) == '\0')
    return NULL;

  *p = '|';

  return p + 1;
}

/* Toma un comando en buffer con argumentos separados por espacio
(excepto la fecha que se separa primero con |),
e intenta poner un separador | en el espacio antes de cada notificacion
(asumiendo que son tres al final), retorna 1 si pudo o 0 sino */
int marcar_notifs(char* buf) {
  char* p;
  /* Repite por cada notificacion que deberia haber */
  for (int i = 0; i < 3; i++) {
    /* strrchr busca la ultima ocurrencia del caracter */
    p = strrchr(buf, ' ');
    /* Si no halla o es el primer caracter hay un error
    y no se puede separar */
    if (!p || p == buf)
      return 0;

    *p = '|';
  }
  return 1;
}

/* Toma un comando en buffer con departamento y localidad separados con espacio
y un caracter con el cual detener la lectura,
e intenta separar las partes por coma (asumiendo que estan al principio, que
ambas pueden tener espacios adentro y que se diferencian porque el departamento
tiene minusculas y la localidad no), retorna 1 si pudo o 0 sino */
int marcar_lugar(char* buf, char hasta) {
  int i = 0; /* Indice para recorrer el buffer */
  /* Se recorre la primera palabra a menos que empiece con numero
  o caracter menor (para casos como 9 de Julio)*/
  if (buf[i] > '9') {
    /* Se busca si la primera palabra es parte del departamento
    (tiene minusculas) */
    for (; buf[i] != ' ' && buf[i] != hasta; i++) {
      if (buf[i] >= 'a' && buf[i] <= 'z')
        break;
    }
  }
  /* Si ya se llego hasta el caracter limite o se leyeron varios caracteres
  sin hallar minusculas, faltan partes y no se puede separar */
  if (buf[i] == hasta || (buf[i] == ' ' && i > 1))
    return 0;
  /* Sino se recorre el string hasta espacio o limite */
  for (; buf[i] != ' ' && buf[i] != hasta; i++);
  /* Se vuelve a chequear si llego al limite */
  if (buf[i] == hasta)
    return 0;
  /* Se escanea la palabra luego del espacio
  a ver si es departamento o localidad */
  for (int j = i + 1; buf[j] != hasta; j++) {
    int min = 0; /* Flag para indicar que se hallo minuscula */
    /* Se recorre la palabra a ver si tiene minusculas */
    for (; buf[j] != ' ' && buf[j] != hasta; j++) {
      if (buf[j] >= 'a' && buf[j] <= 'z') {
        min = 1;
        for (; buf[j] != ' ' && buf[j] != hasta; j++);
        break;
      }
    }
    /* Si no tiene minusculas es localidad
    asi que se pone coma en el espacio que marca i */
    if (!min) {
      buf[i] = ',';
      break;
    /* Si tenia minusculas y se llego al limite falta localidad
    y no se puede separar */
    } else if (buf[j] == hasta)
      return 0;
    /* Sino se llego a espacio y la palabra es parte del departamento,
    asi que se guarda la posicion del nuevo espacio y se sigue buscando */
    i = j;
  }

  return 1;
}


/* Recibe un string e intenta convertirlo en int
inicialmente convirtiendolo en long y chequeando que el resultado sea valido
en cuyo caso se guarda en un puntero y se retorna
sino se retorna NULL */
int* string_to_int(char* str) {
  /* El puntero endp indica hasta donde se pudo convertir str */
  char* endp = NULL;
  /* Se convierte en base 10 */
  long n = strtol(str, &endp, 10);
  /* Si el puntero sigue al principio o no llego a /0
  (es decir que no se pudo convertir todo el string)
  o si el resultado esta fuera del rango representable por enteros
  no es valido y se retorna NULL */
  if (endp == str || *endp != '\0' || n < INT_MIN || n > INT_MAX)
    return NULL;
  int* m = malloc(sizeof(int));
  *m = (int) n;
  return m;
}

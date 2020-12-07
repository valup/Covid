#include "straux.h"

wchar_t* wgetline(wchar_t* buf, FILE* stream) {

  buf = malloc(sizeof(wchar_t) * 100);
  memset(buf, 0, sizeof(wchar_t) * 100);

  /* Empieza leyendo 99 caracteres */
  if (!fgetws(buf, 100, stream)) {
    free(buf);
    return NULL;
  }

  wchar_t* aux = NULL; /* Puntero auxiliar para seguir leyendo */

  /* Hasta que llegue al caracter de nueva linea sigue leyendo
  de a 49 caracteres en el puntero auxiliar y los concatena al buffer */
  while(wcslen(buf) > 0 && buf[wcslen(buf) - 1] != L'\n') {

    if (!aux) {
      aux = malloc(sizeof(wchar_t) * 50);
      memset(aux, 0, sizeof(wchar_t) * 50);
    }
    fgetws(aux, 50, stream);

    buf = realloc(buf, sizeof(wchar_t) * (wcslen(buf) + wcslen(aux) + 1));
    wcscat(buf, aux);
  }

  if (aux)
    free(aux);

  return buf;
}

wchar_t* unir(wchar_t* str1, wchar_t* str2, wchar_t* sep) {

  wchar_t* str = malloc(sizeof(wchar_t)
              * (wcslen(str1) + wcslen(str2) + wcslen(sep) + 1));

  wcscpy(str, str1);
  wcscat(str, sep);
  wcscat(str, str2);

  return str;
}

int dias(struct tm* f1, struct tm* f2) {
  /* mktime convierte las la informacion de struct tm* a valores time_t,
  difftime calcula la diferencia en segundos entre los time_t,
  y se divide el resultado por cantidad de segundos por dia*/
  return difftime(mktime(f1), mktime(f2))/(24.0*60*60);
}

int* string_to_int(wchar_t* str) {

  /* El puntero endp indica hasta donde se pudo convertir str */
  wchar_t* endp = NULL;

  /* Se convierte en base 10 */
  long n = wcstol(str, &endp, 10);

  /* Si el puntero sigue al principio o no llego a /0
  (es decir que no se pudo convertir todo el string)
  o si el resultado esta fuera del rango representable por enteros
  no es valido y se retorna NULL */
  if (endp == str || *endp != L'\0' || n < INT_MIN || n > INT_MAX)
    return NULL;

  int* m = malloc(sizeof(int));
  *m = (int) n;
  return m;
}

struct tm* string_fecha(wchar_t* fecha) {

  int partes[3];
  int n = swscanf(fecha, L"%d-%d-%d", partes, &partes[1], &partes[2]);
  if (n != 3)
    return NULL;

  /* Como la pandemia se comenzo a esparcir en 2020
  el año no deberia ser anterior a eso */
  if (partes[0] < 2020) {
    printf("\nERROR: Fechas deben ser a partir del 2020.\n\n");
    return NULL;
  }

  /* En struct tm* los años se guardan como cantidad de años desde el 1900
  y los meses como cantidad de meses desde Enero, asi que se resta
  a los valores lo necesario para guardarlos en la estructura */
  partes[0] -= 1900;
  partes[1] -= 1;

  struct tm* tm = malloc(sizeof(struct tm));
  memset(tm, 0, sizeof(struct tm));

  tm->tm_year = partes[0];
  tm->tm_mon = partes[1];
  tm->tm_mday = partes[2];

  /* Se realiza mktime para confirmar la validez de la fecha.
  mktime retorna una estructura de tiempo time_t que si la fecha no es valida
  puede devolver -1, o tambien actualiza la estructura de argumento
  a una fecha valida */
  if (mktime(tm) == -1 || tm->tm_year != partes[0]
      || tm->tm_mon != partes[1] || tm->tm_mday != partes[2]) {
    printf("\nERROR: Fecha invalida.\n\n");
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
    free(tm);
    return NULL;
  }

  return tm;
}

wchar_t* marcar_fecha(wchar_t* buf) {

  wchar_t* p = wcschr(buf, L' ');

  /* Si no hay espacios o es el final del comando se retorna NULL sin marcar */
  if (!p || *(p + 1) == L'\n' || *(p + 1) == L'\0')
    return NULL;

  *p = L'|';

  return p + 1;
}

int marcar_notifs(wchar_t* buf) {

  wchar_t* p; /* Puntero para wcsrchr */

  /* Repite por cada notificacion que deberia haber */
  for (int i = 0; i < 3; i++) {

    /* wcsrchr busca el ultimo espacio */
    p = wcsrchr(buf, L' ');

    if (!p || p == buf)
      return 0;

    *p = L'|';
  }

  return 1;
}

int marcar_lugar(wchar_t* buf, wchar_t hasta) {

  int i = 0;

  /* Se recorre la primera palabra a menos que empiece con numero
  o caracter menor (para casos como 9 de Julio) */
  if (buf[i] > L'9') {

    /* Se busca si la primera palabra es parte del departamento
    (tiene minusculas) */
    for (; buf[i] != L' ' && buf[i] != hasta
      && (buf[i] < L'a' || buf[i] > L'z'); i++);
  }

  /* Si ya se llego hasta el caracter limite o se leyeron varios caracteres
  sin hallar minusculas, faltan partes y no se puede separar */
  if (buf[i] == hasta || (buf[i] == L' ' && i > 1))
    return 0;

  for (; buf[i] != L' ' && buf[i] != hasta; i++);
  if (buf[i] == hasta)
    return 0;

  /* Se escanea la palabra luego del espacio
  a ver si es departamento o localidad */
  for (int j = i + 1; buf[j] != hasta; j++) {

    /* Se recorre la palabra a ver si tiene minusculas */
    for (; buf[j] != L' ' && buf[j] != hasta
      && (buf[j] < L'a' || buf[j] > L'z'); j++);

    /* Si tiene minusculas sigue recorriendo hasta limite o espacio */
    if (buf[j] >= L'a' && buf[j] <= L'z') {

      for (; buf[j] != L' ' && buf[j] != hasta; j++);

      if (buf[j] == hasta)
        return 0;

    } else {
      buf[i] = L',';
      return 1;
    }

    /* Si no retorna se llego a espacio y la palabra es parte del departamento,
    asi que se guarda la posicion del nuevo espacio y se sigue buscando */
    i = j;
  }

  return 1;
}

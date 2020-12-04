#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>

// void wgetline(wchar_t** buf, FILE* stream) {
//   *buf = malloc(sizeof(wchar_t) * 100);
//   fgetws(*buf, 100, stream);
//   while((*buf)[wcslen(*buf) - 1] != '\n') {
//     wchar_t* aux = malloc(sizeof(wchar_t) * 50);
//     fgetws(aux, 50, stream);
//     *buf = realloc(*buf, sizeof(wchar_t) * (wcslen(aux) + 100));
//     wcscat(*buf, aux);
//     free(aux);
//   }
// }

int main() {
  // para poder leer con wchar_t
  setlocale(LC_ALL, "spanish");

  char* str = "9 de Julio,LOGROÑO,3,5,0,8\n";
  wchar_t* wstr = malloc(sizeof(wchar_t) * (strlen(str) + 1));
  mbstowcs(wstr, str, strlen(str)+1);
  wchar_t* p;
  wchar_t* a = wcstok(wstr, L"\n", &p);

  printf("%ls\n", a);
  printf("%d\n", wcscoll(L"Ñ", L"O"));
  free(wstr);
  return 0;
}

//agregar_registro 2020-11-23 9 de Julio ANTONIO PINI 4 0 0

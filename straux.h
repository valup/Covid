#ifndef __STRAUX_H__
#define __STRAUX_H__

#include <wchar.h>

wchar_t* wgetline(wchar_t* buf, FILE* stream);

wchar_t* unir(wchar_t* str1, wchar_t* str2, wchar_t* sep);

int* string_to_int(wchar_t* str);

wchar_t* strim_izq(wchar_t* str);

wchar_t* strim_der(wchar_t* str);

wchar_t* strim(wchar_t* str);

int dias(struct tm* f1, struct tm* f2);

struct tm* string_fecha(wchar_t* fecha);

wchar_t* marcar_fecha(wchar_t* buf);

int marcar_notifs(wchar_t* buf);

int marcar_lugar(wchar_t* buf, wchar_t hasta);

#endif

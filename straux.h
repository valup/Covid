#ifndef __STRAUX_H__
#define __STRAUX_H__

char* unir(char* str1, char* str2, char* sep);

struct tm* string_fecha(char* fecha);

int* string_to_int(char* str);

char* strim_izq(char* str);

char* strim_der(char* str);

char* strim(char* str);

char* marcar_fecha(char* buf);

int marcar_notifs(char* buf);

int marcar_lugar(char* buf, char hasta);

#endif

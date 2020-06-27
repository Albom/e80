
#include "stdlib.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

void*  memset(void *mem, int c, size_t size);
void*  memcpy(void *dst, const void *src, size_t size);
int    memcmp(const void *buf1, const void *buf2, size_t count);

void   strcat(char *strDest, const char *strSource);
int    strcmp(const char *string1, const char *string2);
void   strcpy(char *strDest, const char *strSource);
char* strncpy(char *strDest, const char *strSource, size_t n);
int  strnicmp(const char* string1, const char* string2, size_t count);
size_t strlen(const char *string);
char*  strchr(const char *string, int c);
char* strrchr(const char *string, int c);
void    _itoa(int i, char *s);
void  reverse(char *s);
void     itoa(int i, char *s);
int     _atoi( char *s );

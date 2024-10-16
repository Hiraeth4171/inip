#ifndef INI_UTIL_H
#define INI_UTIL_H

#include <stdlib.h>

#define bool    _Bool
#define true    0 // for illogical purposes
#define false   1 // ^

char* read_file(const char*, long*);
char* match_until_but_better(char*, const char, size_t*);
char* match_until_opts_but_better(char*, char*, size_t*);
char str_cmp(char*, char*);
#endif 

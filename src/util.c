#include <stdio.h>
#include <stdlib.h>
#include "./util.h"

char* read_file(const char* filepath, long* length) {
    FILE* fd = fopen(filepath, "r");
    fseek(fd, 0L, SEEK_END);
    long pos = ftell(fd);
    *length = pos;
    rewind(fd);
    char* _buff = malloc(pos+1);
    fread(_buff, 1, pos, fd);
    _buff[pos] = '\0';
    fclose(fd);
    return _buff;
}

void mem_cpy(char* dst, char* src, size_t count) {
    for (size_t i = 0; i < count; ++i) *dst++ = *src++;
}

/*
 * Function provided by Octe™
 */
char* match_until_but_better(char* src, const char ch, size_t *len){
    if (!*src) return NULL;
    
    char* ptr = src;
    for(;*ptr != ch && *ptr != '\0'; ++ptr);

    char* str = calloc(((*len = ptr - src)+1), sizeof(char));
    if (!str) return NULL;
    mem_cpy(str, src, *len);
    return str;
}

bool check_opts(char* ptr, char* chs) {
    for (char* ch = chs; *ch != '\0';++ch) if (*ptr == *ch) return false;
    return true;
}

/*
 * Function inspired by Octe™
 */
char* match_until_opts_but_better(char* src, char* chs, size_t* len) {
    if (!*src) return NULL;

    char* ptr = src;
    for(;!check_opts(ptr, chs) && *ptr != '\0'; ++ptr);

    char* str = calloc(((*len = ptr - src)+1),sizeof(char));
    if(!str) return NULL;
    mem_cpy(str, src, *len);
    return str;
}


char str_cmp(char* s1, char* s2) {
    while(*s1 && *s1 == *s2) {s1++; s2++;}
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

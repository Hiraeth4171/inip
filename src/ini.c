#include <stdio.h>
#include <stdlib.h>
#include "include/inip/ini.h"
#include "util.h"

Token* tokenize(char* buff);

char* lex(char* buff) {
    char *ptr = buff, in_str_flag = 1;
    unsigned long i = 0;
    for (;*(ptr+1) != '\0';++ptr,++i){
        if (in_str_flag) while(*ptr == ' ') ptr++;
        while ((*ptr == ';' || *ptr == '#') && (ptr == buff || *(ptr - 1) == '\n')) {
            while(*ptr != '\n') ptr++;
            ptr++;
        }
        if (*ptr == '"') in_str_flag ^= 1;
        buff[i] = *ptr;
    }
    buff = realloc(buff, i+2); // MEMORY_CHECK MACRO
    buff[i] = '\n';
    buff[i+1] = '\0';
    return buff;
}

void print_tokens(Token* tokens) {
    for(Token* ptr = tokens; ptr->type != END; ++ptr) {
        if (ptr->type == SECTION) printf("\t[SECTION, data=%s]\n", ptr->data.data);
        else printf("\t[PAIR, key=%s | val=%s]\n", ptr->pair.key.data, ptr->pair.val.data);
    }
}

#define PAIRS_SIZE 16

// http://www.cse.yorku.ca/~oz/hash.html#sdbm
size_t hash_function(char* key, size_t size) {
    unsigned long hash = 0;
    int c;
    while((c = *key++))
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash % size;
}
// http://www.cse.yorku.ca/~oz/hash.html#sdbm

Hashtable* hashtable_init(size_t size) {
    Hashtable* section = malloc(sizeof(Hashtable));
    section->items = malloc(size * sizeof(Pair*));
    for (size_t i = 0; i < size; ++i) {
        section->items[i] = NULL;
    }
    section->length = size;
    return section;
}

void hashtable_add(Hashtable* section, Pair pair) {
    size_t i = hash_function(pair.key.data, section->length);
    while (section->items[i] != NULL) { 
        i++;
        if (i >= section->length) i = 0;
    }
    //printf("%lu", i);
    section->items[i] = malloc(sizeof(Pair));
    section->items[i]->key = pair.key;
    section->items[i]->val = pair.val;
}

Pair* hashtable_get(Hashtable* section, char* key) {
    size_t i = hash_function(key, section->length);
    while (section->items[i] != NULL) {
        if(str_cmp(section->items[i]->key.data, key) == 0) return section->items[i];
        i++;
        if (i >= section->length) i = 0;
    }
    return section->items[i];
}

Hashtable* ast_section(Token* tokens) {
    size_t _len = 0;
    for(Token* ptr = tokens; ptr->type != SECTION && ptr->type != END; ++ptr) _len++;
    Hashtable* result = hashtable_init(_len);
    for(Token* ptr = tokens; ptr->type != SECTION && ptr->type != END; ++ptr) {
        hashtable_add(result, ptr->pair);
    }
    return result;
}

void ini_add(INI* ini, Hashtable* section) {
    size_t i = hash_function(section->name.data, ini->length);
    while (ini->items[i] != NULL) { 
        i++;
        if (i >= ini->length) i = 0;
    }
    ini->items[i] = malloc(sizeof(Hashtable));
    ini->items[i] = section;
}

INI* ast(Token* tokens) {
    INI* result = malloc(sizeof(INI));
    result->length = 0;
    // TEMPORARY
    for(Token* ptr = tokens; ptr->type != END; ++ptr) if (ptr->type == SECTION) {
        result->length++;
    }
    result->items = calloc(result->length, sizeof(Hashtable*));
    Hashtable* temp = NULL;
    for(Token* ptr = tokens; ptr->type != END; ++ptr) {
        switch (ptr->type) {
            case SECTION:
                temp = ast_section(ptr+1);
                temp->name = ptr->data;
                ini_add(result, temp);
                ptr += temp->length;
                break;
            default:
                exit(-2);
                break;
        }
    }
    return result;
}

void free_tokens(Token** tokens) {
    for(Token* ptr = *tokens; ptr->type != END; ++ptr) {
        if (ptr->type == SECTION) free(ptr->data.data);
        else {
            free(ptr->pair.key.data);
            free(ptr->pair.val.data);
        }
    }
    free(*tokens);
}

INI* load_ini (const char* filepath) {
    long* len = malloc(sizeof(long));
    char* _buff = read_file(filepath, len);
    _buff = lex(_buff);
    Token* tokens = tokenize(_buff);
    free(_buff);
    //print_tokens(tokens);
    INI* ini = ast(tokens);
    free(len);
    return ini;
}

#define TOKENS_SIZE 16

void add_token(Token** tokens, size_t* tokens_cap, size_t* len, Token token) {
    if (*len > *tokens_cap) {
        *tokens_cap += TOKENS_SIZE;
        Token* tmp = realloc(tokens, *tokens_cap * sizeof(Token));
        if (tmp == NULL) exit(-1);
        *tokens = tmp;
    }
    switch (token.type) {
        case PAIR:
            (*tokens)[*len].type = token.type;
            (*tokens)[*len].pair.key.data = token.pair.key.data;
            (*tokens)[*len].pair.key.size = token.pair.key.size;
            (*tokens)[*len].pair.val.data = token.pair.val.data;
            (*tokens)[(*len)++].pair.val.size = token.pair.val.size;
            break;
        default:
            (*tokens)[*len].type = token.type;
            (*tokens)[*len].data.data = token.data.data;
            (*tokens)[(*len)++].data.size = token.data.size;
            break;
    }
}

Token* tokenize(char* buff) {
    size_t *tokens_cap = calloc(1, sizeof(size_t)), *length = calloc(1, sizeof(size_t)), *len = calloc(1, sizeof(size_t));
    *tokens_cap = TOKENS_SIZE;
    Token* tokens = malloc(*tokens_cap * sizeof(Token));
    char* str = NULL;
    for(char *ptr = buff; *ptr != '\0';) {
        switch (*ptr) {
            case '[': // section
                str = match_until_but_better(ptr+1, ']', len);
                add_token(&tokens, tokens_cap, length, (Token){.type = SECTION, .data = (String){.data = str, .size = *len}});
                ptr+=*len+3;
                break;
            default:  // pair
                str = match_until_opts_but_better(ptr, ":=", len);
                ptr+=*len+1;
                size_t key_len = *len;
                if (*ptr == '"') ptr++;
                char* val = match_until_opts_but_better(ptr, "\n\"", len);
                if (*(ptr+*len) == '"') ptr++;
                ptr+=*len+1;
                add_token(&tokens, tokens_cap, length, (Token){.type=PAIR, .pair=(Pair){.key=(String){.data=str,.size=key_len}, .val=(String){.data=val,.size=*len}}});
                break;
        }
    }
    add_token(&tokens, tokens_cap, length, (Token){.type=END, .data = (String){0,NULL}});
    tokens = realloc(tokens, ((*length)+1)*sizeof(Token));
    free(tokens_cap); free(length); free(len);
    return tokens;
}

Hashtable* get_section (INI* ini, char* obj_name) {
    size_t i = hash_function(obj_name, ini->length);
    while (ini->items[i] == NULL) {
        i++;
        if (i >= ini->length) return NULL;
    }
    return ini->items[i];
}

String* get_val (Hashtable* section, char* key) {
    return &hashtable_get(section, key)->val;
    return NULL;
}

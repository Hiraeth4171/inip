#include <stdio.h>

/*      typedefs      */
typedef struct Hashtable Hashtable;
typedef struct String String;
typedef struct INI INI;
typedef struct Token Token;
typedef struct Pair Pair;

struct String {
    size_t size;
    char* data;
};

struct Pair {
    String key;
    String val;
};

struct Hashtable {
    String name;
    size_t length;
    Pair** items;
}; 

struct INI {
    size_t length;
    Hashtable** items;
};

struct Token {
    enum {
        SECTION,
        PAIR,
        END
    } type;
    union {
        String data;
        Pair pair;
    };
};

INI* load_ini (const char* filepath);
Hashtable* get_section (INI* ini, char* obj_name);
String* get_val (Hashtable* ini, char* val_name);
void free_ini(INI* ini);

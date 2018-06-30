#include "bool_methods.h"

int bool_tostr(VM *vm) {
    int64_t val = POP(vm).value.ival;
    String_t* string;
    if (val == 0) {
        string = str_new_sized(snprintf(NULL, 0, "%s", "false"));
        sprintf(string->str, "%s", "false");
    } else {
        string = str_new_sized(snprintf(NULL, 0, "%s", "true"));
        sprintf(string->str, "%s", "true");
    }
    PUSH(vm, ((YASL_Object){STR, (int64_t)string}));
    return 0;
}
#include "YASL_Object.h"

#include <string.h>

#include "YASL_string.h"
#include "hashtable.h"
#include "float_methods.h"
#include "userdata.h"

char *float64_to_str(double d);

// Keep up to date with the YASL_Types
const char *YASL_TYPE_NAMES[] = {
    "undef",    // Y_UNDEF,
    "float64",  // Y_FLOAT64,
    "int64",    // Y_INT64,
    "bool",     // Y_BOOL,
    "str",      // Y_STR,
    "str",      // Y_STR_W,
    "list",     // Y_LIST,
    "list",     // Y_LIST_W,
    "table",    // Y_TABLE,
    "table",    // Y_TABLE_W,
    "fn",       // Y_FN,
    "mn",       // Y_BFN,
    "userptr",  // Y_USERPTR,
    "userdata", // Y_USERDATA,
    "userdata", // Y_USERDATA_W
};

struct CFunction_s *new_cfn(int (*value)(struct YASL_State *), int num_args) {
    struct CFunction_s *fn = malloc(sizeof(struct CFunction_s));
    fn->value = value;
    fn->num_args = num_args;
    fn->rc = rc_new();
    return fn;
}

void cfn_del_data(struct CFunction_s *cfn) {
}

void cfn_del_rc(struct CFunction_s *cfn) {
    rc_del(cfn->rc);
    free(cfn);
}

struct YASL_Object *YASL_Undef(void) {
    struct YASL_Object *undef = malloc(sizeof(struct YASL_Object));
    undef->type = Y_UNDEF;
    undef->value.ival = 0;
    return undef;
}
struct YASL_Object *YASL_Float(double value) {
    struct YASL_Object *num = malloc(sizeof(struct YASL_Object));
    num->type = Y_FLOAT64;
    num->value.dval = value;
    return num;
}

struct YASL_Object *YASL_Integer(int64_t value) {
    struct YASL_Object *integer = malloc(sizeof(struct YASL_Object));
    integer->type = Y_INT64;
    integer->value.ival = value;
    return integer;
}

struct YASL_Object *YASL_Boolean(int value) {
    struct YASL_Object *boolean = malloc(sizeof(struct YASL_Object));
    boolean->type = Y_BOOL;
    boolean->value.ival = value;
    return boolean;
}

struct YASL_Object *YASL_String(String_t *str) {
    struct YASL_Object *string = malloc(sizeof(struct YASL_Object));
    string->type = Y_STR;
    string->value.sval = str;
    return string;
}

struct YASL_Object *YASL_Table() {
    struct YASL_Object *table = malloc(sizeof(struct YASL_Object));
    table->type = Y_TABLE;
    table->value.mval = rcht_new();
    return table;
}

struct YASL_Object *YASL_UserPointer(void *userpointer) {
    struct YASL_Object *userptr = malloc(sizeof(struct YASL_Object));
    userptr->type = Y_USERPTR;
    userptr->value.pval = userpointer;
    return userptr;
}

struct YASL_Object *YASL_UserData(void *userdata, int tag) {
    struct YASL_Object *obj = malloc(sizeof(struct YASL_Object));
    obj->type = Y_USERDATA;
    obj->value.uval = ud_new(userdata, tag);
    return obj;
}

struct YASL_Object *YASL_Function(int64_t index) {
    struct YASL_Object *fn = malloc(sizeof(struct YASL_Object));
    fn->type = Y_FN;
    fn->value.ival = index;
    return fn;
}

struct YASL_Object *YASL_CFunction(int (*value)(struct YASL_State *), int num_args) {
    struct YASL_Object *fn = malloc(sizeof(struct YASL_Object));
    fn->type = Y_CFN;
    fn->value.pval = malloc(sizeof(struct CFunction_s));
    fn->value.cval->value = value;
    fn->value.cval->num_args = num_args;
    fn->value.cval->rc = rc_new();
    return fn;
}

int isfalsey(struct YASL_Object v) {
    // TODO: add NaN as falsey
    return (
            YASL_ISUNDEF(v) ||
            (YASL_ISBOOL(v) && YASL_GETBOOL(v) == 0) ||
            (YASL_ISSTR(v) && yasl_string_len(YASL_GETSTR(v)) == 0) ||
            (YASL_ISFLOAT(v) && YASL_GETFLOAT(v) != YASL_GETFLOAT(v))
    );
}

struct YASL_Object isequal(struct YASL_Object a, struct YASL_Object b) {
        if (YASL_ISUNDEF(a) && YASL_ISUNDEF(b)) {
            return TRUE_C;
        }
        switch(a.type) {
        case Y_BOOL:
            if (YASL_ISBOOL(b)) {
                if (YASL_GETBOOL(a) == YASL_GETBOOL(b)) {
                    return TRUE_C;
                } else {
                    return FALSE_C;
                }
            } else {
                return FALSE_C;
            }
        case Y_TABLE:
        case Y_TABLE_W:
            if (YASL_ISTBL(b)) {
                puts("Warning: comparison of hashes currently is not implemented.");
                return UNDEF_C;
            }
            return FALSE_C;
        case Y_LIST:
        case Y_LIST_W:
            if (YASL_ISLIST(b)) {
                puts("Warning: comparison of lists currently is not implemented.");
                return UNDEF_C;
            }
            return FALSE_C;
        case Y_STR:
        case Y_STR_W:
            if (YASL_ISSTR(b)) {
                if (yasl_string_len(YASL_GETSTR(a)) != yasl_string_len(YASL_GETSTR(b))) {
                    return FALSE_C;
                } else {
                    return memcmp(YASL_GETSTR(a)->str + YASL_GETSTR(a)->start,
                                  YASL_GETSTR(b)->str + YASL_GETSTR(b)->start,
                             yasl_string_len(YASL_GETSTR(a))) ? FALSE_C : TRUE_C;
                }
            }
            return FALSE_C;
        default:
            if (YASL_ISBOOL(b) || YASL_ISTBL(b)) {
                return FALSE_C;
            }
            int c;
            if (YASL_ISINT(a) && YASL_ISINT(b)) {
                c = YASL_GETINT(a) == YASL_GETINT(b);
            } else if (YASL_ISFLOAT(a) && YASL_ISINT(b)) {
                c = YASL_GETFLOAT(a) == (double)YASL_GETINT(b);
            } else if (YASL_ISINT(a) && YASL_ISFLOAT(b)) {
                c = (double)YASL_GETINT(a) == YASL_GETFLOAT(b);
            } else if (YASL_ISFLOAT(a) && YASL_ISFLOAT(b)) {
                c = YASL_GETFLOAT(a) == YASL_GETFLOAT(b);
            } else {
                // printf("== and != not supported for operands of types %x and %x.\n", a.type, b.type);
                return UNDEF_C;
            }
            return (struct YASL_Object) { .type = Y_BOOL, .value.ival = c};
        }
}

int print(struct YASL_Object v) {
    int64_t i;
    switch (v.type) {
        case Y_INT64:
            printf("%" PRId64 "", YASL_GETINT(v));
            //printf("int64: %" PRId64 "\n", v.value);
            break;
        case Y_FLOAT64: {
            char *tmp = float64_to_str(YASL_GETFLOAT(v));
            printf("%s", tmp);
            free(tmp);
            break;
        }
        case Y_BOOL:
            if (YASL_GETBOOL(v) == 0) printf("false");
            else printf("true");
            break;
        case Y_UNDEF:
            printf("undef");
            break;
        case Y_STR:
            for (i = 0; i < yasl_string_len(YASL_GETSTR(v)); i++) {
                printf("%c", YASL_GETSTR(v)->str[i + YASL_GETSTR(v)->start]);
            }
            break;
        case Y_TABLE:
            printf("<table %p>", (void*)YASL_GETTBL(v));
            break;
        case Y_LIST:
            //ls_print((List_t*)v.value);
            printf("<list %p>", (void*)YASL_GETLIST(v));
            break;
        case Y_FN:
            printf("<fn: %p>", (void*)YASL_GETFN(v));
            break;
        case Y_CFN:
            printf("<fn: %p>", (void*)(*(char**)&v.value.cval->value));
            break;
        case Y_USERPTR:
            printf("0x%p", YASL_GETUSERPTR(v));
            break;
        default:
            printf("Error, unknown type: %x", v.type);
            return -1;
    }
    return 0;
}

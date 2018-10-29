#pragma once

#include "interpreter/refcount/refcount.h"

struct Hash_s;

typedef struct UserData_s {
    int tag;
    //struct Hash_s *mt;
    void *data;
    RefCount *rc;
} UserData_t;


UserData_t *ud_new(void *data, int tag);
void ud_del_data(UserData_t *ud);
void ud_del_rc(UserData_t *ud);
void ud_del(UserData_t *ud);
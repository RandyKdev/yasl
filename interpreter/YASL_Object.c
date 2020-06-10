#include "YASL_Object.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "debug.h"
#include "data-structures/YASL_Table.h"
#include "interpreter/userdata.h"
#include "interpreter/refcount.h"

char *float64_to_str(yasl_float d);

// Keep up to date with the YASL_Types
const char *YASL_TYPE_NAMES[] = {
	"undef",    // Y_UNDEF,
	"float",    // Y_FLOAT,
	"int",      // Y_INT,
	"bool",     // Y_BOOL,
	"str",      // Y_STR,
	"str",      // Y_STR_W,
	"list",     // Y_LIST,
	"list",     // Y_LIST_W,
	"table",    // Y_TABLE,
	"table",    // Y_TABLE_W,
	"fn",       // Y_FN,
	"fn",	    // Y_CLOSURE,
	"fn",       // Y_CFN,
	"userptr",  // Y_USERPTR,
	"userdata", // Y_USERDATA,
	"userdata", // Y_USERDATA_W
};

struct CFunction *new_cfn(int (*value)(struct YASL_State *), int num_args) {
	struct CFunction *fn = (struct CFunction *) malloc(sizeof(struct CFunction));
	fn->value = value;
	fn->num_args = num_args;
	fn->rc = rc_new();
	return fn;
}

void cfn_del_data(struct CFunction *cfn) {
	(void) cfn;
}

void cfn_del_rc(struct CFunction *cfn) {
	rc_del(cfn->rc);
	free(cfn);
}

struct YASL_Object *YASL_Table(void) {
	struct YASL_Object *table = (struct YASL_Object *) malloc(sizeof(struct YASL_Object));
	table->type = Y_TABLE;
	table->value.uval = rcht_new();
	return table;
}

int yasl_object_cmp(struct YASL_Object a, struct YASL_Object b) {
	if (obj_isstr(&a) && obj_isstr(&b)) {
		return YASL_String_cmp(obj_getstr(&a), obj_getstr(&b));
	} else if (obj_isnum(&a) && obj_isnum(&b)) {
		yasl_float aVal, bVal;
		if(obj_isint(&a)) {
			aVal = (yasl_float)obj_getint(&a);
		} else {
			aVal = obj_getfloat(&a);
		}
		if(obj_isint(&b)) {
			bVal = (yasl_float)obj_getint(&b);
		} else {
			bVal = obj_getfloat(&b);
		}

		if (aVal < bVal) return -1;
		if (aVal > bVal) return 1;
		return 0;
	} else {
		printf("Cannot apply object compare to types %s and %s.\n", YASL_TYPE_NAMES[a.type], YASL_TYPE_NAMES[b.type]);
		exit(-1);
	}
	return 0;
}

bool ishashable(const struct YASL_Object *const v) {
	return (
		obj_isundef(v) ||
		obj_isbool(v) ||
		obj_isfloat(v) ||
		obj_isint(v) ||
		obj_isstr(v) ||
		obj_isuserptr(v)
		);
}

bool isfalsey(const struct YASL_Object *const v) {
	/*
	 * Falsey values are:
	 * 	undef
	 * 	false
	 * 	''
	 * 	NaN
	 */
	return (
		obj_isundef(v) ||
		(obj_isbool(v) && obj_getbool(v) == 0) ||
		(obj_isstr(v) && YASL_String_len(obj_getstr(v)) == 0) ||
		(obj_isfloat(v) && obj_getfloat(v) != obj_getfloat(v))
	);
}

bool isequal(const struct YASL_Object *const a, const struct YASL_Object *const b) {
	if (obj_isbool(a) && obj_isbool(b)) {
		return obj_getbool(a) == obj_getbool(b);
	}

	if (obj_isint(a) && obj_isint(b)) {
		return obj_getint(a) == obj_getint(b);
	}

	if (obj_isnum(a) && obj_isnum(b)) {
		return obj_getnum(a) == obj_getnum(b);
	}

	if (obj_isstr(a) && obj_isstr(b)) {
		struct YASL_String *left = obj_getstr(a);
		struct YASL_String *right = obj_getstr(b);
		return left == right || YASL_String_cmp(left, right) == 0;
	}

	if (obj_isuserptr(a) && obj_isuserptr(b)) {
		return YASL_GETUSERPTR(*a) == YASL_GETUSERPTR(*b);
	}

	return false;
}

int print(struct YASL_Object v) {
	int64_t i;
	switch (v.type) {
	case Y_END:
		YASL_ASSERT(false, "should not have called print with Y_END");
		break;
	case Y_INT:
		printf("%" PRId64 "", obj_getint(&v));
		break;
	case Y_FLOAT: {
		char *tmp = float64_to_str(obj_getfloat(&v));
		printf("%s", tmp);
		free(tmp);
		break;
	}
	case Y_BOOL:
		if (obj_getbool(&v) == 0) printf("false");
		else printf("true");
		break;
	case Y_UNDEF:
		printf("undef");
		break;
	case Y_STR:
	case Y_STR_W:
		for (i = 0; i < (yasl_int) YASL_String_len(obj_getstr(&v)); i++) {
			printf("%c", obj_getstr(&v)->str[i + obj_getstr(&v)->start]);
		}
		break;
	case Y_TABLE:
	case Y_TABLE_W:
		printf("<table>");
		break;
	case Y_LIST:
	case Y_LIST_W:
		printf("<list>");
		break;
	case Y_FN:
	case Y_CLOSURE:
	case Y_CFN:
		printf("<fn>");
		break;
	case Y_USERPTR:
		printf("0x%p", YASL_GETUSERPTR(v));
		break;
	case Y_USERDATA:
	case Y_USERDATA_W:
		printf("<userdata>");
		break;
	}
	return 0;
}

extern inline bool obj_isundef(const struct YASL_Object *const v);
extern inline bool obj_isfloat(const struct YASL_Object *const v);
extern inline bool obj_isint(const struct YASL_Object *const v);
extern inline bool obj_isnum(const struct YASL_Object *const v);
extern inline bool obj_isbool(const struct YASL_Object *const v);
extern inline bool obj_isstr(const struct YASL_Object *const v);
extern inline bool obj_islist(const struct YASL_Object *const v);
extern inline bool obj_istable(const struct YASL_Object *const v);
extern inline bool obj_isuserdata(const struct YASL_Object *const v);
extern inline bool obj_isuserptr(const struct YASL_Object *const v);
extern inline bool obj_isfn(const struct YASL_Object *const v);
extern inline bool obj_isclosure(const struct YASL_Object *const v);
extern inline bool obj_iscfn(const struct YASL_Object *const v);

extern inline bool obj_getbool(const struct YASL_Object *const v);
extern inline yasl_float obj_getfloat(const struct YASL_Object *const v);
extern inline yasl_int obj_getint(const struct YASL_Object *const v);
extern inline yasl_float obj_getnum(const struct YASL_Object *const v);
extern inline struct YASL_String *obj_getstr(const struct YASL_Object *const v);

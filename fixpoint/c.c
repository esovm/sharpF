/* c.sf */
#define MODULE module_c
#define LOAD() module_7(); module_6(); module_5(); module_4(); module_3(); module_2(); module_1(); module_0(); 
extern void module_0(void); /* 0.sf */
extern void module_1(void); /* 1.sf */
extern void module_2(void); /* 2.sf */
extern void module_3(void); /* 3.sf */
extern void module_4(void); /* 4.sf */
extern void module_5(void); /* 5.sf */
extern void module_6(void); /* 6.sf */
extern void module_7(void); /* 7.sf */

/* standard includes */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
/* extra includes */
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

/* standard definitions */
#define REGS_SIZE 5000

typedef ptrdiff_t obj;        /* pointers are this size, lower bit zero */
typedef ptrdiff_t cxoint_t;   /* same thing, used as integer */
typedef struct {              /* type descriptor */
  const char *tname;          /* name (debug) */
  void (*free)(void*);        /* deallocator */
} cxtype_t;

#define notobjptr(o)          (((char*)(o) - (char*)cxg_heap) & cxg_hmask)
#define isobjptr(o)           (!notobjptr(o))
#define notaptr(o)            ((o) & 1)
#define isaptr(o)             (!notaptr(o))

#define obj_from_obj(o)       (o)
#define obj_from_objptr(p)    ((obj)(p))
#define obj_from_size(n)      (((cxoint_t)(n) << 1) | 1)

#define objptr_from_objptr(p) (p)
#define objptr_from_obj(o)    ((obj*)(o))

#define size_from_obj(o)      ((int)((o) >> 1))

#define obj_from_case(n)      obj_from_objptr(cases+(n))
#define case_from_obj(o)      (objptr_from_obj(o)-cases)
#define obj_from_ktrap()      obj_from_size(0x5D56F806)
#define obj_from_void(v)      ((void)(v), obj_from_size(0x6F56DF77))

#define bool_from_obj(o)      (o)
#define bool_from_bool(b)     (b)
#define bool_from_size(s)     (s)

#define void_from_void(v)     (void)(v)
#define void_from_obj(o)      (void)(o)

#define rreserve(m)           if (r > cxg_regs + REGS_SIZE - 2*(m)) r = cxm_rgc(r, r+(m))
#define hpushptr(p, pt, l)    (hreserve(2, l), *--hp = (obj)(p), *--hp = (obj)(pt), (obj)(hp+1))   
#define hbsz(s)               ((s) + 1) /* 1 extra word to store block size */
#define hreserve(n, l)        ((hp < cxg_heap + (n)) ? hp = cxm_hgc(r, r+(l), hp, n) : hp)
#define hendblk(n)            (*--hp = obj_from_size(n), (obj)(hp+1))
#define hblklen(p)            size_from_obj(((obj*)(p))[-1])
#define hblkref(p, i)         (((obj*)(p))[i])

typedef obj (*cxhost_t)(obj);
typedef struct cxroot_tag {
  int globc; obj **globv;
  struct cxroot_tag *next;
} cxroot_t;

extern obj *cxg_heap;
extern obj *cxg_hp;
extern cxoint_t cxg_hmask;
extern cxroot_t *cxg_rootp;
extern obj *cxm_rgc(obj *regs, obj *regp);
extern obj *cxm_hgc(obj *regs, obj *regp, obj *hp, size_t needs);
extern obj cxg_regs[REGS_SIZE];
extern void *cxm_cknull(void *p, char *msg);
#ifndef NDEBUG
extern int cxg_rc;
#endif

/* extra definitions */
/* basic object representation */
#define isimm(o, t) (((o) & 0xff) == (((t) << 1) | 1))
#ifdef NDEBUG
  #define getimmu(o, t) (int)(((o) >> 8) & 0xffffff)
  #define getimms(o, t) (int)(((((o) >> 8) & 0xffffff) ^ 0x800000) - 0x800000)
#else
  extern int getimmu(obj o, int t);
  extern int getimms(obj o, int t);
#endif
#define mkimm(o, t) ((((o) & 0xffffff) << 8) | ((t) << 1) | 1)
#ifdef NDEBUG
   static int isnative(obj o, cxtype_t *tp) 
     { return isobjptr(o) && objptr_from_obj(o)[-1] == (obj)tp;  }
   #define getnative(o, t) ((void*)(*objptr_from_obj(o)))
#else
  extern int isnative(obj o, cxtype_t *tp);
  extern void *getnative(obj o, cxtype_t *tp);
#endif
extern int istagged(obj o, int t);
#ifdef NDEBUG
  #define cktagged(o, t) (o)
  #define taggedlen(o, t) (hblklen(o)-1) 
  #define taggedref(o, t, i) (&hblkref(o, (i)+1))
#else
  extern obj cktagged(obj o, int t);
  extern int taggedlen(obj o, int t);
  extern obj* taggedref(obj o, int t, int i); 
#endif
/* booleans */
#define TRUE_ITAG 0
typedef int bool_t;
#define is_bool_obj(o) (!((o) & ~(obj)1))
#define is_bool_bool(b) ((void)(b), 1)
#define void_from_bool(b) (void)(b)
#define obj_from_bool(b) ((b) ? mkimm(0, TRUE_ITAG) : 0)
/* fixnums */
#define FIXNUM_ITAG 1
typedef int fixnum_t;
#define is_fixnum_obj(o) (isimm(o, FIXNUM_ITAG))
#define is_fixnum_fixnum(i) ((void)(i), 1)
#define fixnum_from_obj(o) (getimms(o, FIXNUM_ITAG))
#define fixnum_from_fixnum(i) (i)
#define void_from_fixnum(i) (void)(i)
#define obj_from_fixnum(i) mkimm(i, FIXNUM_ITAG)
#define FIXNUM_MIN -8388608
#define FIXNUM_MAX 8388607
/* flonums */
extern cxtype_t *FLONUM_NTAG;
typedef double flonum_t;
#define is_flonum_obj(o) (isnative(o, FLONUM_NTAG))
#define is_flonum_flonum(f) ((void)(f), 1)
#define flonum_from_obj(o) (*(flonum_t*)getnative(o, FLONUM_NTAG))
#define flonum_from_flonum(l, f) (f)
#define void_from_flonum(l, f) (void)(f)
#define obj_from_flonum(l, f) hpushptr(dupflonum(f), FLONUM_NTAG, l)
extern flonum_t *dupflonum(flonum_t f);
/* characters */
#define CHAR_ITAG 2
typedef int char_t;
#define is_char_obj(o) (isimm(o, CHAR_ITAG))
#define is_char_char(i) ((i), 1)
#define char_from_obj(o) (getimms(o, CHAR_ITAG))
#define char_from_char(i) (i)
#define void_from_char(i) (void)(i)
#define obj_from_char(i) mkimm(i, CHAR_ITAG)
/* strings */
extern cxtype_t *STRING_NTAG;
#define isstring(o) (isnative(o, STRING_NTAG))
#define stringdata(o) ((int*)getnative(o, STRING_NTAG))
#define stringlen(o) (*stringdata(o))
#define stringchars(o) ((char*)(stringdata(o)+1))
#define hpushstr(l, s) hpushptr(s, STRING_NTAG, l)
#ifdef NDEBUG
  #define stringref(o, i) (stringchars(o)+(i))
#else
  extern char* stringref(obj o, int i);
#endif
extern int *newstring(char *s);
extern int *allocstring(int n, int c);
extern int *substring(int *d, int from, int to);
extern int *stringcat(int *d0, int *d1);
extern int *dupstring(int *d);
extern void stringfill(int *d, int c);
extern int strcmp_ci(char *s1, char*s2);
/* vectors */
#define VECTOR_BTAG 1
#define isvector(o) istagged(o, VECTOR_BTAG)
#define vectorref(v, i) *taggedref(v, VECTOR_BTAG, i)
#define vectorlen(v) taggedlen(v, VECTOR_BTAG)
/* boxes */
#define BOX_BTAG 2
#define isbox(o) istagged(o, BOX_BTAG)
#define boxref(o) *taggedref(o, BOX_BTAG, 0)
/* null */
#define NULL_ITAG 2
#define mknull() mkimm(0, NULL_ITAG)
#define isnull(o) ((o) == mkimm(0, NULL_ITAG))
/* pairs and lists */
#define PAIR_BTAG 3
#define ispair(o) istagged(o, PAIR_BTAG)
#define car(o) *taggedref(o, PAIR_BTAG, 0)
#define cdr(o) *taggedref(o, PAIR_BTAG, 1)
/* symbols */
#define SYMBOL_ITAG 3
#define issymbol(o) (isimm(o, SYMBOL_ITAG))
#define mksymbol(i) mkimm(i, SYMBOL_ITAG)
#define getsymbol(o) getimmu(o, SYMBOL_ITAG)
extern char *symbolname(int sym);
extern int internsym(char *name);
/* eof */
#define EOF_ITAG 127
#define mkeof() mkimm(-1, EOF_ITAG)
#define iseof(o) ((o) == mkimm(-1, EOF_ITAG))
/* input ports */
extern cxtype_t *IPORT_NTAG;
#define isiport(o) (isnative(o, IPORT_NTAG))
#define iportdata(o) ((FILE*)getnative(o, IPORT_NTAG))
#define mkiport(l, fp) hpushptr(fp, IPORT_NTAG, l)
/* output ports */
extern cxtype_t *OPORT_NTAG;
#define isoport(o) (isnative(o, OPORT_NTAG))
#define oportdata(o) ((FILE*)getnative(o, OPORT_NTAG))
#define mkoport(l, fp) hpushptr(fp, OPORT_NTAG, l)
extern int iseqv(obj x, obj y);
extern obj ismemv(obj x, obj l);
extern obj isassv(obj x, obj l);
extern int isequal(obj x, obj y);
extern obj ismember(obj x, obj l);
extern obj isassoc(obj x, obj l);

/* cx globals */
extern obj cx__2Acurrent_2Derror_2Dport_2A; /* *current-error-port* */
extern obj cx__2Acurrent_2Doutput_2Dport_2A; /* *current-output-port* */
extern obj cx_analyze_2Dglobals; /* analyze-globals */
extern obj cx_beta_2Dsubstitute; /* beta-substitute */
extern obj cx_box_2Dexp_3F; /* box-exp? */
extern obj cx_boxref_2Dexp_3F; /* boxref-exp? */
extern obj cx_boxset_2Dexp_3F; /* boxset-exp? */
extern obj cx_c_2Derror_2A; /* c-error* */
extern obj cx_c_2Dmangle; /* c-mangle */
extern obj cx_code_2Dgenerate; /* code-generate */
extern obj cx_code_2Dhost_2Depilogue; /* code-host-epilogue */
extern obj cx_code_2Dhost_2Dprologue; /* code-host-prologue */
extern obj cx_code_2Dmain; /* code-main */
extern obj cx_code_2Dmodule; /* code-module */
extern obj cx_code_2Druntime; /* code-runtime */
extern obj cx_code_2Dstandard_2Ddefinitions; /* code-standard-definitions */
extern obj cx_code_2Dstandard_2Dincludes; /* code-standard-includes */
extern obj cx_constant_2Dfold; /* constant-fold */
extern obj cx_cps_2Dconvert; /* cps-convert */
extern obj cx_display_2Dtext; /* display-text */
extern obj cx_exp_2D_3Efree_2Dvars; /* exp->free-vars */
extern obj cx_exp_2Dvinfo; /* exp-vinfo */
extern obj cx_expand_2Dtop_2Dlevel_2Dforms_21; /* expand-top-level-forms! */
extern obj cx_file_2Dexpand_2Dtop_2Dlevel_2Dforms_21; /* file-expand-top-level-forms! */
extern obj cx_fix_2Dletrecs; /* fix-letrecs */
extern obj cx_fprintf_2A; /* fprintf* */
extern obj cx_id_2D_3Euname; /* id->uname */
extern obj cx_labelapp_2Dexp_3F; /* labelapp-exp? */
extern obj cx_lambda_2Dlift; /* lambda-lift */
extern obj cx_letrec_2A_2Dexp_2D_3Ebody; /* letrec*-exp->body */
extern obj cx_letrec_2A_2Dexp_2D_3Erands; /* letrec*-exp->rands */
extern obj cx_letrec_2A_2Dexp_3F; /* letrec*-exp? */
extern obj cx_letrec_2Dexp_3F; /* letrec-exp? */
extern obj cx_null_2Dmstore; /* null-mstore */
extern obj cx_parse_2Dprogram; /* parse-program */
extern obj cx_path_2Dstrip_2Ddirectory; /* path-strip-directory */
extern obj cx_path_2Dstrip_2Dextension; /* path-strip-extension */
extern obj cx_read_2F1; /* read/1 */
extern obj cx_remove_2Dassignments; /* remove-assignments */
extern obj cx_reset; /* reset */
extern obj cx_reset_2Dtimestamps; /* reset-timestamps */
extern obj cx_sexp_2Dmatch_3F; /* sexp-match? */
extern obj cx_stack_2Dfunctions_2Dcode_2Dgenerate; /* stack-functions-code-generate */
extern obj cx_unbox_2Dvalues; /* unbox-values */
extern obj cx_var_2Dreferenced_2Din_2Dexp_3F; /* var-referenced-in-exp? */
extern obj cx_var_2Duses_2Din_2Dexp; /* var-uses-in-exp */
extern obj cx_with_2Doutput_2Dto_2Dfile; /* with-output-to-file */
extern obj cx_write_2F3; /* write/3 */
obj cx__2Ashowlog_2A; /* *showlog* */
obj cx__2Asilent_2A; /* *silent* */
obj cx_clear_2Dmemoizations; /* clear-memoizations */
obj cx_compile_2Dfile; /* compile-file */
obj cx_display_2Dmodule; /* display-module */
obj cx_main; /* main */
obj cx_maybe_2Dlog; /* maybe-log */
obj cx_maybe_2Dpp_2Dsource; /* maybe-pp-source */
obj cx_maybe_2Dpp_2Dtext; /* maybe-pp-text */
obj cx_parse_2Dfile; /* parse-file */
obj cx_source; /* source */
static obj cx__231003; /* constant #1003 */
static obj cx__231022; /* constant #1022 */
static obj cx__231039; /* constant #1039 */
static obj cx__231044; /* constant #1044 */
static obj cx__231052; /* constant #1052 */
static obj cx__231076; /* constant #1076 */
static obj cx__231084; /* constant #1084 */
static obj cx__231089; /* constant #1089 */
static obj cx__231097; /* constant #1097 */
static obj cx__231105; /* constant #1105 */
static obj cx__231118; /* constant #1118 */
static obj cx__231121; /* constant #1121 */
static obj cx__231169; /* constant #1169 */
static obj cx__231182; /* constant #1182 */
static obj cx__231189; /* constant #1189 */
static obj cx__231196; /* constant #1196 */
static obj cx__231269; /* constant #1269 */
static obj cx__231278; /* constant #1278 */
static obj cx__231282; /* constant #1282 */
static obj cx__231285; /* constant #1285 */
static obj cx__231286; /* constant #1286 */
static obj cx__231290; /* constant #1290 */
static obj cx__231291; /* constant #1291 */
static obj cx__231295; /* constant #1295 */
static obj cx__231296; /* constant #1296 */
static obj cx__231300; /* constant #1300 */
static obj cx__231301; /* constant #1301 */
static obj cx__231305; /* constant #1305 */
static obj cx__231306; /* constant #1306 */
static obj cx__231310; /* constant #1310 */
static obj cx__231311; /* constant #1311 */
static obj cx__231315; /* constant #1315 */
static obj cx__231316; /* constant #1316 */
static obj cx__231320; /* constant #1320 */
static obj cx__231322; /* constant #1322 */
static obj cx__231324; /* constant #1324 */
static obj cx__231328; /* constant #1328 */
static obj cx__231329; /* constant #1329 */
static obj cx__231333; /* constant #1333 */
static obj cx__231334; /* constant #1334 */
static obj cx__231338; /* constant #1338 */
static obj cx__231339; /* constant #1339 */
static obj cx__231360; /* constant #1360 */
static obj cx__231362; /* constant #1362 */
static obj cx__231367; /* constant #1367 */
static obj cx__231370; /* constant #1370 */
static obj cx__231371; /* constant #1371 */
static obj cx__231372; /* constant #1372 */
static obj cx__231383; /* constant #1383 */
static obj cx__231386; /* constant #1386 */
static obj cx__231397; /* constant #1397 */
static obj cx__2314; /* constant #14 */
static obj cx__231401; /* constant #1401 */
static obj cx__231402; /* constant #1402 */
static obj cx__231406; /* constant #1406 */
static obj cx__231472; /* constant #1472 */
static obj cx__231481; /* constant #1481 */
static obj cx__231485; /* constant #1485 */
static obj cx__231491; /* constant #1491 */
static obj cx__231497; /* constant #1497 */
static obj cx__2315; /* constant #15 */
static obj cx__231503; /* constant #1503 */
static obj cx__231509; /* constant #1509 */
static obj cx__23247; /* constant #247 */
static obj cx__23626; /* constant #626 */
static obj cx__23629; /* constant #629 */
static obj cx__23631; /* constant #631 */
static obj cx__23635; /* constant #635 */
static obj cx__23915; /* constant #915 */
static obj cx__23959; /* constant #959 */
static obj cx__23965; /* constant #965 */
static obj cx__23973; /* constant #973 */
static obj cx__23978; /* constant #978 */
static obj cx__23983; /* constant #983 */
static obj cx__23991; /* constant #991 */
static obj cx__23996; /* constant #996 */
static obj cx_begin_2Dexp_3F_2355; /* constant begin-exp?#55 */
static obj cx_curry_2Dexp_3F_23513; /* constant curry-exp?#513 */

/* helper functions */
/* let-exp?#85 */
static obj cxs_let_2Dexp_3F_2385(obj v87_exp)
{ 
  if (bool_from_obj((isvector((v87_exp))) ? (((vectorlen((v87_exp))) == (3)) ? obj_from_bool((vectorref((v87_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v91_rator = (vectorref((v87_exp), (1)));
  if (bool_from_obj((isvector((v91_rator))) ? (((vectorlen((v91_rator))) == (3)) ? obj_from_bool((vectorref((v91_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v1689_tmp;
    obj v1688_tmp;
    { /* length */
    int n; obj l = (vectorref((v87_exp), (2)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v1689_tmp = obj_from_fixnum(n); };
    { /* length */
    int n; obj l = (vectorref((v91_rator), (1)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v1688_tmp = obj_from_fixnum(n); };
    return obj_from_bool(fixnum_from_obj(v1688_tmp) == fixnum_from_obj(v1689_tmp));
  }
  } else {
    return obj_from_bool(0);
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp->body#122 */
static obj cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_23122(obj v124_exp)
{ 
    return ((isnull((vectorref((v124_exp), (2))))) ? (vectorref((vectorref((v124_exp), (1))), (2))) : (car((vectorref((v124_exp), (2))))));
}

/* null-let-exp?#148 */
static obj cxs_null_2Dlet_2Dexp_3F_23148(obj v150_exp)
{ 
  if (bool_from_obj((isvector((v150_exp))) ? (((vectorlen((v150_exp))) == (3)) ? obj_from_bool((vectorref((v150_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if ((isnull((vectorref((v150_exp), (2)))))) {
  { /* let */
    obj v154_rator = (vectorref((v150_exp), (1)));
    return (bool_from_obj((isvector((v154_rator))) ? (((vectorlen((v154_rator))) == (3)) ? obj_from_bool((vectorref((v154_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool(isnull((vectorref((v154_rator), (1))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-lambda-exp?#194 */
static obj cxs_identity_2Dlambda_2Dexp_3F_23194(obj v196_exp)
{ 
  if (bool_from_obj((isvector((v196_exp))) ? (((vectorlen((v196_exp))) == (3)) ? obj_from_bool((vectorref((v196_exp), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v203_body = (vectorref((v196_exp), (2)));
    obj v204_ids = (vectorref((v196_exp), (1)));
    return ((ispair((v204_ids))) ? ((isnull((cdr((v204_ids))))) ? (bool_from_obj((isvector((v203_body))) ? (((vectorlen((v203_body))) == (2)) ? obj_from_bool((vectorref((v203_body), (0))) == (mksymbol(internsym("var-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool((car((v204_ids))) == (vectorref((v203_body), (1)))) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-let-exp?#181 */
static obj cxs_identity_2Dlet_2Dexp_3F_23181(obj v183_exp)
{ 
  if (bool_from_obj((isvector((v183_exp))) ? (((vectorlen((v183_exp))) == (3)) ? obj_from_bool((vectorref((v183_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if (bool_from_obj(cxs_identity_2Dlambda_2Dexp_3F_23194((vectorref((v183_exp), (1)))))) {
  { /* let */
    obj v187_rands = (vectorref((v183_exp), (2)));
    return ((ispair((v187_rands))) ? obj_from_bool(isnull((cdr((v187_rands))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp?#145 */
static obj cxs_degenerate_2Dlet_2Dexp_3F_23145(obj v147_exp)
{ 
  { /* let */
    obj v180_x = (cxs_null_2Dlet_2Dexp_3F_23148((v147_exp)));
    return (bool_from_obj(v180_x) ? (v180_x) : (cxs_identity_2Dlet_2Dexp_3F_23181((v147_exp))));
  }
}

/* void-exp?#912 */
static obj cxs_void_2Dexp_3F_23912(obj v914_exp)
{ 
    return (bool_from_obj((isvector((v914_exp))) ? (((vectorlen((v914_exp))) == (4)) ? obj_from_bool((vectorref((v914_exp), (0))) == (mksymbol(internsym("primapp-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool(isequal((vectorref((v914_exp), (2))), (cx__23915))) : obj_from_bool(0));
}

/* gc roots */
static obj *globv[] = {
  &cx__2Ashowlog_2A,
  &cx__2Asilent_2A,
  &cx__231003,
  &cx__231022,
  &cx__231039,
  &cx__231044,
  &cx__231052,
  &cx__231076,
  &cx__231084,
  &cx__231089,
  &cx__231097,
  &cx__231105,
  &cx__231118,
  &cx__231121,
  &cx__231169,
  &cx__231182,
  &cx__231189,
  &cx__231196,
  &cx__231269,
  &cx__231278,
  &cx__231282,
  &cx__231285,
  &cx__231286,
  &cx__231290,
  &cx__231291,
  &cx__231295,
  &cx__231296,
  &cx__231300,
  &cx__231301,
  &cx__231305,
  &cx__231306,
  &cx__231310,
  &cx__231311,
  &cx__231315,
  &cx__231316,
  &cx__231320,
  &cx__231322,
  &cx__231324,
  &cx__231328,
  &cx__231329,
  &cx__231333,
  &cx__231334,
  &cx__231338,
  &cx__231339,
  &cx__231360,
  &cx__231362,
  &cx__231367,
  &cx__231370,
  &cx__231371,
  &cx__231372,
  &cx__231383,
  &cx__231386,
  &cx__231397,
  &cx__2314,
  &cx__231401,
  &cx__231402,
  &cx__231406,
  &cx__231472,
  &cx__231481,
  &cx__231485,
  &cx__231491,
  &cx__231497,
  &cx__2315,
  &cx__231503,
  &cx__231509,
  &cx__23247,
  &cx__23626,
  &cx__23629,
  &cx__23631,
  &cx__23635,
  &cx__23915,
  &cx__23959,
  &cx__23965,
  &cx__23973,
  &cx__23978,
  &cx__23983,
  &cx__23991,
  &cx__23996,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[226] = {
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 28
static obj host(obj pc)
{
  register obj *r = cxg_regs;
  register obj *hp = cxg_hp;
#ifndef NDEBUG
  register int rc = cxg_rc;
#endif
  jump: 
  switch (case_from_obj(pc)) {

case 0: /* load module */
    { static char s[] = { 58, 10, 0 };
    cx__2314 = (hpushstr(0, newstring(s))); }
    cx__2315 = (hpushstr(0, newstring("-------------------------- ")));
    { static obj c[] = { obj_from_case(1) }; cx_begin_2Dexp_3F_2355 = (obj)c; }
    cx__23247 = (hpushstr(0, newstring("unknown exp")));
    { static obj c[] = { obj_from_case(3) }; cx_curry_2Dexp_3F_23513 = (obj)c; }
    { static char s[] = { 46, 10, 0 };
    cx__23626 = (hpushstr(0, newstring(s))); }
    cx__23629 = (hpushstr(0, newstring("no clause matches ~s")));
    cx__23631 = (hpushstr(0, newstring("Error: ")));
    cx__23635 = (hpushstr(0, newstring("Error in ~a: ")));
    cx__23915 = (hpushstr(0, newstring("void(0)")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%definition")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23959 = (hendblk(3)); }
    { static char s[] = { 34, 10, 0 };
    cx__23965 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 35, 105, 110, 99, 108, 117, 100, 101, 32, 34, 0 };
    cx__23973 = (hpushstr(0, newstring(s))); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%include")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23978 = (hendblk(3)); }
    { static char s[] = { 10, 0 };
    cx__23983 = (hpushstr(0, newstring(s))); }
    cx__23991 = (hpushstr(0, newstring("#include ")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<symbol>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%include")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23996 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("*")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("define")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231003 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("load")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231022 = (hendblk(3)); }
    cx__231039 = (hpushstr(0, newstring("  visiting ~a~%")));
    cx__231044 = (hpushstr(0, newstring("  already visited: ~a~%")));
    cx__231052 = (hpushstr(0, newstring("module_")));
    { static char s[] = { 32, 42, 47, 10, 0 };
    cx__231076 = (hpushstr(0, newstring(s))); }
    cx__231084 = (hpushstr(0, newstring("(void); /* ")));
    cx__231089 = (hpushstr(0, newstring("extern void ")));
    cx__231097 = (hpushstr(0, newstring("load should precede definitions and expressions")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("<string>")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("%localdef")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231105 = (hendblk(3)); }
    cx__231118 = (hpushstr(0, newstring("%% section(s) should precede definitions and expressions")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mksymbol(internsym("*")));
    *--hp = (mksymbol(internsym("%%")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__231121 = (hendblk(3)); }
    cx__231169 = (hpushstr(0, newstring("(); ")));
    cx__231182 = (hpushstr(0, newstring("#define LOAD() ")));
    cx__231189 = (hpushstr(0, newstring("#define MODULE ")));
    cx__231196 = (hpushstr(0, newstring("/* ")));
    cx__231269 = (hpushstr(0, newstring(".c")));
    { static char s[] = { 32, 119, 114, 10, 0 };
    cx__231278 = (hpushstr(0, newstring(s))); }
    cx__231282 = (hpushstr(0, newstring("C CODE")));
    cx__231285 = (hpushstr(0, newstring(" cg")));
    cx__231286 = (hpushstr(0, newstring("EXP AFTER VALUE UNBOXING")));
    cx__231290 = (hpushstr(0, newstring(" uv")));
    cx__231291 = (hpushstr(0, newstring("EXP AFTER LABEL CONVERSION")));
    cx__231295 = (hpushstr(0, newstring(" b3")));
    cx__231296 = (hpushstr(0, newstring("EXP AFTER LAMBDA LIFTING")));
    cx__231300 = (hpushstr(0, newstring(" ll")));
    cx__231301 = (hpushstr(0, newstring("EXP AFTER POST-CPS BETA")));
    cx__231305 = (hpushstr(0, newstring(" b2")));
    cx__231306 = (hpushstr(0, newstring("EXP AFTER CPS-CONVERSION")));
    cx__231310 = (hpushstr(0, newstring(" cps")));
    cx__231311 = (hpushstr(0, newstring("EXP AFTER BETA-SUBSTITUTION")));
    cx__231315 = (hpushstr(0, newstring(" b1")));
    cx__231316 = (hpushstr(0, newstring("EXP AFTER CONSTANT-FOLDING")));
    cx__231320 = (hpushstr(0, newstring(" cf")));
    cx__231322 = (hpushstr(0, newstring(" scg")));
    cx__231324 = (hpushstr(0, newstring("EXP ANALYSIS OF GLOBALS")));
    cx__231328 = (hpushstr(0, newstring(" ag")));
    cx__231329 = (hpushstr(0, newstring("EXP AFTER BOXING")));
    cx__231333 = (hpushstr(0, newstring(" ra")));
    cx__231334 = (hpushstr(0, newstring("EXP AFTER LETREC FIXING")));
    cx__231338 = (hpushstr(0, newstring("  fl")));
    cx__231339 = (hpushstr(0, newstring("EXP")));
    { static char s[] = { 10, 47, 42, 32, 111, 115, 32, 101, 110, 116, 114, 121, 32, 112, 111, 105, 110, 116, 32, 42, 47, 0 };
    cx__231360 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 98, 97, 115, 105, 99, 32, 114, 117, 110, 116, 105, 109, 101, 32, 42, 47, 0 };
    cx__231362 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 109, 111, 100, 117, 108, 101, 32, 108, 111, 97, 100, 32, 42, 47, 0 };
    cx__231367 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 104, 111, 115, 116, 32, 112, 114, 111, 99, 101, 100, 117, 114, 101, 32, 42, 47, 10, 0 };
    cx__231370 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 47, 42, 32, 101, 110, 116, 114, 121, 32, 112, 111, 105, 110, 116, 115, 32, 42, 47, 10, 0 };
    cx__231371 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 47, 42, 32, 103, 99, 32, 114, 111, 111, 116, 115, 32, 42, 47, 10, 0 };
    cx__231372 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 47, 42, 32, 104, 101, 108, 112, 101, 114, 32, 102, 117, 110, 99, 116, 105, 111, 110, 115, 32, 42, 47, 10, 0 };
    cx__231383 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 99, 120, 32, 103, 108, 111, 98, 97, 108, 115, 32, 42, 47, 10, 0 };
    cx__231386 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 101, 120, 116, 114, 97, 32, 100, 101, 102, 105, 110, 105, 116, 105, 111, 110, 115, 32, 42, 47, 10, 0 };
    cx__231397 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 115, 116, 97, 110, 100, 97, 114, 100, 32, 100, 101, 102, 105, 110, 105, 116, 105, 111, 110, 115, 32, 42, 47, 10, 0 };
    cx__231401 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 47, 42, 32, 101, 120, 116, 114, 97, 32, 105, 110, 99, 108, 117, 100, 101, 115, 32, 42, 47, 10, 0 };
    cx__231402 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 47, 42, 32, 115, 116, 97, 110, 100, 97, 114, 100, 32, 105, 110, 99, 108, 117, 100, 101, 115, 32, 42, 47, 10, 0 };
    cx__231406 = (hpushstr(0, newstring(s))); }
    cx__231472 = (hpushstr(0, newstring("sfc build 11q~%Usage: sfc [-vd] file ...~%")));
    cx__231481 = (hpushstr(0, newstring("  writing ~a~%")));
    cx__231485 = (hpushstr(0, newstring("compiling ~a:~%")));
    cx__231491 = (hpushstr(0, newstring("cannot open input file ~a~%")));
    cx__231497 = (hpushstr(0, newstring("--help")));
    cx__231503 = (hpushstr(0, newstring("-d")));
    cx__231509 = (hpushstr(0, newstring("-v")));
    { static obj c[] = { obj_from_case(5) }; cx_clear_2Dmemoizations = (obj)c; }
    cx__2Asilent_2A = obj_from_bool(1);
    cx__2Ashowlog_2A = obj_from_bool(0);
    { static obj c[] = { obj_from_case(8) }; cx_maybe_2Dpp_2Dsource = (obj)c; }
    { static obj c[] = { obj_from_case(13) }; cx_maybe_2Dpp_2Dtext = (obj)c; }
    { static obj c[] = { obj_from_case(17) }; cx_maybe_2Dlog = (obj)c; }
    { static obj c[] = { obj_from_case(19) }; cx_source = (obj)c; }
    { static obj c[] = { obj_from_case(93) }; cx_parse_2Dfile = (obj)c; }
    { static obj c[] = { obj_from_case(127) }; cx_compile_2Dfile = (obj)c; }
    { static obj c[] = { obj_from_case(181) }; cx_display_2Dmodule = (obj)c; }
    { static obj c[] = { obj_from_case(216) }; cx_main = (obj)c; }
    r[0] = obj_from_void(0);
    r[1+0] = r[0];
    pc = 0; /* exit from module init */
    r[1+1] = r[0];  
    r += 1; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 1: /* begin-exp?#55 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_begin_2Dexp_3F_2355: /* k exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_2385((r[1])))) {
    r[2] = (vectorref((r[1]), (1)));
    r[2] = (vectorref((r[2]), (1)));
    r[3] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[3]), (2)));
  if ((ispair((r[2])))) {
    r[4] = (cdr((r[2])));
    r[4] = obj_from_bool(isnull((r[4])));
  if (bool_from_obj(r[4])) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(2);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_var_2Dreferenced_2Din_2Dexp_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (car((r[2])));
    r[5+3] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 2: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(!bool_from_obj(r[1]));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 3: /* curry-exp?#513 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_curry_2Dexp_3F_23513: /* k exp */
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(4);
    r[4] = (hendblk(3+1));
    r[5+0] = (cx_labelapp_2Dexp_3F);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 4: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k body */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[0] = r[3];  
    r[1] = r[2];  
    r[2] = r[5];  
    goto s_loop;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop: /* k ids rands */
  if ((isnull((r[1])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[2])))) {
    r[3] = (car((r[2])));
  if ((isvector((r[3])))) {
  if (((vectorlen((r[3]))) == (2))) {
    r[4] = (vectorref((r[3]), (0)));
    r[3] = obj_from_bool((r[4]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    r[3] = (car((r[2])));
    r[3] = (vectorref((r[3]), (1)));
    r[4] = (car((r[1])));
    r[3] = obj_from_bool((r[3]) == (r[4]));
  if (bool_from_obj(r[3])) {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

case 5: /* clear-memoizations k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
gs_clear_2Dmemoizations: /* k */
    hreserve(hbsz(1+1), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(6);
    r[1] = (hendblk(1+1));
    r[2+0] = (cx_exp_2Dvinfo);
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = r[1];  
    r[2+2] = obj_from_bool(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 6: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(7);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_var_2Duses_2Din_2Dexp);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = obj_from_bool(0);
    r[4+3] = obj_from_bool(0);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 7: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_exp_2D_3Efree_2Dvars);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 8: /* maybe-pp-source k msg exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_maybe_2Dpp_2Dsource: /* k msg exp */
  if ((!bool_from_obj(cx__2Asilent_2A))) {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(9);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx__2315);
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 9: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  msg exp k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(10);
    r[5] = (hendblk(2+1));
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 10: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(11);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__2314);
    r[5+3] = obj_from_bool(1);
    r[5+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 11: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(12);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_source;

case 12: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r[3+3] = obj_from_bool(0);
    r[3+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 13: /* maybe-pp-text k msg text */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_maybe_2Dpp_2Dtext: /* k msg text */
  if ((!bool_from_obj(cx__2Asilent_2A))) {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(14);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx__2315);
    r[4+3] = obj_from_bool(1);
    r[4+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 14: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  msg text k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(15);
    r[5] = (hendblk(2+1));
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 15: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  text k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(16);
    r[4] = (hendblk(2+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__2314);
    r[5+3] = obj_from_bool(1);
    r[5+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 16: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  text k */
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 17: /* maybe-log k msg */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_maybe_2Dlog: /* k msg */
  if (bool_from_obj(cx__2Ashowlog_2A)) {
    r[2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(18);
    r[3] = (hendblk(2+1));
    r[4+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r[4+3] = obj_from_bool(1);
    r[4+4] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_void(0);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 18: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k op */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(fflush(oportdata((r[3]))));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 19: /* source k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_source: /* k exp */
  if (bool_from_obj(cxs_void_2Dexp_3F_23912((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mksymbol(internsym("%%void")));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3+0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[0];  
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("varassign-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(20);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[3];  
    goto gs_source;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[2] = (vectorref((r[1]), (0)));
    r[2] = obj_from_bool((r[2]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (vectorref((r[1]), (1)));
    r[3] = (vectorref((r[1]), (2)));
    r[4] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_case(22);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_source;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_23145((r[1])))) {
    /* r[0] */    
    r[1] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_23122((r[1])));
    goto gs_source;
  } else {
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(88);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(25);
    r[2] = (hendblk(3+1));
    r[3+0] = (cx_letrec_2Dexp_3F);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }
  }

case 20: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(21);
    r[5] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 21: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("set!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 22: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp test-exp k */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(23);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[2];  
    goto gs_source;

case 23: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r test-exp k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(24);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[2];  
    goto gs_source;

case 24: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("if")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 25: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (1)));
    r[5] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[6]), (2)));
    r[7] = (vectorref((r[6]), (2)));
    r[6] = (car((r[7])));
    r[6] = (vectorref((r[6]), (2)));
    r[7] = (vectorref((r[4]), (1)));
    r[7] = (vectorref((r[7]), (2)));
    r[8] = (vectorref((r[7]), (1)));
    r[7] = (vectorref((r[8]), (2)));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(26);
    r[8] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(31);
    r[5] = (hendblk(3+1));
    r[0] = (cx_letrec_2A_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 26: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r rands ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(30);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4145;

s_loop_v4145: /* k id id */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(27);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v4145;
  }

case 27: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(28);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;

case 28: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(29);
    r[6] = (hendblk(3+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 29: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 30: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("letrec")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 31: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(32);
    r[5] = (hendblk(3+1));
    r[0] = (cx_letrec_2A_2Dexp_2D_3Ebody);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(39);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_begin_2Dexp_3F_2355;
  }

case 32: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(33);
    r[5] = (hendblk(4+1));
    r[0] = (cx_letrec_2A_2Dexp_2D_3Erands);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 33: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body r k exp */
    r[6] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[6]), (1)));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_case(34);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 34: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(38);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4096;

s_loop_v4096: /* k id id */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(35);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v4096;
  }

case 35: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(36);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;

case 36: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(37);
    r[6] = (hendblk(3+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 37: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 38: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("letrec*")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 39: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[5] = (car((r[5])));
    r[6] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[6]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(40);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_2385((r[4])))) {
    r[5] = (vectorref((r[4]), (1)));
    r[5] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    r[7] = (vectorref((r[4]), (1)));
    r[7] = (vectorref((r[7]), (2)));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(42);
    r[8] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (4))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    r[7] = (vectorref((r[4]), (3)));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(47);
    r[8] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(60);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto s_loop_v3936;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(62);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_curry_2Dexp_3F_23513;
  }
  }
  }
  }

case 40: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp1 k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(41);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_source;

case 41: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("begin")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 42: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r rands ids k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(46);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4039;

s_loop_v4039: /* k id id */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = obj_from_case(43);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v4039;
  }

case 43: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id id */
    r[5] = (car((r[4])));
    r[6] = (car((r[3])));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[5];  
    *--hp = obj_from_case(44);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;

case 44: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(45);
    r[6] = (hendblk(3+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 45: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 46: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("let")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 47: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body lams ids k */
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = obj_from_case(57);
    r[6] = (hendblk(2+1));
    r[7+0] = r[6];  
    r[7+1] = r[4];  
    r[7+2] = r[3];  
    r[7+3] = r[2];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3961;

s_loop_v3961: /* k id id source-body */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    r[5] = (cdr((r[2])));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(48);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[4];  
    r[2] = r[5];  
    /* r[3] */    
    goto s_loop_v3961;
  }

case 48: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k id id */
    r[6] = (car((r[5])));
    r[7] = (car((r[4])));
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(49);
    r[8] = (hendblk(2+1));
  if ((isvector((r[7])))) {
  if (((vectorlen((r[7]))) == (3))) {
    r[9] = (vectorref((r[7]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[7]), (1)));
    r[10] = (vectorref((r[7]), (2)));
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = obj_from_case(50);
    r[11] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[9] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[9]))));
    hreserve(hbsz(3+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = obj_from_case(54);
    r[10] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[9];  
    r[3] = (cx__23635);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__23631);
    r[3] = obj_from_bool(1);
    r[4] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }

case 49: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 50: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids id k */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(53);
    r[6] = (hendblk(2+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto s_loop_v3990;

s_loop_v3990: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(51);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3990;
  }

case 51: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(52);
    r[4] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 52: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 53: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("define")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 54: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  lam ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(55);
    r[5] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[3];  
    r[7+3] = (cx__23629);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 55: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(56);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__23626);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 56: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_reset);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 57: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { fixnum_t v4342_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4342_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4342_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 5); /* 5 live regs */
    p = r[4];   /* gc-safe */
    *--hp = p; *--hp = (mknull());
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("let")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[5] = (p); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v3936: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(58);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3936;
  }

case 58: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(59);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_source;

case 59: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 60: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r rator k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(61);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_source;

case 61: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 62: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[5] = (vectorref((r[5]), (1)));
    r[5] = (vectorref((r[5]), (1)));
    r[6] = (vectorref((r[4]), (1)));
    { fixnum_t v4341_tmp;
    r[7] = (vectorref((r[4]), (2)));
    r[7] = (vectorref((r[7]), (2)));
    r[8] = (vectorref((r[4]), (1)));
    { /* length */
    int n; obj l = r[8];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4341_tmp = (n); }
    { /* list-tail */
    obj l = r[7];   int c = (v4341_tmp);
    while (c-- > 0) l = cdr(l);
    r[7] = (l); } }
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_case(65);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    r[1] = r[7];  
    goto s_loop_v3903;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(70);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(74);
    r[5] = (hendblk(3+1));
    r[0] = (cx_box_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }

s_loop_v3903: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(63);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3903;
  }

case 63: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(64);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_source;

case 64: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 65: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids id k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(68);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v3882;

s_loop_v3882: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(66);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3882;
  }

case 66: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(67);
    r[4] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 67: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 68: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(69);
    r[5] = (hendblk(3+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 69: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*4, 5); /* 5 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("%%curry")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[5] = (p); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 70: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r ids k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(73);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto s_loop_v3847;

s_loop_v3847: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(71);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3847;
  }

case 71: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(72);
    r[4] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 72: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 73: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("lambda")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 74: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (3)));
    r[5] = (car((r[5])));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(75);
    r[6] = (hendblk(1+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto gs_source;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(76);
    r[5] = (hendblk(3+1));
    r[0] = (cx_boxref_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 75: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("%%box")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 76: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (3)));
    r[5] = (car((r[5])));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(77);
    r[6] = (hendblk(1+1));
    r[0] = r[6];  
    r[1] = r[5];  
    goto gs_source;
  } else {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(78);
    r[5] = (hendblk(3+1));
    r[0] = (cx_boxset_2Dexp_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 77: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("%%box-ref")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 78: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (3)));
    r[5] = (car((r[5])));
    r[6] = (vectorref((r[4]), (3)));
    r[7] = (cdr((r[6])));
    r[6] = (car((r[7])));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(79);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (4))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    r[7] = (vectorref((r[4]), (3)));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = obj_from_case(83);
    r[8] = (hendblk(3+1));
    r[0] = r[8];  
    r[1] = r[7];  
    goto s_loop_v3786;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("letcc-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(84);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (3))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("withcc-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[4]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(86);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_source;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cx__23247);
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }
  }
  }

case 79: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r box k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(80);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto gs_source;

case 80: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("%%box-set!")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v3786: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(81);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3786;
  }

case 81: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(82);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = (car((r[2])));
    goto gs_source;

case 82: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 83: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k prim effect */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 5); /* 5 live regs */
    p = r[1];   /* gc-safe */
    *--hp = p; *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (((((r[4]) == (mksymbol(internsym("no-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("no-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("no-effect"))))))) && (1)) ? (mksymbol(internsym("%prim"))) : (((((r[4]) == (mksymbol(internsym("*-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("*-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("*-effect"))))))) && (1)) ? (mksymbol(internsym("%prim*"))) : (((((r[4]) == (mksymbol(internsym("?-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("?-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("?-effect"))))))) && (1)) ? (mksymbol(internsym("%prim?"))) : (((((r[4]) == (mksymbol(internsym("!-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("!-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("!-effect"))))))) && (1)) ? (mksymbol(internsym("%prim!"))) : (((((r[4]) == (mksymbol(internsym("?!-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("?!-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("?!-effect"))))))) && (1)) ? (mksymbol(internsym("%prim?!"))) : (((((r[4]) == (mksymbol(internsym("*?-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("*?-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("*?-effect"))))))) && (1)) ? (mksymbol(internsym("%prim*?"))) : (((((r[4]) == (mksymbol(internsym("*!-effect")))) || ((is_flonum_obj(r[4])) && ((is_flonum_obj(mksymbol(internsym("*!-effect")))) && (flonum_from_obj(r[4]) == flonum_from_obj(mksymbol(internsym("*!-effect"))))))) && (1)) ? (mksymbol(internsym("%prim*!"))) : (mksymbol(internsym("%prim*?!"))))))))));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[5] = (p); }
    r[6+0] = r[2];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 84: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(85);
    r[4] = (hendblk(2+1));
    r[0] = (cx_id_2D_3Euname);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 85: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* list* */
    obj p;
    hreserve(hbsz(3)*3, 4); /* 4 live regs */
    p = r[3];   /* gc-safe */
    *--hp = p; *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    *--hp = p; *--hp = (mksymbol(internsym("letcc")));
    *--hp = obj_from_size(PAIR_BTAG); p = hendblk(3);
    r[4] = (p); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 86: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r cont-exp k */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(87);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto gs_source;

case 87: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (mksymbol(internsym("withcc")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 88: /* clo k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_source_2Dbody: /* k exp */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(88);
    r[2] = (hendblk(0+1));
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_23145((r[1])))) {
    r[3] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_23122((r[1])));
    /* r[0] */    
    r[1] = r[3];  
    goto s_source_2Dbody;
  } else {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(89);
    r[3] = (hendblk(3+1));
    r[0] = r[3];  
    /* r[1] */    
    goto gs_begin_2Dexp_3F_2355;
  }

case 89: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r source-body k exp */
  if (bool_from_obj(r[1])) {
    r[5] = (vectorref((r[4]), (2)));
    r[5] = (car((r[5])));
    r[6] = (vectorref((r[4]), (1)));
    r[6] = (vectorref((r[6]), (2)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = obj_from_case(90);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(92);
    r[5] = (hendblk(1+1));
    r[0] = r[5];  
    r[1] = r[4];  
    goto gs_source;
  }

case 90: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r exp1 k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(91);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_source;

case 91: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 92: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 93: /* parse-file k filename */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_parse_2Dfile: /* k filename */
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(94);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(1), 3); /* 3 live regs */
    *--hp = obj_from_void(0);
    r[3] = (hendblk(1));
    hreserve(hbsz(1), 4); /* 4 live regs */
    *--hp = obj_from_void(0);
    r[4] = (hendblk(1));
    hreserve(hbsz(1), 5); /* 5 live regs */
    *--hp = obj_from_void(0);
    r[5] = (hendblk(1));
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(115);
    r[6] = (hendblk(4+1));
    hreserve(hbsz(7+1), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = obj_from_case(97);
    r[6] = (hendblk(7+1));
    r[7+0] = (cx_null_2Dmstore);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 94: /* clo k filename */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k filename */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(95);
    r[2] = (hendblk(1+1));
    r[3+0] = (cx_path_2Dstrip_2Ddirectory);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 95: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(96);
    r[3] = (hendblk(1+1));
    r[4+0] = (cx_path_2Dstrip_2Dextension);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 96: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_c_2Dmangle);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r[3+3] = (cx__231052);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 97: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! module-name k filename module-visits module-loads mstore */
    (void)(objptr_from_obj(r[8])[0] = (r[1]));
    (void)(objptr_from_obj(r[7])[0] = (mknull()));
    (void)(objptr_from_obj(r[6])[0] = (mknull()));
    r[9] = (mkiport(9, cxm_cknull(fopen(stringchars((r[5])), "r"), "fopen")));
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_case(114);
    r[10] = (hendblk(2+1));
    r[11+0] = (r[10]);
    r[11+1] = (mknull());
    r[11+2] = (mknull());
    r[11+3] = (mknull());
    r[11+4] = (mknull());
    r[11+5] = r[9];  
    r[11+6] = r[2];  
    r[11+7] = r[8];  
    r[11+8] = r[7];  
    r[11+9] = r[3];  
    r[11+10] = r[5];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3574;

case 98: /* clo k specs incs defs forms */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4];
    r[1+9] = p[5];
    r[1+10] = p[6]; }
    r += 1; /* shift reg. wnd */
s_loop_v3574: /* k specs incs defs forms p module-add-load! mstore module-loads module-name filename */
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(98);
    r[11] = (hendblk(6+1));
    hreserve(hbsz(11+1), 12); /* 12 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = (r[11]);
    *--hp = r[6];  
    *--hp = obj_from_case(99);
    r[11] = (hendblk(11+1));
    r[12+0] = (cx_read_2F1);
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = r[5];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 99: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop mstore forms module-loads module-name k specs filename incs defs */
  if ((iseof((r[1])))) {
    hreserve(hbsz(7+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(100);
    r[13] = (hendblk(7+1));
    { fixnum_t v4340_tmp;
    { /* length */
    int n; obj l = r[5];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4340_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v4340_tmp);
    hreserve(hbsz(3)*c, 14); /* 14 live regs */
    l = r[5];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[14] = (o); } }
    r[0] = (cx_expand_2Dtop_2Dlevel_2Dforms_21);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (r[14]);
    r[3] = (objptr_from_obj(r[4])[0]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 13); /* 13 live regs */
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = (r[11]);
    *--hp = (r[12]);
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(105);
    r[13] = (hendblk(8+1));
    r[14+0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[13]);
    r[14+2] = (cx__23996);
    r[14+3] = r[1];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 100: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-loads module-name k specs filename incs defs */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(101);
    r[9] = (hendblk(7+1));
    r[10+0] = (cx_parse_2Dprogram);
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[9];  
    r[10+2] = r[1];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 101: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-loads module-name k specs filename incs defs */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { fixnum_t v4338_tmp;
    { /* length */
    int n; obj l = r[8];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4338_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v4338_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[8];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[10] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { fixnum_t v4339_tmp;
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4339_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v4339_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = r[7];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[11] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[12] = (objptr_from_obj(r[2])[0]);
    hreserve(hbsz(5+1), 13); /* 13 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = (r[11]);
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(103);
    r[13] = (hendblk(5+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    goto s_loop_v3651;

s_loop_v3651: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (cdr((r[1])));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(102);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v3651;
  }

case 102: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = (cx__231169);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 103: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-name k r specs filename */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = (cx__23983);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__231182);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__23983);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(104);
    r[10] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 104: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r specs filename r */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = (cx__231189);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__231076);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__231196);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { fixnum_t v4337_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4337_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v4337_tmp);
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    l = r[4];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[12] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 13); /* 13 live regs */
    *--hp = r[3];  
    *--hp = (r[12]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[13]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 105: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs incs r specs k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__23983);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11] = (cdr((r[7])));
    r[11] = (car((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__23991);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = (r[10]);
    r[11+4] = r[5];  
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(106);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__23978);
    r[3] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 106: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs incs r specs k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__23965);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11] = (cdr((r[7])));
    r[11] = (car((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__23973);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = (r[10]);
    r[11+4] = r[5];  
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(107);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231121);
    r[3] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 107: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs incs specs r k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(108);
    r[10] = (hendblk(7+1));
  if ((ispair((r[4])))) {
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231118);
    r[3] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    goto s_l_v3618;
  }
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(109);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__23959);
    r[3] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 108: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
s_l_v3618: /* ek  loop forms defs incs specs r k */
    r[9] = (cdr((r[7])));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[6];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[9];  
    r[10+3] = r[5];  
    r[10+4] = r[4];  
    r[10+5] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 109: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs r incs specs k */
  if (bool_from_obj(r[1])) {
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[5];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = r[7];  
    r[11+4] = (r[10]);
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(110);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231105);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 110: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs r incs specs k */
  if (bool_from_obj(r[1])) {
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[5];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = r[7];  
    r[11+4] = (r[10]);
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(111);
    r[10] = (hendblk(8+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231022);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 111: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop forms defs incs specs r k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(112);
    r[10] = (hendblk(8+1));
  if ((ispair((r[4])))) {
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231097);
    r[3] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_l_v3590;
  }
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = r[3];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[7];  
    r[11+3] = r[6];  
    r[11+4] = r[5];  
    r[11+5] = (r[10]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 112: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
s_l_v3590: /* ek  module-add-load! loop forms defs incs specs r k */
    r[10] = (cdr((r[8])));
    r[10] = (car((r[10])));
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(113);
    r[11] = (hendblk(6+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    r[3] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 113: /* clo k name xincs xdefs */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5];
    r[1+9] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* k name xincs xdefs loop forms defs incs specs r */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__231076);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11] = (cdr((r[9])));
    r[11] = (car((r[11])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__231084);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__231089);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
  } else {
    r[10] = r[8];  
  }
    { fixnum_t v4336_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4336_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4336_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = r[2];   t = r[7];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[11] = (o); } }
    { fixnum_t v4335_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4335_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4335_tmp);
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    l = r[3];   t = r[6];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[12] = (o); } }
    r[13+0] = r[4];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = r[0];  
    r[13+2] = (r[10]);
    r[13+3] = (r[11]);
    r[13+4] = (r[12]);
    r[13+5] = r[5];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 114: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k p */
    (void)(fclose(iportdata((r[3]))));
    r[4] = r[1];  
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 115: /* clo k str return */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* k str return module-name mstore module-loads module-visits */
    hreserve(hbsz(1), 7); /* 7 live regs */
    *--hp = obj_from_void(0);
    r[7] = (hendblk(1));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(115);
    r[8] = (hendblk(4+1));
    hreserve(hbsz(8+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = obj_from_case(116);
    r[8] = (hendblk(8+1));
    r[9+0] = r[3];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 116: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r mstore module-add-load! module-loads str return k module-visits load-name */
    (void)(objptr_from_obj(r[9])[0] = (r[1]));
    r[10] = (objptr_from_obj(r[9])[0]);
    r[11] = (objptr_from_obj(r[8])[0]);
    r[10] = (ismember((r[10]), (r[11])));
  if (bool_from_obj(r[10])) {
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(117);
    r[10] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[3] = (cx__231044);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 10); /* 10 live regs */
    *--hp = r[8];  
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(118);
    r[10] = (hendblk(8+1));
  if (bool_from_obj(cx__2Ashowlog_2A)) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[3] = (cx__231039);
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[11+0] = obj_from_ktrap();
    r[11+1] = obj_from_void(0);
    r[11+2] = r[2];  
    r[11+3] = r[5];  
    r[11+4] = r[3];  
    r[11+5] = r[6];  
    r[11+6] = r[4];  
    r[11+7] = r[7];  
    r[11+8] = r[9];  
    r[11+9] = r[8];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v3491;
  }
  }

case 117: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  return k */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = obj_from_bool(0);
    r[4+3] = (mknull());
    r[4+4] = (mknull());
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 118: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
s_l_v3491: /* ek  mstore str module-add-load! return module-loads k load-name module-visits */
    r[10] = (objptr_from_obj(r[9])[0]);
    r[11] = (objptr_from_obj(r[8])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    (void)(objptr_from_obj(r[9])[0] = (r[10]));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(119);
    r[10] = (hendblk(5+1));
    r[11+0] = (cx_file_2Dexpand_2Dtop_2Dlevel_2Dforms_21);
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = (r[10]);
    r[11+2] = r[3];  
    r[11+3] = (objptr_from_obj(r[2])[0]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 119: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! return load-name module-loads k */
    r[7+0] = r[6];  
    r[7+1] = r[1];  
    r[7+2] = (mknull());
    r[7+3] = (mknull());
    r[7+4] = (mknull());
    r[7+5] = r[2];  
    r[7+6] = r[3];  
    r[7+7] = r[4];  
    r[7+8] = r[5];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v3494;

case 120: /* clo k forms incs defs sdefs */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4]; }
    r += 1; /* shift reg. wnd */
s_loop_v3494: /* k forms incs defs sdefs module-add-load! return load-name module-loads */
  if ((isnull((r[1])))) {
  if ((isnull((r[4])))) {
    r[9] = obj_from_bool(0);
  } else {
    r[9] = (objptr_from_obj(r[8])[0]);
    r[10] = (objptr_from_obj(r[7])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    (void)(objptr_from_obj(r[8])[0] = (r[9]));
    r[9] = (objptr_from_obj(r[7])[0]);
  }
    r[10+0] = r[6];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[0];  
    r[10+2] = r[9];  
    r[10+3] = r[2];  
    r[10+4] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[9] = (car((r[1])));
    r[10] = (cdr((r[1])));
    hreserve(hbsz(4+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(120);
    r[11] = (hendblk(4+1));
    hreserve(hbsz(8+1), 12); /* 12 live regs */
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[11]);
    *--hp = r[5];  
    *--hp = obj_from_case(121);
    r[11] = (hendblk(8+1));
    r[12+0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = (cx__231022);
    r[12+3] = r[9];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 121: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek r module-add-load! loop sdefs defs incs forms s k */
  if (bool_from_obj(r[1])) {
    r[10] = (cdr((r[8])));
    r[10] = (car((r[10])));
    hreserve(hbsz(5+1), 11); /* 11 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(122);
    r[11] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    r[3] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(123);
    r[10] = (hendblk(7+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (cx__231003);
    r[3] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 122: /* clo k name xincs xdefs */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* k name xincs xdefs loop sdefs defs incs forms */
    { fixnum_t v4334_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4334_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4334_tmp);
    hreserve(hbsz(3)*c, 9); /* 9 live regs */
    l = r[2];   t = r[7];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[9] = (o); } }
    { fixnum_t v4333_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v4333_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v4333_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[3];   t = r[6];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[10] = (o); } }
    r[11+0] = r[4];  
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[0];  
    r[11+2] = r[8];  
    r[11+3] = r[9];  
    r[11+4] = (r[10]);
    r[11+5] = r[5];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 123: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop sdefs s defs incs forms k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[7];  
    r[10+3] = r[6];  
    r[10+4] = r[5];  
    r[10+5] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(124);
    r[9] = (hendblk(7+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cx__23996);
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 124: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop sdefs defs incs s forms k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__23983);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__23991);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[7];  
    r[10+3] = r[9];  
    r[10+4] = r[4];  
    r[10+5] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(125);
    r[9] = (hendblk(7+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cx__23978);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 125: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop sdefs defs incs s forms k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__23965);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10] = (cdr((r[6])));
    r[10] = (car((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__23973);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[7];  
    r[10+3] = r[9];  
    r[10+4] = r[4];  
    r[10+5] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(126);
    r[9] = (hendblk(7+1));
    r[0] = (cx_sexp_2Dmatch_3F);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cx__23959);
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 126: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop sdefs defs s incs forms k */
  if (bool_from_obj(r[1])) {
    r[9] = (cdr((r[5])));
    r[9] = (car((r[9])));
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[8];  
    r[10+2] = r[7];  
    r[10+3] = r[6];  
    r[10+4] = r[9];  
    r[10+5] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[8];  
    r[9+2] = r[7];  
    r[9+3] = r[6];  
    r[9+4] = r[4];  
    r[9+5] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 127: /* compile-file k filename */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_compile_2Dfile: /* k filename */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(128);
    r[2] = (hendblk(2+1));
    r[3+0] = (cx_reset_2Dtimestamps);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;

case 128: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  filename k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(129);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    goto gs_clear_2Dmemoizations;

case 129: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  filename k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(130);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[2];  
    goto gs_parse_2Dfile;

case 130: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename k */
    r[4] = (car((r[1])));
    r[5] = (cdr((r[1])));
    r[5] = (car((r[5])));
    r[6] = (cdr((r[1])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[7] = (cdr((r[1])));
    r[7] = (cdr((r[7])));
    r[7] = (cdr((r[7])));
    r[7] = (car((r[7])));
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = r[7];  
    *--hp = obj_from_case(131);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231339);
    r[2] = r[7];  
    goto gs_maybe_2Dpp_2Dsource;

case 131: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(132);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231338);
    goto gs_maybe_2Dlog;

case 132: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp filename defs incs specs k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(133);
    r[8] = (hendblk(5+1));
    r[0] = (cx_fix_2Dletrecs);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 133: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename defs incs specs k */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(134);
    r[7] = (hendblk(6+1));
    r[8+0] = r[7];  
    r[8+1] = (cx__231334);
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 134: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(135);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    goto gs_clear_2Dmemoizations;

case 135: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(136);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231333);
    goto gs_maybe_2Dlog;

case 136: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(137);
    r[8] = (hendblk(5+1));
    r[0] = (cx_remove_2Dassignments);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 137: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename defs incs specs k */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(138);
    r[7] = (hendblk(6+1));
    r[8+0] = r[7];  
    r[8+1] = (cx__231329);
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 138: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(139);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    goto gs_clear_2Dmemoizations;

case 139: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(140);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231328);
    goto gs_maybe_2Dlog;

case 140: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(141);
    r[8] = (hendblk(5+1));
    r[0] = (cx_analyze_2Dglobals);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 141: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename defs incs specs k */
    hreserve(hbsz(6+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(142);
    r[7] = (hendblk(6+1));
    r[8+0] = r[7];  
    r[8+1] = (cx__231324);
    r[8+2] = (car((r[1])));
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 142: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(143);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    goto gs_clear_2Dmemoizations;

case 143: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(144);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = (cx__231322);
    goto gs_maybe_2Dlog;

case 144: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename defs incs specs k */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(145);
    r[8] = (hendblk(6+1));
    r[0] = (cx_stack_2Dfunctions_2Dcode_2Dgenerate);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = (cdr((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 145: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r r filename defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(146);
    r[8] = (hendblk(7+1));
    r[0] = r[8];  
    r[1] = (cx__231320);
    goto gs_maybe_2Dlog;

case 146: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(147);
    r[9] = (hendblk(6+1));
    r[0] = (cx_constant_2Dfold);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (car((r[2])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 147: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(148);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231316);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 148: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(149);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 149: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(150);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231315);
    goto gs_maybe_2Dlog;

case 150: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(151);
    r[9] = (hendblk(6+1));
    r[0] = (cx_beta_2Dsubstitute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    r[3] = obj_from_bool(0);
    r[4] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 151: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(152);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231311);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 152: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(153);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 153: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(154);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231310);
    goto gs_maybe_2Dlog;

case 154: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(155);
    r[9] = (hendblk(6+1));
    r[0] = (cx_cps_2Dconvert);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 155: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(156);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231306);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 156: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(157);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 157: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(158);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231305);
    goto gs_maybe_2Dlog;

case 158: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(159);
    r[9] = (hendblk(6+1));
    r[0] = (cx_beta_2Dsubstitute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 159: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(160);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231301);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 160: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(161);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 161: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(162);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231300);
    goto gs_maybe_2Dlog;

case 162: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(163);
    r[9] = (hendblk(6+1));
    r[0] = (cx_lambda_2Dlift);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 163: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(164);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231296);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 164: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(165);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 165: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(166);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231295);
    goto gs_maybe_2Dlog;

case 166: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(167);
    r[9] = (hendblk(6+1));
    r[0] = (cx_beta_2Dsubstitute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    r[3] = obj_from_bool(1);
    r[4] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 167: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(168);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231291);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 168: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(169);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 169: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(170);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231290);
    goto gs_maybe_2Dlog;

case 170: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(171);
    r[9] = (hendblk(6+1));
    r[0] = (cx_unbox_2Dvalues);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 171: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(172);
    r[8] = (hendblk(7+1));
    r[9+0] = r[8];  
    r[9+1] = (cx__231286);
    r[9+2] = r[1];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dsource;

case 172: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(173);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    goto gs_clear_2Dmemoizations;

case 173: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(174);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231285);
    goto gs_maybe_2Dlog;

case 174: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  r filename r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(175);
    r[9] = (hendblk(6+1));
    r[0] = (cx_code_2Dgenerate);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 175: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r filename r defs incs specs k */
    hreserve(hbsz(7+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(176);
    r[8] = (hendblk(7+1));
    r[9] = (cdr((r[1])));
    r[9] = (car((r[9])));
    r[10+0] = r[8];  
    r[10+1] = (cx__231282);
    r[10+2] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_maybe_2Dpp_2Dtext;

case 176: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  filename r r defs incs specs k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(177);
    r[9] = (hendblk(7+1));
    r[0] = r[9];  
    r[1] = (cx__231278);
    goto gs_maybe_2Dlog;

case 177: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* ek  filename r r defs incs specs k */
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(178);
    r[9] = (hendblk(6+1));
    r[0] = (cx_path_2Dstrip_2Dextension);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 178: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r r r defs incs specs k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__231269)));
    r[8] = (hpushstr(8, d)); }
    { /* delete-file */ 
    int res = remove(stringchars((r[8])));
    (void)(res == 0); }
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(179);
    r[9] = (hendblk(2+1));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(180);
    r[10] = (hendblk(5+1));
    r[11+0] = (cx_with_2Doutput_2Dto_2Dfile);
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = r[8];  
    r[11+3] = (r[10]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 179: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k r */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 180: /* clo k */
    assert(rc == 2);
    { obj* p = objptr_from_obj(r[0]);
    r[1+1] = p[1];
    r[1+2] = p[2];
    r[1+3] = p[3];
    r[1+4] = p[4];
    r[1+5] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* k r r defs incs specs */
    r[6+0] = r[0];  
    r[6+1] = r[5];  
    r[6+2] = r[4];  
    r[6+3] = r[3];  
    r[6+4] = r[2];  
    r[6+5] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_display_2Dmodule;

case 181: /* display-module k specs incs defs sfcs module-code */
    assert(rc == 7);
    r += 1; /* shift reg. wnd */
gs_display_2Dmodule: /* k specs incs defs sfcs module-code */
    r[6] = (car((r[5])));
    r[7] = (cdr((r[5])));
    r[7] = (car((r[7])));
    r[8] = (cdr((r[5])));
    r[8] = (cdr((r[8])));
    r[8] = (car((r[8])));
    r[9] = (cdr((r[5])));
    r[9] = (cdr((r[9])));
    r[9] = (cdr((r[9])));
    r[9] = (car((r[9])));
    { /* list-ref */
    obj l = r[5];   int c = (4);
    while (c-- > 0) l = cdr(l);
    r[10] = (car(l)); }
    { /* list-ref */
    obj l = r[5];   int c = (5);
    while (c-- > 0) l = cdr(l);
    r[11] = (car(l)); }
    hreserve(hbsz(0+1), 12); /* 12 live regs */
    *--hp = obj_from_case(182);
    r[12] = (hendblk(0+1));
    hreserve(hbsz(11+1), 13); /* 13 live regs */
    *--hp = r[6];  
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = r[7];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = (r[12]);
    *--hp = r[8];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(183);
    r[12] = (hendblk(11+1));
    r[13+0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = (r[12]);
    r[13+2] = r[1];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 182: /* clo k s */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k s */
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
    *--hp = (cx__23983);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[2] = (hendblk(3)); }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 183: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek  incs defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(11+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(184);
    r[13] = (hendblk(11+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cx__231406);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 184: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek  incs defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(11+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(185);
    r[13] = (hendblk(11+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cx_code_2Dstandard_2Dincludes);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 185: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10];
    r[1+12] = p[11]; }
    r += 1; /* shift reg. wnd */
    /* ek  incs defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(10+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(186);
    r[13] = (hendblk(10+1));
  if ((ispair((r[2])))) {
    hreserve(hbsz(2+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = r[2];  
    *--hp = obj_from_case(215);
    r[14] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[14]);
    r[2] = (cx__231402);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    r[9] = (r[10]);
    r[10] = (r[11]);
    r[11] = (r[12]);
    goto s_l_v2950;
  }

case 186: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
s_l_v2950: /* ek  defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(187);
    r[12] = (hendblk(10+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cx__231401);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 187: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
    /* ek  defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(188);
    r[12] = (hendblk(10+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cx_code_2Dstandard_2Ddefinitions);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 188: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9];
    r[1+11] = p[10]; }
    r += 1; /* shift reg. wnd */
    /* ek  defs globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(9+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(189);
    r[12] = (hendblk(9+1));
  if ((ispair((r[2])))) {
    hreserve(hbsz(3+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(211);
    r[13] = (hendblk(3+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cx__231397);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    r[8] = r[9];  
    r[9] = (r[10]);
    r[10] = (r[11]);
    goto s_l_v2985;
  }

case 189: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
s_l_v2985: /* ek  globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(190);
    r[11] = (hendblk(9+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (cx__231386);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 190: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8];
    r[1+10] = p[9]; }
    r += 1; /* shift reg. wnd */
    /* ek  globals add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(191);
    r[11] = (hendblk(8+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 191: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6];
    r[1+8] = p[7];
    r[1+9] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* ek  add-nl sfcs roots cases liveregs code k main? */
    hreserve(hbsz(6+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(192);
    r[10] = (hendblk(6+1));
  if ((ispair((r[3])))) {
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(207);
    r[11] = (hendblk(3+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (cx__231383);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    r[2] = r[4];  
    r[3] = r[5];  
    r[4] = r[6];  
    r[5] = r[7];  
    r[6] = r[8];  
    r[7] = r[9];  
    goto s_l_v3020;
  }

case 192: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
s_l_v3020: /* ek  roots cases liveregs code k main? */
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(193);
    r[8] = (hendblk(6+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = (cx__231372);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 193: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  roots cases liveregs code k main? */
    hreserve(hbsz(5+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(194);
    r[8] = (hendblk(5+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 194: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  cases liveregs code k main? */
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(195);
    r[7] = (hendblk(5+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = (cx__231371);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 195: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek  cases liveregs code k main? */
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(196);
    r[7] = (hendblk(4+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 196: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  liveregs code k main? */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(197);
    r[6] = (hendblk(4+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__231370);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 197: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  liveregs code k main? */
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(198);
    r[6] = (hendblk(3+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 198: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  code k main? */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(199);
    r[5] = (hendblk(3+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cx_code_2Dhost_2Dprologue);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 199: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  code k main? */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(200);
    r[5] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 200: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k main? */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(201);
    r[4] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx_code_2Dhost_2Depilogue);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 201: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k main? */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(202);
    r[4] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx__231367);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 202: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k main? */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(203);
    r[4] = (hendblk(2+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx_code_2Dmodule);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 203: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k main? */
  if (bool_from_obj(r[3])) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(204);
    r[4] = (hendblk(1+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cx__231362);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 204: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(205);
    r[3] = (hendblk(1+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cx_code_2Druntime);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 205: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(206);
    r[3] = (hendblk(1+1));
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    r[2] = (cx__231360);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 206: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[2];  
    r[2] = (cx_code_2Dmain);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 207: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  add-nl sfcs k */
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(210);
    r[5] = (hendblk(1+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v2998;

s_loop_v2998: /* k id add-nl */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(208);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v2998;
  }

case 208: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r add-nl id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(209);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 209: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 210: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 211: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  add-nl defs k */
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(214);
    r[5] = (hendblk(1+1));
    r[0] = r[5];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v2963;

s_loop_v2963: /* k id add-nl */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(212);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v2963;
  }

case 212: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r add-nl id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(213);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 213: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 214: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 215: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  incs k */
    r[0] = (cx_display_2Dtext);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 216: /* main k argv */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k argv */
    hreserve(hbsz(1), 2); /* 2 live regs */
    *--hp = obj_from_void(0);
    r[2] = (hendblk(1));
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(217);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = obj_from_case(219);
    r[3] = (hendblk(3+1));
    r[4+0] = r[3];  
    r[4+1] = (mknull());
    r[4+2] = obj_from_fixnum(0);
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v2914;

case 217: /* clo k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
    /* k */
    hreserve(hbsz(1+1), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(218);
    r[1] = (hendblk(1+1));
    r[2+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = r[1];  
    r[2+2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[2+3] = (cx__231472);
    r[2+4] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 218: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v2914: /* k r i argv */
    { const fixnum_t v4332_i = fixnum_from_obj(r[2]);
    { /* argv-ref */
    int i = (v4332_i);
    char *s = ((char **)((r[3])))[i];
    if (s) r[4] = (hpushstr(4, newstring(s)));
    else r[4] = obj_from_bool(0); }
  if (bool_from_obj(r[4])) {
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[5];  
    r[2] = obj_from_fixnum((v4332_i) + (1));
    /* r[3] */    
    goto s_loop_v2914;
  } else {
    { /* reverse! */
    obj t, v = mknull(), l = r[1];  
    while (l != mknull()) t = cdr(l), cdr(l) = v, v = l, l = t;
    r[5] = (v); }
    r[6+0] = r[0];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = obj_from_ktrap();
    r[6+2] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } } 

case 219: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r about k args */
    r[5] = (cdr((r[1])));
    (void)(objptr_from_obj(r[4])[0] = (r[5]));
    r[6] = (objptr_from_obj(r[4])[0]);
    r[6] = obj_from_bool(isnull((r[6])));
  if (bool_from_obj(r[6])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  } else {
    r[6] = (objptr_from_obj(r[4])[0]);
    r[0] = r[3];  
    r[1] = r[6];  
    /* r[2] */    
    goto s_loop_v2875;
  }

case 220: /* clo k args */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v2875: /* k args about */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(220);
    r[3] = (hendblk(1+1));
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (car((r[1])));
    r[4] = obj_from_bool(strcmp(stringchars((r[4])), stringchars((cx__231509))) == 0);
  if (bool_from_obj(r[4])) {
    cx__2Ashowlog_2A = obj_from_bool(1);
    r[4] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v2875;
  } else {
    r[4] = (car((r[1])));
    r[4] = obj_from_bool(strcmp(stringchars((r[4])), stringchars((cx__231503))) == 0);
  if (bool_from_obj(r[4])) {
    cx__2Asilent_2A = obj_from_bool(0);
    r[4] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v2875;
  } else {
    r[4] = (car((r[1])));
    r[4] = obj_from_bool(strcmp(stringchars((r[4])), stringchars((cx__231497))) == 0);
  if (bool_from_obj(r[4])) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(221);
    r[4] = (hendblk(1+1));
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  } else {
    { bool_t v4331_tmp;
    r[4] = (car((r[1])));
    { /* file-exists? */ 
    FILE *f = fopen(stringchars((r[4])), "r");
    if (f != NULL) fclose(f);
    v4331_tmp = (f != NULL); }
    r[4] = obj_from_bool(!(v4331_tmp)); }
  if (bool_from_obj(r[4])) {
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(222);
    r[4] = (hendblk(1+1));
    r[5] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[6+3] = (cx__231491);
    r[6+4] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(223);
    r[4] = (hendblk(3+1));
  if (bool_from_obj(cx__2Ashowlog_2A)) {
    r[5] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[6+3] = (cx__231485);
    r[6+4] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[5+0] = obj_from_ktrap();
    r[5+1] = obj_from_void(0);
    r[5+2] = r[3];  
    r[5+3] = r[1];  
    r[5+4] = r[0];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v2877;
  }
  }
  }
  }
  }
  }

case 221: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 222: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek  k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 223: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v2877: /* ek  loop args k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(224);
    r[5] = (hendblk(3+1));
    r[0] = r[5];  
    r[1] = (car((r[3])));
    goto gs_compile_2Dfile;

case 224: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop args k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(225);
    r[5] = (hendblk(3+1));
  if (bool_from_obj(cx__2Ashowlog_2A)) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[7+0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r[7+3] = (cx__231481);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_void(0);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto s_l_v2881;
  }

case 225: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
s_l_v2881: /* ek  loop args k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

default: /* inter-host call */
    cxg_hp = hp;
    cxm_rgc(r, r + MAX_LIVEREGS);
#ifndef NDEBUG
    cxg_rc = rc;
#endif
    return pc;
  }
}

/* module load */
void MODULE(void)
{
  obj pc;
  if (!root.next) {
    root.next = cxg_rootp;
    cxg_rootp = &root;
    LOAD();
    pc = obj_from_case(0);
    assert((cxg_rc = 0, 1));
    while (pc) pc = (*(cxhost_t*)pc)(pc); 
    assert(cxg_rc == 2);
  }
}

/* basic runtime */
#define HEAP_SIZE 131072 /* 2^17 */

obj *cxg_heap = NULL;
cxoint_t cxg_hmask = 0;
obj *cxg_hp = NULL;
cxroot_t *cxg_rootp = NULL;
obj cxg_regs[REGS_SIZE];
#ifndef NDEBUG
int cxg_rc = 0;
#endif

static obj *heap2 = NULL;
static size_t hsize = 0; 
static cxoint_t hmask2 = 0;
static int gccount = 0, bumpcount = 0;

#define notfwdptr(o) (((char*)(o) - (char*)heap2) & hmask2)

static void toheap2(obj* p)
{
  obj o = *p, *op, fo, *fop;
  if (notobjptr(o)) return;
  fo = (op = objptr_from_obj(o))[-1]; assert(fo);
  if (notaptr(fo)) {
    fop = op + size_from_obj(fo); while (fop >= op) *--cxg_hp = *--fop;
    *p = *fop = obj_from_objptr(cxg_hp+1);
  } else if (notfwdptr(fo)) {
    *--cxg_hp = *op--; *--cxg_hp = *op;
    *p = *op = obj_from_objptr(cxg_hp+1);
  } else *p = fo;
}

static void finalize(obj *hp, obj *end)
{
  while (hp < end) {
    obj fo = *hp++; assert(fo);
    if (notaptr(fo)) hp += size_from_obj(fo);
    else if (notfwdptr(fo)) ((cxtype_t*)fo)->free((void*)*hp++);
    else if (notaptr(fo = objptr_from_obj(fo)[-1])) hp += size_from_obj(fo);
    else ++hp;
  } assert(hp == end);
}

static size_t relocate(obj *regs, obj *regp, obj *end2, obj *end1) 
{
  obj *p, *hp1 = cxg_hp; cxroot_t *pr; 
  cxg_hp = end2; if (!cxg_heap) return 0;
  for (p = regs; p < regp; ++p) toheap2(p);
  for (pr = cxg_rootp; pr; pr = pr->next) {
    obj **pp = pr->globv; int c = pr->globc;
    while (c-- > 0) toheap2(*pp++);
  }
  for (p = end2; p > cxg_hp; --p) toheap2(p-1);
  if (end1) finalize(hp1, end1);
  return end2 - cxg_hp;
}

obj *cxm_hgc(obj *regs, obj *regp, obj *hp, size_t needs) 
{
  obj *p = cxg_heap; cxg_hp = hp; ++gccount;
  if (hsize < (needs += relocate(regs, regp, heap2 + hsize, p + hsize))) {
    size_t s = HEAP_SIZE; while (s < needs) s *= 2;
    hmask2 = 1 | ~(s*sizeof(obj)-1);
    if (!(p = realloc(cxg_heap, s*sizeof(obj)))) { perror("alloc[h]"); exit(2); }
    cxg_heap = heap2; heap2 = p; relocate(regs, regp, heap2 + s, 0);
    if (!(p = realloc(cxg_heap, s*sizeof(obj)))) { perror("alloc[h]"); exit(2); }
    hsize = s; cxg_hmask = hmask2; ++bumpcount;
  }
  cxg_heap = heap2; heap2 = p;
  return cxg_hp;
}

obj *cxm_rgc(obj *regs, obj *regp) 
{
  obj *p = cxg_regs;
  assert(regp <= cxg_regs + REGS_SIZE);
  while (regs < regp) *p++ = *regs++;
  return cxg_regs;
}

void *cxm_cknull(void *p, char *msg)
{
  if (!p) { 
    perror(msg); exit(2); 
  }
  return p;
}

/* os entry point */
int main(int argc, char **argv) {
  int res; obj pc;
  obj retcl[1] = { 0 }; 
  MODULE();
  cxg_regs[0] = cx_main;
  cxg_regs[1] = (obj)retcl;
  cxg_regs[2] = (obj)argv;
  assert(cxg_rc = 3);
  pc = objptr_from_obj(cx_main)[0];
  while (pc) pc = (*(cxhost_t*)pc)(pc); 
  assert(cxg_rc == 3);
  res = (cxg_regs[2] != 0); 
  /* fprintf(stderr, "%d collections, %d reallocs\n", gccount, bumpcount); */
  return res; 
}

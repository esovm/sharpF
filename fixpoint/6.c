/* 6.sf */
#define MODULE module_6
#define LOAD() module_3(); module_0(); 
extern void module_0(void); /* 0.sf */
extern void module_3(void); /* 3.sf */

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
extern obj cx_beta_2Dsubstitute; /* beta-substitute */
extern obj cx_c_2Derror_2A; /* c-error* */
extern obj cx_fixnum_2D_3Estring; /* fixnum->string */
extern obj cx_flonum_2D_3Estring; /* flonum->string */
extern obj cx_fprintf_2A; /* fprintf* */
extern obj cx_list_3F; /* list? */
extern obj cx_reset; /* reset */
extern obj cx_string_2Dappend_2A; /* string-append* */
extern obj cx_timestamp; /* timestamp */
extern obj cx_var_2Dreferenced_2Din_2Dexp_3F; /* var-referenced-in-exp? */
extern obj cx_write_2F3; /* write/3 */
obj cx_c_2Dargref_2Dctype; /* c-argref-ctype */
obj cx_c_2Dformat_2A; /* c-format* */
obj cx_c_2Dformat_2Dprim_2A; /* c-format-prim* */
obj cx_c_2Dformat_2Dprim_2Dtext_2A; /* c-format-prim-text* */
obj cx_c_2Dformat_2Dprimexp_2A; /* c-format-primexp* */
obj cx_c_2Dmangle; /* c-mangle */
obj cx_c_2Dstringify_2Dtext; /* c-stringify-text */
obj cx_c_2Dundecorate_2Dalvar; /* c-undecorate-alvar */
obj cx_cleanup_2Dc_2Dcode_21; /* cleanup-c-code! */
obj cx_code_2Dhost_2Depilogue; /* code-host-epilogue */
obj cx_code_2Dhost_2Dprologue; /* code-host-prologue */
obj cx_code_2Dmain; /* code-main */
obj cx_code_2Dmodule; /* code-module */
obj cx_code_2Druntime; /* code-runtime */
obj cx_code_2Dstandard_2Ddefinitions; /* code-standard-definitions */
obj cx_code_2Dstandard_2Dincludes; /* code-standard-includes */
obj cx_display_2Dtext; /* display-text */
obj cx_path_2Dstrip_2Ddirectory; /* path-strip-directory */
obj cx_path_2Dstrip_2Dextension; /* path-strip-extension */
obj cx_prim_2Dcexp_3F; /* prim-cexp? */
obj cx_prim_2Dctype; /* prim-ctype */
obj cx_primexp_2Dctype; /* primexp-ctype */
obj cx_stack_2Dfunction_2Dcode_2Dgenerate; /* stack-function-code-generate */
obj cx_stack_2Dfunctions_2Dcode_2Dgenerate; /* stack-functions-code-generate */
obj cx_string_2Dcontains_2Dat_3F; /* string-contains-at? */
obj cx_string_2Dcontains_3F; /* string-contains? */
obj cx_string_2Dends_2Dwith_3F; /* string-ends-with? */
obj cx_string_2Dmember_2Dci_3F; /* string-member-ci? */
obj cx_string_2Dmember_3F; /* string-member? */
obj cx_string_2Dreverse; /* string-reverse */
obj cx_string_2Dspan; /* string-span */
obj cx_string_2Dspan_2Dci; /* string-span-ci */
obj cx_string_2Dstarts_2Dwith_3F; /* string-starts-with? */
obj cx_string_2Dupcase; /* string-upcase */
obj cx_substring_2Dafter; /* substring-after */
obj cx_substring_2Dafter_2Dprefix; /* substring-after-prefix */
obj cx_substring_2Dafter_2Dspan; /* substring-after-span */
obj cx_substring_2Dbefore; /* substring-before */
obj cx_substring_2Dbefore_2Dsuffix; /* substring-before-suffix */
obj cx_typeassert_2Dprim_2Dctype; /* typeassert-prim-ctype */
obj cx_typecheck_2Dprim_2Dctype; /* typecheck-prim-ctype */
static obj cx__231001; /* constant #1001 */
static obj cx__231003; /* constant #1003 */
static obj cx__231005; /* constant #1005 */
static obj cx__231007; /* constant #1007 */
static obj cx__231009; /* constant #1009 */
static obj cx__231011; /* constant #1011 */
static obj cx__231037; /* constant #1037 */
static obj cx__231040; /* constant #1040 */
static obj cx__231042; /* constant #1042 */
static obj cx__231046; /* constant #1046 */
static obj cx__231070; /* constant #1070 */
static obj cx__231075; /* constant #1075 */
static obj cx__231078; /* constant #1078 */
static obj cx__231083; /* constant #1083 */
static obj cx__231097; /* constant #1097 */
static obj cx__231114; /* constant #1114 */
static obj cx__231117; /* constant #1117 */
static obj cx__231203; /* constant #1203 */
static obj cx__231206; /* constant #1206 */
static obj cx__231262; /* constant #1262 */
static obj cx__231265; /* constant #1265 */
static obj cx__231270; /* constant #1270 */
static obj cx__231284; /* constant #1284 */
static obj cx__231292; /* constant #1292 */
static obj cx__231308; /* constant #1308 */
static obj cx__231354; /* constant #1354 */
static obj cx__231371; /* constant #1371 */
static obj cx__231525; /* constant #1525 */
static obj cx__231572; /* constant #1572 */
static obj cx__231624; /* constant #1624 */
static obj cx__231718; /* constant #1718 */
static obj cx__231725; /* constant #1725 */
static obj cx__231861; /* constant #1861 */
static obj cx__231936; /* constant #1936 */
static obj cx__231952; /* constant #1952 */
static obj cx__231962; /* constant #1962 */
static obj cx__231991; /* constant #1991 */
static obj cx__232020; /* constant #2020 */
static obj cx__232032; /* constant #2032 */
static obj cx__23208; /* constant #208 */
static obj cx__23210; /* constant #210 */
static obj cx__23214; /* constant #214 */
static obj cx__23216; /* constant #216 */
static obj cx__23224; /* constant #224 */
static obj cx__23226; /* constant #226 */
static obj cx__23231; /* constant #231 */
static obj cx__23233; /* constant #233 */
static obj cx__232379; /* constant #2379 */
static obj cx__232397; /* constant #2397 */
static obj cx__232434; /* constant #2434 */
static obj cx__232444; /* constant #2444 */
static obj cx__232449; /* constant #2449 */
static obj cx__23248; /* constant #248 */
static obj cx__232480; /* constant #2480 */
static obj cx__232487; /* constant #2487 */
static obj cx__232493; /* constant #2493 */
static obj cx__232500; /* constant #2500 */
static obj cx__232509; /* constant #2509 */
static obj cx__232522; /* constant #2522 */
static obj cx__232525; /* constant #2525 */
static obj cx__232536; /* constant #2536 */
static obj cx__232550; /* constant #2550 */
static obj cx__232553; /* constant #2553 */
static obj cx__232561; /* constant #2561 */
static obj cx__232564; /* constant #2564 */
static obj cx__232572; /* constant #2572 */
static obj cx__23259; /* constant #259 */
static obj cx__23274; /* constant #274 */
static obj cx__23289; /* constant #289 */
static obj cx__23300; /* constant #300 */
static obj cx__23309; /* constant #309 */
static obj cx__23314; /* constant #314 */
static obj cx__23325; /* constant #325 */
static obj cx__23344; /* constant #344 */
static obj cx__23351; /* constant #351 */
static obj cx__23361; /* constant #361 */
static obj cx__23401; /* constant #401 */
static obj cx__23420; /* constant #420 */
static obj cx__23428; /* constant #428 */
static obj cx__23447; /* constant #447 */
static obj cx__23448; /* constant #448 */
static obj cx__23456; /* constant #456 */
static obj cx__23539; /* constant #539 */
static obj cx__23584; /* constant #584 */
static obj cx__23591; /* constant #591 */
static obj cx__23609; /* constant #609 */
static obj cx__23662; /* constant #662 */
static obj cx__23666; /* constant #666 */
static obj cx__23667; /* constant #667 */
static obj cx__23683; /* constant #683 */
static obj cx__23690; /* constant #690 */
static obj cx__23731; /* constant #731 */
static obj cx__23733; /* constant #733 */
static obj cx__23734; /* constant #734 */
static obj cx__23735; /* constant #735 */
static obj cx__23736; /* constant #736 */
static obj cx__23761; /* constant #761 */
static obj cx__23777; /* constant #777 */
static obj cx__23999; /* constant #999 */
static obj cx_begin_2Dexp_3F_231627; /* constant begin-exp?#1627 */
static obj cx_intersectionq_231133; /* constant intersectionq#1133 */

/* helper functions */
/* rec-exp?#1443 */
static obj cxs_rec_2Dexp_3F_231443(obj v1445_exp)
{ 
  if (bool_from_obj((isvector((v1445_exp))) ? (((vectorlen((v1445_exp))) == (4)) ? obj_from_bool((vectorref((v1445_exp), (0))) == (mksymbol(internsym("fix-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v1452_body = (vectorref((v1445_exp), (3)));
    obj v1453_ids = (vectorref((v1445_exp), (1)));
    return ((ispair((v1453_ids))) ? ((isnull((cdr((v1453_ids))))) ? (bool_from_obj((isvector((v1452_body))) ? (((vectorlen((v1452_body))) == (2)) ? obj_from_bool((vectorref((v1452_body), (0))) == (mksymbol(internsym("var-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool((car((v1453_ids))) == (vectorref((v1452_body), (1)))) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* loop-exp?#1437 */
static obj cxs_loop_2Dexp_3F_231437(obj v1439_exp)
{ 
    return (bool_from_obj((isvector((v1439_exp))) ? (((vectorlen((v1439_exp))) == (3)) ? obj_from_bool((vectorref((v1439_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? (cxs_rec_2Dexp_3F_231443((vectorref((v1439_exp), (1))))) : obj_from_bool(0));
}

/* let-exp?#1575 */
static obj cxs_let_2Dexp_3F_231575(obj v1577_exp)
{ 
  if (bool_from_obj((isvector((v1577_exp))) ? (((vectorlen((v1577_exp))) == (3)) ? obj_from_bool((vectorref((v1577_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v1581_rator = (vectorref((v1577_exp), (1)));
  if (bool_from_obj((isvector((v1581_rator))) ? (((vectorlen((v1581_rator))) == (3)) ? obj_from_bool((vectorref((v1581_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v2864_tmp;
    obj v2863_tmp;
    { /* length */
    int n; obj l = (vectorref((v1577_exp), (2)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2864_tmp = obj_from_fixnum(n); };
    { /* length */
    int n; obj l = (vectorref((v1581_rator), (1)));
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v2863_tmp = obj_from_fixnum(n); };
    return obj_from_bool(fixnum_from_obj(v2863_tmp) == fixnum_from_obj(v2864_tmp));
  }
  } else {
    return obj_from_bool(0);
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp->body#1735 */
static obj cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231735(obj v1737_exp)
{ 
    return ((isnull((vectorref((v1737_exp), (2))))) ? (vectorref((vectorref((v1737_exp), (1))), (2))) : (car((vectorref((v1737_exp), (2))))));
}

/* null-let-exp?#1761 */
static obj cxs_null_2Dlet_2Dexp_3F_231761(obj v1763_exp)
{ 
  if (bool_from_obj((isvector((v1763_exp))) ? (((vectorlen((v1763_exp))) == (3)) ? obj_from_bool((vectorref((v1763_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if ((isnull((vectorref((v1763_exp), (2)))))) {
  { /* let */
    obj v1767_rator = (vectorref((v1763_exp), (1)));
    return (bool_from_obj((isvector((v1767_rator))) ? (((vectorlen((v1767_rator))) == (3)) ? obj_from_bool((vectorref((v1767_rator), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool(isnull((vectorref((v1767_rator), (1))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-lambda-exp?#1807 */
static obj cxs_identity_2Dlambda_2Dexp_3F_231807(obj v1809_exp)
{ 
  if (bool_from_obj((isvector((v1809_exp))) ? (((vectorlen((v1809_exp))) == (3)) ? obj_from_bool((vectorref((v1809_exp), (0))) == (mksymbol(internsym("lambda-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  { /* let */
    obj v1816_body = (vectorref((v1809_exp), (2)));
    obj v1817_ids = (vectorref((v1809_exp), (1)));
    return ((ispair((v1817_ids))) ? ((isnull((cdr((v1817_ids))))) ? (bool_from_obj((isvector((v1816_body))) ? (((vectorlen((v1816_body))) == (2)) ? obj_from_bool((vectorref((v1816_body), (0))) == (mksymbol(internsym("var-exp")))) : obj_from_bool(0)) : obj_from_bool(0)) ? obj_from_bool((car((v1817_ids))) == (vectorref((v1816_body), (1)))) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
}

/* identity-let-exp?#1794 */
static obj cxs_identity_2Dlet_2Dexp_3F_231794(obj v1796_exp)
{ 
  if (bool_from_obj((isvector((v1796_exp))) ? (((vectorlen((v1796_exp))) == (3)) ? obj_from_bool((vectorref((v1796_exp), (0))) == (mksymbol(internsym("app-exp")))) : obj_from_bool(0)) : obj_from_bool(0))) {
  if (bool_from_obj(cxs_identity_2Dlambda_2Dexp_3F_231807((vectorref((v1796_exp), (1)))))) {
  { /* let */
    obj v1800_rands = (vectorref((v1796_exp), (2)));
    return ((ispair((v1800_rands))) ? obj_from_bool(isnull((cdr((v1800_rands))))) : obj_from_bool(0));
  }
  } else {
    return obj_from_bool(0);
  }
  } else {
    return obj_from_bool(0);
  }
}

/* degenerate-let-exp?#1758 */
static obj cxs_degenerate_2Dlet_2Dexp_3F_231758(obj v1760_exp)
{ 
  { /* let */
    obj v1793_x = (cxs_null_2Dlet_2Dexp_3F_231761((v1760_exp)));
    return (bool_from_obj(v1793_x) ? (v1793_x) : (cxs_identity_2Dlet_2Dexp_3F_231794((v1760_exp))));
  }
}

/* string-starts-with? */
static obj cxs_string_2Dstarts_2Dwith_3F(obj v2_str, obj v1_sstr)
{ 
  if (((stringlen((v1_sstr))) <= (stringlen((v2_str))))) {
  { /* letrec */
    obj v8_i;
  { /* let */
    obj v2862_tmp = obj_from_fixnum(0);
    /* tail call */
    v8_i = (v2862_tmp);
    goto s_loop;
  }
  s_loop:
  { /* let */
    obj v9_x = obj_from_bool(fixnum_from_obj(v8_i) >= (stringlen((v1_sstr))));
  if (bool_from_obj(v9_x)) {
    return (v9_x);
  } else {
  if (((*stringref((v2_str), fixnum_from_obj(v8_i))) == (*stringref((v1_sstr), fixnum_from_obj(v8_i))))) {
  { /* let */
    obj v2861_tmp = obj_from_fixnum(fixnum_from_obj(v8_i) + (1));
    /* tail call */
    v8_i = (v2861_tmp);
    goto s_loop;
  }
  } else {
    return obj_from_bool(0);
  }
  }
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* string-ends-with? */
static obj cxs_string_2Dends_2Dwith_3F(obj v21_str, obj v20_sstr)
{ 
  { /* let */
    obj v24_ssl = obj_from_fixnum(stringlen((v20_sstr)));
  if ((fixnum_from_obj(v24_ssl) <= (stringlen((v21_str))))) {
  { /* letrec */
    obj v27_i;
  { /* let */
    obj v2860_tmp = obj_from_fixnum(0);
    /* tail call */
    v27_i = (v2860_tmp);
    goto s_loop;
  }
  s_loop:
  if ((fixnum_from_obj(v27_i) >= fixnum_from_obj(v24_ssl))) {
    return obj_from_bool(fixnum_from_obj(v27_i) >= fixnum_from_obj(v24_ssl));
  } else {
  if (((*stringref((v21_str), ((stringlen((v21_str))) - (fixnum_from_obj(v27_i) + (1))))) == (*stringref((v20_sstr), (fixnum_from_obj(v24_ssl) - (fixnum_from_obj(v27_i) + (1))))))) {
  { /* let */
    obj v2859_tmp = obj_from_fixnum(fixnum_from_obj(v27_i) + (1));
    /* tail call */
    v27_i = (v2859_tmp);
    goto s_loop;
  }
  } else {
    return obj_from_bool(0);
  }
  }
  }
  } else {
    return obj_from_bool(0);
  }
  }
}

/* string-contains-at? */
static obj cxs_string_2Dcontains_2Dat_3F(obj v41_str, obj v40_pos, obj v39_sstr)
{ 
  if (((stringlen((v39_sstr))) <= ((stringlen((v41_str))) - fixnum_from_obj(v40_pos)))) {
  { /* letrec */
    obj v47_i;
  { /* let */
    obj v2858_tmp = obj_from_fixnum(0);
    /* tail call */
    v47_i = (v2858_tmp);
    goto s_loop;
  }
  s_loop:
  { /* let */
    obj v48_x = obj_from_bool(fixnum_from_obj(v47_i) >= (stringlen((v39_sstr))));
  if (bool_from_obj(v48_x)) {
    return (v48_x);
  } else {
  if (((*stringref((v41_str), (fixnum_from_obj(v40_pos) + fixnum_from_obj(v47_i)))) == (*stringref((v39_sstr), fixnum_from_obj(v47_i))))) {
  { /* let */
    obj v2857_tmp = obj_from_fixnum(fixnum_from_obj(v47_i) + (1));
    /* tail call */
    v47_i = (v2857_tmp);
    goto s_loop;
  }
  } else {
    return obj_from_bool(0);
  }
  }
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* string-contains? */
static obj cxs_string_2Dcontains_3F(obj v60_str, obj v59_sstr)
{ 
  if (((stringlen((v59_sstr))) <= (stringlen((v60_str))))) {
  { /* letrec */
    obj v66_pos;
  { /* let */
    obj v2856_tmp = obj_from_fixnum(0);
    /* tail call */
    v66_pos = (v2856_tmp);
    goto s_loop;
  }
  s_loop:
  if (((stringlen((v59_sstr))) <= ((stringlen((v60_str))) - fixnum_from_obj(v66_pos)))) {
  { /* let */
    obj v67_x = (cxs_string_2Dcontains_2Dat_3F((v60_str), (v66_pos), (v59_sstr)));
  if (bool_from_obj(v67_x)) {
    return (v67_x);
  } else {
  { /* let */
    obj v2855_tmp = obj_from_fixnum(fixnum_from_obj(v66_pos) + (1));
    /* tail call */
    v66_pos = (v2855_tmp);
    goto s_loop;
  }
  }
  }
  } else {
    return obj_from_bool(0);
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* string-member? */
static obj cxs_string_2Dmember_3F(obj v119_c, obj v118_str)
{ 
  { /* letrec */
    obj v123_pos;
    obj v122_end;
  { /* let */
    obj v2854_tmp = obj_from_fixnum(stringlen((v118_str)));
    obj v2853_tmp = obj_from_fixnum(0);
    /* tail call */
    v123_pos = (v2853_tmp);
    v122_end = (v2854_tmp);
    goto s_lookup;
  }
  s_lookup:
  if ((fixnum_from_obj(v123_pos) < fixnum_from_obj(v122_end))) {
  { /* let */
    obj v128_x = obj_from_bool(char_from_obj(v119_c) == (*stringref((v118_str), fixnum_from_obj(v123_pos))));
  if (bool_from_obj(v128_x)) {
    return (v128_x);
  } else {
  { /* let */
    obj v2852_tmp = (v122_end);
    obj v2851_tmp = obj_from_fixnum((1) + fixnum_from_obj(v123_pos));
    /* tail call */
    v123_pos = (v2851_tmp);
    v122_end = (v2852_tmp);
    goto s_lookup;
  }
  }
  }
  } else {
    return obj_from_bool(0);
  }
  }
}

/* string-member-ci? */
static obj cxs_string_2Dmember_2Dci_3F(obj v134_c, obj v133_str)
{ 
  { /* letrec */
    obj v138_pos;
    obj v137_end;
  { /* let */
    obj v2850_tmp = obj_from_fixnum(stringlen((v133_str)));
    obj v2849_tmp = obj_from_fixnum(0);
    /* tail call */
    v138_pos = (v2849_tmp);
    v137_end = (v2850_tmp);
    goto s_lookup;
  }
  s_lookup:
  if ((fixnum_from_obj(v138_pos) < fixnum_from_obj(v137_end))) {
  { /* let */
    obj v143_x = obj_from_bool(tolower(char_from_obj(v134_c)) == tolower((*stringref((v133_str), fixnum_from_obj(v138_pos)))));
  if (bool_from_obj(v143_x)) {
    return (v143_x);
  } else {
  { /* let */
    obj v2848_tmp = (v137_end);
    obj v2847_tmp = obj_from_fixnum((1) + fixnum_from_obj(v138_pos));
    /* tail call */
    v138_pos = (v2847_tmp);
    v137_end = (v2848_tmp);
    goto s_lookup;
  }
  }
  }
  } else {
    return obj_from_bool(0);
  }
  }
}

/* string-span */
static obj cxs_string_2Dspan(obj v149_str, obj v148_cs)
{ 
  { /* letrec */
    obj v153_pos;
    obj v152_end;
  { /* let */
    obj v2846_tmp = obj_from_fixnum(stringlen((v149_str)));
    obj v2845_tmp = obj_from_fixnum(0);
    /* tail call */
    v153_pos = (v2845_tmp);
    v152_end = (v2846_tmp);
    goto s_loop;
  }
  s_loop:
  if ((fixnum_from_obj(v153_pos) >= fixnum_from_obj(v152_end))) {
    return (v153_pos);
  } else {
  if (bool_from_obj(cxs_string_2Dmember_3F(obj_from_char(*stringref((v149_str), fixnum_from_obj(v153_pos))), (v148_cs)))) {
  { /* let */
    obj v2844_tmp = (v152_end);
    obj v2843_tmp = obj_from_fixnum((1) + fixnum_from_obj(v153_pos));
    /* tail call */
    v153_pos = (v2843_tmp);
    v152_end = (v2844_tmp);
    goto s_loop;
  }
  } else {
    return (v153_pos);
  }
  }
  }
}

/* string-span-ci */
static obj cxs_string_2Dspan_2Dci(obj v161_str, obj v160_cs)
{ 
  { /* letrec */
    obj v165_pos;
    obj v164_end;
  { /* let */
    obj v2842_tmp = obj_from_fixnum(stringlen((v161_str)));
    obj v2841_tmp = obj_from_fixnum(0);
    /* tail call */
    v165_pos = (v2841_tmp);
    v164_end = (v2842_tmp);
    goto s_loop;
  }
  s_loop:
  if ((fixnum_from_obj(v165_pos) >= fixnum_from_obj(v164_end))) {
    return (v165_pos);
  } else {
  if (bool_from_obj(cxs_string_2Dmember_2Dci_3F(obj_from_char(*stringref((v161_str), fixnum_from_obj(v165_pos))), (v160_cs)))) {
  { /* let */
    obj v2840_tmp = (v164_end);
    obj v2839_tmp = obj_from_fixnum((1) + fixnum_from_obj(v165_pos));
    /* tail call */
    v165_pos = (v2839_tmp);
    v164_end = (v2840_tmp);
    goto s_loop;
  }
  } else {
    return (v165_pos);
  }
  }
  }
}

/* cleanup-c-code! */
static obj cxs_cleanup_2Dc_2Dcode_21(obj v809_str)
{ 
  { /* let */
    obj v811_len = obj_from_fixnum(stringlen((v809_str)));
  { /* letrec */
    obj v813_i;
  { /* let */
    obj v2838_tmp = obj_from_fixnum(0);
    /* tail call */
    v813_i = (v2838_tmp);
    goto s_loop;
  }
  s_loop:
  if ((fixnum_from_obj(v813_i) >= fixnum_from_obj(v811_len))) {
    return (v809_str);
  } else {
  { /* let */
    obj v816_c = obj_from_char(*stringref((v809_str), fixnum_from_obj(v813_i)));
  if ((char_from_obj(v816_c) == (34))) {
    return (v809_str);
  } else {
  if (bool_from_obj(((fixnum_from_obj(v813_i) + (8)) < fixnum_from_obj(v811_len)) ? ((char_from_obj(v816_c) == (61)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (1)))) == (32)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (2)))) == (40)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (3)))) == (114)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (4)))) == (91)) ? ((isdigit((*stringref((v809_str), (fixnum_from_obj(v813_i) + (5)))))) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (6)))) == (93)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (7)))) == (41)) ? obj_from_bool((*stringref((v809_str), (fixnum_from_obj(v813_i) + (8)))) == (59)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0))) {
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (2))) = (114));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (3))) = (91));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (4))) = (*stringref((v809_str), (fixnum_from_obj(v813_i) + (5)))));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (5))) = (93));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (6))) = (59));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (7))) = (32));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (8))) = (32));
  { /* let */
    obj v2837_tmp = obj_from_fixnum(fixnum_from_obj(v813_i) + (9));
    /* tail call */
    v813_i = (v2837_tmp);
    goto s_loop;
  }
  } else {
  if (bool_from_obj(((fixnum_from_obj(v813_i) + (13)) < fixnum_from_obj(v811_len)) ? ((char_from_obj(v816_c) == (114)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (1)))) == (91)) ? ((isdigit((*stringref((v809_str), (fixnum_from_obj(v813_i) + (2)))))) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (3)))) == (93)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (4)))) == (32)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (5)))) == (61)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (6)))) == (32)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (7)))) == (40)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (8)))) == (114)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (9)))) == (91)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (10)))) == (*stringref((v809_str), (fixnum_from_obj(v813_i) + (2))))) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (11)))) == (93)) ? (((*stringref((v809_str), (fixnum_from_obj(v813_i) + (12)))) == (41)) ? obj_from_bool((*stringref((v809_str), (fixnum_from_obj(v813_i) + (13)))) == (59)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0)) : obj_from_bool(0))) {
    (void) obj_from_void(*stringref((v809_str), fixnum_from_obj(v813_i)) = (47));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (1))) = (42));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (2))) = (32));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (3))) = (114));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (4))) = (91));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (5))) = (*stringref((v809_str), (fixnum_from_obj(v813_i) + (10)))));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (6))) = (93));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (7))) = (32));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (8))) = (42));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (9))) = (47));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (10))) = (32));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (11))) = (32));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (12))) = (32));
    (void) obj_from_void(*stringref((v809_str), (fixnum_from_obj(v813_i) + (13))) = (32));
  { /* let */
    obj v2836_tmp = obj_from_fixnum(fixnum_from_obj(v813_i) + (14));
    /* tail call */
    v813_i = (v2836_tmp);
    goto s_loop;
  }
  } else {
  { /* let */
    obj v2835_tmp = obj_from_fixnum(fixnum_from_obj(v813_i) + (1));
    /* tail call */
    v813_i = (v2835_tmp);
    goto s_loop;
  }
  }
  }
  }
  }
  }
  }
  }
}

/* gc roots */
static obj *globv[] = {
  &cx_code_2Dhost_2Depilogue,
  &cx_code_2Dhost_2Dprologue,
  &cx_code_2Dmain,
  &cx_code_2Dmodule,
  &cx_code_2Druntime,
  &cx_code_2Dstandard_2Ddefinitions,
  &cx_code_2Dstandard_2Dincludes,
  &cx__231001,
  &cx__231003,
  &cx__231005,
  &cx__231007,
  &cx__231009,
  &cx__231011,
  &cx__231037,
  &cx__231040,
  &cx__231042,
  &cx__231046,
  &cx__231070,
  &cx__231075,
  &cx__231078,
  &cx__231083,
  &cx__231097,
  &cx__231114,
  &cx__231117,
  &cx__231203,
  &cx__231206,
  &cx__231262,
  &cx__231265,
  &cx__231270,
  &cx__231284,
  &cx__231292,
  &cx__231308,
  &cx__231354,
  &cx__231371,
  &cx__231525,
  &cx__231572,
  &cx__231624,
  &cx__231718,
  &cx__231725,
  &cx__231861,
  &cx__231936,
  &cx__231952,
  &cx__231962,
  &cx__231991,
  &cx__232020,
  &cx__232032,
  &cx__23208,
  &cx__23210,
  &cx__23214,
  &cx__23216,
  &cx__23224,
  &cx__23226,
  &cx__23231,
  &cx__23233,
  &cx__232379,
  &cx__232397,
  &cx__232434,
  &cx__232444,
  &cx__232449,
  &cx__23248,
  &cx__232480,
  &cx__232487,
  &cx__232493,
  &cx__232500,
  &cx__232509,
  &cx__232522,
  &cx__232525,
  &cx__232536,
  &cx__232550,
  &cx__232553,
  &cx__232561,
  &cx__232564,
  &cx__232572,
  &cx__23259,
  &cx__23274,
  &cx__23289,
  &cx__23300,
  &cx__23309,
  &cx__23314,
  &cx__23325,
  &cx__23344,
  &cx__23351,
  &cx__23361,
  &cx__23401,
  &cx__23420,
  &cx__23428,
  &cx__23447,
  &cx__23448,
  &cx__23456,
  &cx__23539,
  &cx__23584,
  &cx__23591,
  &cx__23609,
  &cx__23662,
  &cx__23666,
  &cx__23667,
  &cx__23683,
  &cx__23690,
  &cx__23731,
  &cx__23733,
  &cx__23734,
  &cx__23735,
  &cx__23736,
  &cx__23761,
  &cx__23777,
  &cx__23999,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[246] = {
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
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 35
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
    cx__23208 = (hpushstr(0, newstring("(")));
    cx__23210 = (hpushstr(0, newstring("abcdefghijklmnopqrstuvwxyz0123456789_")));
    { static char s[] = { 36, 114, 101, 116, 117, 114, 110, 0 };
    cx__23214 = (hpushstr(0, newstring(s))); }
    cx__23216 = (hpushstr(0, newstring(" ")));
    cx__23224 = (hpushstr(0, newstring("bool(is_")));
    { static char s[] = { 95, 36, 97, 114, 103, 41, 0 };
    cx__23226 = (hpushstr(0, newstring(s))); }
    cx__23231 = (hpushstr(0, newstring("void(assert(is_")));
    { static char s[] = { 95, 36, 97, 114, 103, 41, 41, 0 };
    cx__23233 = (hpushstr(0, newstring(s))); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(99);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(103);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(114);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(97);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23248 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(49);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(45);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(35);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(103);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(114);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(97);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23259 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(49);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(43);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(35);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(103);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(114);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(97);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23274 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(35);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(103);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(114);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(97);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23289 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(103);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(114);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(97);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23300 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(97);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23309 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(101);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(118);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(105);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(108);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23314 = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(110);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(114);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(117);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(116);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(101);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(114);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23325 = (hendblk(3)); }
    cx__23344 = (hpushstr(0, newstring("unrecognized format directive ")));
    cx__23351 = (hpushstr(0, newstring("infinite loop in format")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = (mksymbol(internsym("?")));
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23361 = (hendblk(3)); }
    { static char s[] = { 36, 0 };
    cx__23401 = (hpushstr(0, newstring(s))); }
    cx__23420 = (hpushstr(0, newstring("_from_")));
    cx__23428 = (hpushstr(0, newstring("void_from_")));
    cx__23447 = (hpushstr(0, newstring(", ")));
    cx__23448 = (hpushstr(0, newstring("01234567890")));
    cx__23456 = (hpushstr(0, newstring("(void)(")));
    cx__23539 = (hpushstr(0, newstring("missing format directive")));
    cx__23584 = (hpushstr(0, newstring("missing argument in format")));
    cx__23591 = (hpushstr(0, newstring("unused arguments in format")));
    cx__23609 = (hpushstr(0, newstring("can't flatten???")));
    cx__23662 = (hpushstr(0, newstring("<no result info>")));
    cx__23666 = (hpushstr(0, newstring("### no result info ###")));
    cx__23667 = (hpushstr(0, newstring("### no live info ###")));
    cx__23683 = (hpushstr(0, newstring("0")));
    cx__23690 = (hpushstr(0, newstring("_")));
    cx__23731 = (hpushstr(0, newstring("arg")));
    cx__23733 = (hpushstr(0, newstring("arg#")));
    cx__23734 = (hpushstr(0, newstring("argc")));
    cx__23735 = (hpushstr(0, newstring("{")));
    cx__23736 = (hpushstr(0, newstring("}")));
    cx__23761 = (hpushstr(0, newstring("")));
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_char(58);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(47);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[0] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 1); /* 1 live regs */
    *--hp = r[0];  
    *--hp = obj_from_char(92);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__23777 = (hendblk(3)); }
    { static char s[] = { 35, 105, 110, 99, 108, 117, 100, 101, 32, 60, 115, 116, 100, 105, 111, 46, 104, 62, 10, 35, 105, 110, 99, 108, 117, 100, 101, 32, 60, 115, 116, 100, 100, 101, 102, 46, 104, 62, 10, 35, 105, 110, 99, 108, 117, 100, 101, 32, 60, 115, 116, 100, 108, 105, 98, 46, 104, 62, 10, 35, 105, 110, 99, 108, 117, 100, 101, 32, 60, 97, 115, 115, 101, 114, 116, 46, 104, 62, 10, 0 };
    cx__23999 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 35, 100, 101, 102, 105, 110, 101, 32, 82, 69, 71, 83, 95, 83, 73, 90, 69, 32, 53, 48, 48, 48, 10, 10, 116, 121, 112, 101, 100, 101, 102, 32, 112, 116, 114, 100, 105, 102, 102, 95, 116, 32, 111, 98, 106, 59, 32, 32, 32, 32, 32, 32, 32, 32, 47, 42, 32, 112, 111, 105, 110, 116, 101, 114, 115, 32, 97, 114, 101, 32, 116, 104, 105, 115, 32, 115, 105, 122, 101, 44, 32, 108, 111, 119, 101, 114, 32, 98, 105, 116, 32, 122, 101, 114, 111, 32, 42, 47, 10, 116, 121, 112, 101, 100, 101, 102, 32, 112, 116, 114, 100, 105, 102, 102, 95, 116, 32, 99, 120, 111, 105, 110, 116, 95, 116, 59, 32, 32, 32, 47, 42, 32, 115, 97, 109, 101, 32, 116, 104, 105, 110, 103, 44, 32, 117, 115, 101, 100, 32, 97, 115, 32, 105, 110, 116, 101, 103, 101, 114, 32, 42, 47, 10, 116, 121, 112, 101, 100, 101, 102, 32, 115, 116, 114, 117, 99, 116, 32, 123, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 47, 42, 32, 116, 121, 112, 101, 32, 100, 101, 115, 99, 114, 105, 112, 116, 111, 114, 32, 42, 47, 10, 32, 32, 99, 111, 110, 115, 116, 32, 99, 104, 97, 114, 32, 42, 116, 110, 97, 109, 101, 59, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 47, 42, 32, 110, 97, 109, 101, 32, 40, 100, 101, 98, 117, 103, 41, 32, 42, 47, 10, 32, 32, 118, 111, 105, 100, 32, 40, 42, 102, 114, 101, 101, 41, 40, 118, 111, 105, 100, 42, 41, 59, 32, 32, 32, 32, 32, 32, 32, 32, 47, 42, 32, 100, 101, 97, 108, 108, 111, 99, 97, 116, 111, 114, 32, 42, 47, 10, 125, 32, 99, 120, 116, 121, 112, 101, 95, 116, 59, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 110, 111, 116, 111, 98, 106, 112, 116, 114, 40, 111, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 40, 99, 104, 97, 114, 42, 41, 40, 111, 41, 32, 45, 32, 40, 99, 104, 97, 114, 42, 41, 99, 120, 103, 95, 104, 101, 97, 112, 41, 32, 38, 32, 99, 120, 103, 95, 104, 109, 97, 115, 107, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 105, 115, 111, 98, 106, 112, 116, 114, 40, 111, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 33, 110, 111, 116, 111, 98, 106, 112, 116, 114, 40, 111, 41, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 110, 111, 116, 97, 112, 116, 114, 40, 111, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 111, 41, 32, 38, 32, 49, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 105, 115, 97, 112, 116, 114, 40, 111, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 33, 110, 111, 116, 97, 112, 116, 114, 40, 111, 41, 41, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 111, 41, 32, 32, 32, 32, 32, 32, 32, 40, 111, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 111, 98, 106, 112, 116, 114, 40, 112, 41, 32, 32, 32, 32, 40, 40, 111, 98, 106, 41, 40, 112, 41, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 115, 105, 122, 101, 40, 110, 41, 32, 32, 32, 32, 32, 32, 40, 40, 40, 99, 120, 111, 105, 110, 116, 95, 116, 41, 40, 110, 41, 32, 60, 60, 32, 49, 41, 32, 124, 32, 49, 41, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 111, 98, 106, 112, 116, 114, 40, 112, 41, 32, 40, 112, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 111, 41, 32, 32, 32, 32, 40, 40, 111, 98, 106, 42, 41, 40, 111, 41, 41, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 115, 105, 122, 101, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 111, 41, 32, 32, 32, 32, 32, 32, 40, 40, 105, 110, 116, 41, 40, 40, 111, 41, 32, 62, 62, 32, 49, 41, 41, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 99, 97, 115, 101, 40, 110, 41, 32, 32, 32, 32, 32, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 111, 98, 106, 112, 116, 114, 40, 99, 97, 115, 101, 115, 43, 40, 110, 41, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 99, 97, 115, 101, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 111, 41, 32, 32, 32, 32, 32, 32, 40, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 111, 41, 45, 99, 97, 115, 101, 115, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 107, 116, 114, 97, 112, 40, 41, 32, 32, 32, 32, 32, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 115, 105, 122, 101, 40, 48, 120, 53, 68, 53, 54, 70, 56, 48, 54, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 118, 111, 105, 100, 40, 118, 41, 32, 32, 32, 32, 32, 32, 40, 40, 118, 111, 105, 100, 41, 40, 118, 41, 44, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 115, 105, 122, 101, 40, 48, 120, 54, 70, 53, 54, 68, 70, 55, 55, 41, 41, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 111, 41, 32, 32, 32, 32, 32, 32, 40, 111, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 98, 111, 111, 108, 40, 98, 41, 32, 32, 32, 32, 32, 40, 98, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 115, 105, 122, 101, 40, 115, 41, 32, 32, 32, 32, 32, 40, 115, 41, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 118, 111, 105, 100, 95, 102, 114, 111, 109, 95, 118, 111, 105, 100, 40, 118, 41, 32, 32, 32, 32, 32, 40, 118, 111, 105, 100, 41, 40, 118, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 118, 111, 105, 100, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 111, 41, 32, 32, 32, 32, 32, 32, 40, 118, 111, 105, 100, 41, 40, 111, 41, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 114, 114, 101, 115, 101, 114, 118, 101, 40, 109, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 105, 102, 32, 40, 114, 32, 62, 32, 99, 120, 103, 95, 114, 101, 103, 115, 32, 43, 32, 82, 69, 71, 83, 95, 83, 73, 90, 69, 32, 45, 32, 50, 42, 40, 109, 41, 41, 32, 114, 32, 61, 32, 99, 120, 109, 95, 114, 103, 99, 40, 114, 44, 32, 114, 43, 40, 109, 41, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 104, 112, 117, 115, 104, 112, 116, 114, 40, 112, 44, 32, 112, 116, 44, 32, 108, 41, 32, 32, 32, 32, 40, 104, 114, 101, 115, 101, 114, 118, 101, 40, 50, 44, 32, 108, 41, 44, 32, 42, 45, 45, 104, 112, 32, 61, 32, 40, 111, 98, 106, 41, 40, 112, 41, 44, 32, 42, 45, 45, 104, 112, 32, 61, 32, 40, 111, 98, 106, 41, 40, 112, 116, 41, 44, 32, 40, 111, 98, 106, 41, 40, 104, 112, 43, 49, 41, 41, 32, 32, 32, 10, 35, 100, 101, 102, 105, 110, 101, 32, 104, 98, 115, 122, 40, 115, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 115, 41, 32, 43, 32, 49, 41, 32, 47, 42, 32, 49, 32, 101, 120, 116, 114, 97, 32, 119, 111, 114, 100, 32, 116, 111, 32, 115, 116, 111, 114, 101, 32, 98, 108, 111, 99, 107, 32, 115, 105, 122, 101, 32, 42, 47, 10, 35, 100, 101, 102, 105, 110, 101, 32, 104, 114, 101, 115, 101, 114, 118, 101, 40, 110, 44, 32, 108, 41, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 104, 112, 32, 60, 32, 99, 120, 103, 95, 104, 101, 97, 112, 32, 43, 32, 40, 110, 41, 41, 32, 63, 32, 104, 112, 32, 61, 32, 99, 120, 109, 95, 104, 103, 99, 40, 114, 44, 32, 114, 43, 40, 108, 41, 44, 32, 104, 112, 44, 32, 110, 41, 32, 58, 32, 104, 112, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 104, 101, 110, 100, 98, 108, 107, 40, 110, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 42, 45, 45, 104, 112, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 115, 105, 122, 101, 40, 110, 41, 44, 32, 40, 111, 98, 106, 41, 40, 104, 112, 43, 49, 41, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 104, 98, 108, 107, 108, 101, 110, 40, 112, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 115, 105, 122, 101, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 40, 40, 111, 98, 106, 42, 41, 40, 112, 41, 41, 91, 45, 49, 93, 41, 10, 35, 100, 101, 102, 105, 110, 101, 32, 104, 98, 108, 107, 114, 101, 102, 40, 112, 44, 32, 105, 41, 32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 40, 111, 98, 106, 42, 41, 40, 112, 41, 41, 91, 105, 93, 41, 10, 10, 116, 121, 112, 101, 100, 101, 102, 32, 111, 98, 106, 32, 40, 42, 99, 120, 104, 111, 115, 116, 95, 116, 41, 40, 111, 98, 106, 41, 59, 10, 116, 121, 112, 101, 100, 101, 102, 32, 115, 116, 114, 117, 99, 116, 32, 99, 120, 114, 111, 111, 116, 95, 116, 97, 103, 32, 123, 10, 32, 32, 105, 110, 116, 32, 103, 108, 111, 98, 99, 59, 32, 111, 98, 106, 32, 42, 42, 103, 108, 111, 98, 118, 59, 10, 32, 32, 115, 116, 114, 117, 99, 116, 32, 99, 120, 114, 111, 111, 116, 95, 116, 97, 103, 32, 42, 110, 101, 120, 116, 59, 10, 125, 32, 99, 120, 114, 111, 111, 116, 95, 116, 59, 10, 10, 101, 120, 116, 101, 114, 110, 32, 111, 98, 106, 32, 42, 99, 120, 103, 95, 104, 101, 97, 112, 59, 10, 101, 120, 116, 101, 114, 110, 32, 111, 98, 106, 32, 42, 99, 120, 103, 95, 104, 112, 59, 10, 101, 120, 116, 101, 114, 110, 32, 99, 120, 111, 105, 110, 116, 95, 116, 32, 99, 120, 103, 95, 104, 109, 97, 115, 107, 59, 10, 101, 120, 116, 101, 114, 110, 32, 99, 120, 114, 111, 111, 116, 95, 116, 32, 42, 99, 120, 103, 95, 114, 111, 111, 116, 112, 59, 10, 101, 120, 116, 101, 114, 110, 32, 111, 98, 106, 32, 42, 99, 120, 109, 95, 114, 103, 99, 40, 111, 98, 106, 32, 42, 114, 101, 103, 115, 44, 32, 111, 98, 106, 32, 42, 114, 101, 103, 112, 41, 59, 10, 101, 120, 116, 101, 114, 110, 32, 111, 98, 106, 32, 42, 99, 120, 109, 95, 104, 103, 99, 40, 111, 98, 106, 32, 42, 114, 101, 103, 115, 44, 32, 111, 98, 106, 32, 42, 114, 101, 103, 112, 44, 32, 111, 98, 106, 32, 42, 104, 112, 44, 32, 115, 105, 122, 101, 95, 116, 32, 110, 101, 101, 100, 115, 41, 59, 10, 101, 120, 116, 101, 114, 110, 32, 111, 98, 106, 32, 99, 120, 103, 95, 114, 101, 103, 115, 91, 82, 69, 71, 83, 95, 83, 73, 90, 69, 93, 59, 10, 101, 120, 116, 101, 114, 110, 32, 118, 111, 105, 100, 32, 42, 99, 120, 109, 95, 99, 107, 110, 117, 108, 108, 40, 118, 111, 105, 100, 32, 42, 112, 44, 32, 99, 104, 97, 114, 32, 42, 109, 115, 103, 41, 59, 10, 35, 105, 102, 110, 100, 101, 102, 32, 78, 68, 69, 66, 85, 71, 10, 101, 120, 116, 101, 114, 110, 32, 105, 110, 116, 32, 99, 120, 103, 95, 114, 99, 59, 10, 35, 101, 110, 100, 105, 102, 10, 0 };
    cx__231001 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 115, 116, 97, 116, 105, 99, 32, 111, 98, 106, 32, 104, 111, 115, 116, 40, 111, 98, 106, 32, 112, 99, 41, 10, 123, 10, 32, 32, 114, 101, 103, 105, 115, 116, 101, 114, 32, 111, 98, 106, 32, 42, 114, 32, 61, 32, 99, 120, 103, 95, 114, 101, 103, 115, 59, 10, 32, 32, 114, 101, 103, 105, 115, 116, 101, 114, 32, 111, 98, 106, 32, 42, 104, 112, 32, 61, 32, 99, 120, 103, 95, 104, 112, 59, 10, 35, 105, 102, 110, 100, 101, 102, 32, 78, 68, 69, 66, 85, 71, 10, 32, 32, 114, 101, 103, 105, 115, 116, 101, 114, 32, 105, 110, 116, 32, 114, 99, 32, 61, 32, 99, 120, 103, 95, 114, 99, 59, 10, 35, 101, 110, 100, 105, 102, 10, 32, 32, 106, 117, 109, 112, 58, 32, 10, 32, 32, 115, 119, 105, 116, 99, 104, 32, 40, 99, 97, 115, 101, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 112, 99, 41, 41, 32, 123, 10, 10, 0 };
    cx__231003 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 100, 101, 102, 97, 117, 108, 116, 58, 32, 47, 42, 32, 105, 110, 116, 101, 114, 45, 104, 111, 115, 116, 32, 99, 97, 108, 108, 32, 42, 47, 10, 32, 32, 32, 32, 99, 120, 103, 95, 104, 112, 32, 61, 32, 104, 112, 59, 10, 32, 32, 32, 32, 99, 120, 109, 95, 114, 103, 99, 40, 114, 44, 32, 114, 32, 43, 32, 77, 65, 88, 95, 76, 73, 86, 69, 82, 69, 71, 83, 41, 59, 10, 35, 105, 102, 110, 100, 101, 102, 32, 78, 68, 69, 66, 85, 71, 10, 32, 32, 32, 32, 99, 120, 103, 95, 114, 99, 32, 61, 32, 114, 99, 59, 10, 35, 101, 110, 100, 105, 102, 10, 32, 32, 32, 32, 114, 101, 116, 117, 114, 110, 32, 112, 99, 59, 10, 32, 32, 125, 10, 125, 10, 0 };
    cx__231005 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 118, 111, 105, 100, 32, 77, 79, 68, 85, 76, 69, 40, 118, 111, 105, 100, 41, 10, 123, 10, 32, 32, 111, 98, 106, 32, 112, 99, 59, 10, 32, 32, 105, 102, 32, 40, 33, 114, 111, 111, 116, 46, 110, 101, 120, 116, 41, 32, 123, 10, 32, 32, 32, 32, 114, 111, 111, 116, 46, 110, 101, 120, 116, 32, 61, 32, 99, 120, 103, 95, 114, 111, 111, 116, 112, 59, 10, 32, 32, 32, 32, 99, 120, 103, 95, 114, 111, 111, 116, 112, 32, 61, 32, 38, 114, 111, 111, 116, 59, 10, 32, 32, 32, 32, 76, 79, 65, 68, 40, 41, 59, 10, 32, 32, 32, 32, 112, 99, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 99, 97, 115, 101, 40, 48, 41, 59, 10, 32, 32, 32, 32, 97, 115, 115, 101, 114, 116, 40, 40, 99, 120, 103, 95, 114, 99, 32, 61, 32, 48, 44, 32, 49, 41, 41, 59, 10, 32, 32, 32, 32, 119, 104, 105, 108, 101, 32, 40, 112, 99, 41, 32, 112, 99, 32, 61, 32, 40, 42, 40, 99, 120, 104, 111, 115, 116, 95, 116, 42, 41, 112, 99, 41, 40, 112, 99, 41, 59, 32, 10, 32, 32, 32, 32, 97, 115, 115, 101, 114, 116, 40, 99, 120, 103, 95, 114, 99, 32, 61, 61, 32, 50, 41, 59, 10, 32, 32, 125, 10, 125, 10, 0 };
    cx__231007 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 35, 100, 101, 102, 105, 110, 101, 32, 72, 69, 65, 80, 95, 83, 73, 90, 69, 32, 49, 51, 49, 48, 55, 50, 32, 47, 42, 32, 50, 94, 49, 55, 32, 42, 47, 10, 10, 111, 98, 106, 32, 42, 99, 120, 103, 95, 104, 101, 97, 112, 32, 61, 32, 78, 85, 76, 76, 59, 10, 99, 120, 111, 105, 110, 116, 95, 116, 32, 99, 120, 103, 95, 104, 109, 97, 115, 107, 32, 61, 32, 48, 59, 10, 111, 98, 106, 32, 42, 99, 120, 103, 95, 104, 112, 32, 61, 32, 78, 85, 76, 76, 59, 10, 99, 120, 114, 111, 111, 116, 95, 116, 32, 42, 99, 120, 103, 95, 114, 111, 111, 116, 112, 32, 61, 32, 78, 85, 76, 76, 59, 10, 111, 98, 106, 32, 99, 120, 103, 95, 114, 101, 103, 115, 91, 82, 69, 71, 83, 95, 83, 73, 90, 69, 93, 59, 10, 35, 105, 102, 110, 100, 101, 102, 32, 78, 68, 69, 66, 85, 71, 10, 105, 110, 116, 32, 99, 120, 103, 95, 114, 99, 32, 61, 32, 48, 59, 10, 35, 101, 110, 100, 105, 102, 10, 10, 115, 116, 97, 116, 105, 99, 32, 111, 98, 106, 32, 42, 104, 101, 97, 112, 50, 32, 61, 32, 78, 85, 76, 76, 59, 10, 115, 116, 97, 116, 105, 99, 32, 115, 105, 122, 101, 95, 116, 32, 104, 115, 105, 122, 101, 32, 61, 32, 48, 59, 32, 10, 115, 116, 97, 116, 105, 99, 32, 99, 120, 111, 105, 110, 116, 95, 116, 32, 104, 109, 97, 115, 107, 50, 32, 61, 32, 48, 59, 10, 115, 116, 97, 116, 105, 99, 32, 105, 110, 116, 32, 103, 99, 99, 111, 117, 110, 116, 32, 61, 32, 48, 44, 32, 98, 117, 109, 112, 99, 111, 117, 110, 116, 32, 61, 32, 48, 59, 10, 10, 35, 100, 101, 102, 105, 110, 101, 32, 110, 111, 116, 102, 119, 100, 112, 116, 114, 40, 111, 41, 32, 40, 40, 40, 99, 104, 97, 114, 42, 41, 40, 111, 41, 32, 45, 32, 40, 99, 104, 97, 114, 42, 41, 104, 101, 97, 112, 50, 41, 32, 38, 32, 104, 109, 97, 115, 107, 50, 41, 10, 10, 115, 116, 97, 116, 105, 99, 32, 118, 111, 105, 100, 32, 116, 111, 104, 101, 97, 112, 50, 40, 111, 98, 106, 42, 32, 112, 41, 10, 123, 10, 32, 32, 111, 98, 106, 32, 111, 32, 61, 32, 42, 112, 44, 32, 42, 111, 112, 44, 32, 102, 111, 44, 32, 42, 102, 111, 112, 59, 10, 32, 32, 105, 102, 32, 40, 110, 111, 116, 111, 98, 106, 112, 116, 114, 40, 111, 41, 41, 32, 114, 101, 116, 117, 114, 110, 59, 10, 32, 32, 102, 111, 32, 61, 32, 40, 111, 112, 32, 61, 32, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 111, 41, 41, 91, 45, 49, 93, 59, 32, 97, 115, 115, 101, 114, 116, 40, 102, 111, 41, 59, 10, 32, 32, 105, 102, 32, 40, 110, 111, 116, 97, 112, 116, 114, 40, 102, 111, 41, 41, 32, 123, 10, 32, 32, 32, 32, 102, 111, 112, 32, 61, 32, 111, 112, 32, 43, 32, 115, 105, 122, 101, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 102, 111, 41, 59, 32, 119, 104, 105, 108, 101, 32, 40, 102, 111, 112, 32, 62, 61, 32, 111, 112, 41, 32, 42, 45, 45, 99, 120, 103, 95, 104, 112, 32, 61, 32, 42, 45, 45, 102, 111, 112, 59, 10, 32, 32, 32, 32, 42, 112, 32, 61, 32, 42, 102, 111, 112, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 111, 98, 106, 112, 116, 114, 40, 99, 120, 103, 95, 104, 112, 43, 49, 41, 59, 10, 32, 32, 125, 32, 101, 108, 115, 101, 32, 105, 102, 32, 40, 110, 111, 116, 102, 119, 100, 112, 116, 114, 40, 102, 111, 41, 41, 32, 123, 10, 32, 32, 32, 32, 42, 45, 45, 99, 120, 103, 95, 104, 112, 32, 61, 32, 42, 111, 112, 45, 45, 59, 32, 42, 45, 45, 99, 120, 103, 95, 104, 112, 32, 61, 32, 42, 111, 112, 59, 10, 32, 32, 32, 32, 42, 112, 32, 61, 32, 42, 111, 112, 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 111, 98, 106, 112, 116, 114, 40, 99, 120, 103, 95, 104, 112, 43, 49, 41, 59, 10, 32, 32, 125, 32, 101, 108, 115, 101, 32, 42, 112, 32, 61, 32, 102, 111, 59, 10, 125, 10, 10, 115, 116, 97, 116, 105, 99, 32, 118, 111, 105, 100, 32, 102, 105, 110, 97, 108, 105, 122, 101, 40, 111, 98, 106, 32, 42, 104, 112, 44, 32, 111, 98, 106, 32, 42, 101, 110, 100, 41, 10, 123, 10, 32, 32, 119, 104, 105, 108, 101, 32, 40, 104, 112, 32, 60, 32, 101, 110, 100, 41, 32, 123, 10, 32, 32, 32, 32, 111, 98, 106, 32, 102, 111, 32, 61, 32, 42, 104, 112, 43, 43, 59, 32, 97, 115, 115, 101, 114, 116, 40, 102, 111, 41, 59, 10, 32, 32, 32, 32, 105, 102, 32, 40, 110, 111, 116, 97, 112, 116, 114, 40, 102, 111, 41, 41, 32, 104, 112, 32, 43, 61, 32, 115, 105, 122, 101, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 102, 111, 41, 59, 10, 32, 32, 32, 32, 101, 108, 115, 101, 32, 105, 102, 32, 40, 110, 111, 116, 102, 119, 100, 112, 116, 114, 40, 102, 111, 41, 41, 32, 40, 40, 99, 120, 116, 121, 112, 101, 95, 116, 42, 41, 102, 111, 41, 45, 62, 102, 114, 101, 101, 40, 40, 118, 111, 105, 100, 42, 41, 42, 104, 112, 43, 43, 41, 59, 10, 32, 32, 32, 32, 101, 108, 115, 101, 32, 105, 102, 32, 40, 110, 111, 116, 97, 112, 116, 114, 40, 102, 111, 32, 61, 32, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 102, 111, 41, 91, 45, 49, 93, 41, 41, 32, 104, 112, 32, 43, 61, 32, 115, 105, 122, 101, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 102, 111, 41, 59, 10, 32, 32, 32, 32, 101, 108, 115, 101, 32, 43, 43, 104, 112, 59, 10, 32, 32, 125, 32, 97, 115, 115, 101, 114, 116, 40, 104, 112, 32, 61, 61, 32, 101, 110, 100, 41, 59, 10, 125, 10, 10, 115, 116, 97, 116, 105, 99, 32, 115, 105, 122, 101, 95, 116, 32, 114, 101, 108, 111, 99, 97, 116, 101, 40, 111, 98, 106, 32, 42, 114, 101, 103, 115, 44, 32, 111, 98, 106, 32, 42, 114, 101, 103, 112, 44, 32, 111, 98, 106, 32, 42, 101, 110, 100, 50, 44, 32, 111, 98, 106, 32, 42, 101, 110, 100, 49, 41, 32, 10, 123, 10, 32, 32, 111, 98, 106, 32, 42, 112, 44, 32, 42, 104, 112, 49, 32, 61, 32, 99, 120, 103, 95, 104, 112, 59, 32, 99, 120, 114, 111, 111, 116, 95, 116, 32, 42, 112, 114, 59, 32, 10, 32, 32, 99, 120, 103, 95, 104, 112, 32, 61, 32, 101, 110, 100, 50, 59, 32, 105, 102, 32, 40, 33, 99, 120, 103, 95, 104, 101, 97, 112, 41, 32, 114, 101, 116, 117, 114, 110, 32, 48, 59, 10, 32, 32, 102, 111, 114, 32, 40, 112, 32, 61, 32, 114, 101, 103, 115, 59, 32, 112, 32, 60, 32, 114, 101, 103, 112, 59, 32, 43, 43, 112, 41, 32, 116, 111, 104, 101, 97, 112, 50, 40, 112, 41, 59, 10, 32, 32, 102, 111, 114, 32, 40, 112, 114, 32, 61, 32, 99, 120, 103, 95, 114, 111, 111, 116, 112, 59, 32, 112, 114, 59, 32, 112, 114, 32, 61, 32, 112, 114, 45, 62, 110, 101, 120, 116, 41, 32, 123, 10, 32, 32, 32, 32, 111, 98, 106, 32, 42, 42, 112, 112, 32, 61, 32, 112, 114, 45, 62, 103, 108, 111, 98, 118, 59, 32, 105, 110, 116, 32, 99, 32, 61, 32, 112, 114, 45, 62, 103, 108, 111, 98, 99, 59, 10, 32, 32, 32, 32, 119, 104, 105, 108, 101, 32, 40, 99, 45, 45, 32, 62, 32, 48, 41, 32, 116, 111, 104, 101, 97, 112, 50, 40, 42, 112, 112, 43, 43, 41, 59, 10, 32, 32, 125, 10, 32, 32, 102, 111, 114, 32, 40, 112, 32, 61, 32, 101, 110, 100, 50, 59, 32, 112, 32, 62, 32, 99, 120, 103, 95, 104, 112, 59, 32, 45, 45, 112, 41, 32, 116, 111, 104, 101, 97, 112, 50, 40, 112, 45, 49, 41, 59, 10, 32, 32, 105, 102, 32, 40, 101, 110, 100, 49, 41, 32, 102, 105, 110, 97, 108, 105, 122, 101, 40, 104, 112, 49, 44, 32, 101, 110, 100, 49, 41, 59, 10, 32, 32, 114, 101, 116, 117, 114, 110, 32, 101, 110, 100, 50, 32, 45, 32, 99, 120, 103, 95, 104, 112, 59, 10, 125, 10, 10, 111, 98, 106, 32, 42, 99, 120, 109, 95, 104, 103, 99, 40, 111, 98, 106, 32, 42, 114, 101, 103, 115, 44, 32, 111, 98, 106, 32, 42, 114, 101, 103, 112, 44, 32, 111, 98, 106, 32, 42, 104, 112, 44, 32, 115, 105, 122, 101, 95, 116, 32, 110, 101, 101, 100, 115, 41, 32, 10, 123, 10, 32, 32, 111, 98, 106, 32, 42, 112, 32, 61, 32, 99, 120, 103, 95, 104, 101, 97, 112, 59, 32, 99, 120, 103, 95, 104, 112, 32, 61, 32, 104, 112, 59, 32, 43, 43, 103, 99, 99, 111, 117, 110, 116, 59, 10, 32, 32, 105, 102, 32, 40, 104, 115, 105, 122, 101, 32, 60, 32, 40, 110, 101, 101, 100, 115, 32, 43, 61, 32, 114, 101, 108, 111, 99, 97, 116, 101, 40, 114, 101, 103, 115, 44, 32, 114, 101, 103, 112, 44, 32, 104, 101, 97, 112, 50, 32, 43, 32, 104, 115, 105, 122, 101, 44, 32, 112, 32, 43, 32, 104, 115, 105, 122, 101, 41, 41, 41, 32, 123, 10, 32, 32, 32, 32, 115, 105, 122, 101, 95, 116, 32, 115, 32, 61, 32, 72, 69, 65, 80, 95, 83, 73, 90, 69, 59, 32, 119, 104, 105, 108, 101, 32, 40, 115, 32, 60, 32, 110, 101, 101, 100, 115, 41, 32, 115, 32, 42, 61, 32, 50, 59, 10, 32, 32, 32, 32, 104, 109, 97, 115, 107, 50, 32, 61, 32, 49, 32, 124, 32, 126, 40, 115, 42, 115, 105, 122, 101, 111, 102, 40, 111, 98, 106, 41, 45, 49, 41, 59, 10, 32, 32, 32, 32, 105, 102, 32, 40, 33, 40, 112, 32, 61, 32, 114, 101, 97, 108, 108, 111, 99, 40, 99, 120, 103, 95, 104, 101, 97, 112, 44, 32, 115, 42, 115, 105, 122, 101, 111, 102, 40, 111, 98, 106, 41, 41, 41, 41, 32, 123, 32, 112, 101, 114, 114, 111, 114, 40, 34, 97, 108, 108, 111, 99, 91, 104, 93, 34, 41, 59, 32, 101, 120, 105, 116, 40, 50, 41, 59, 32, 125, 10, 32, 32, 32, 32, 99, 120, 103, 95, 104, 101, 97, 112, 32, 61, 32, 104, 101, 97, 112, 50, 59, 32, 104, 101, 97, 112, 50, 32, 61, 32, 112, 59, 32, 114, 101, 108, 111, 99, 97, 116, 101, 40, 114, 101, 103, 115, 44, 32, 114, 101, 103, 112, 44, 32, 104, 101, 97, 112, 50, 32, 43, 32, 115, 44, 32, 48, 41, 59, 10, 32, 32, 32, 32, 105, 102, 32, 40, 33, 40, 112, 32, 61, 32, 114, 101, 97, 108, 108, 111, 99, 40, 99, 120, 103, 95, 104, 101, 97, 112, 44, 32, 115, 42, 115, 105, 122, 101, 111, 102, 40, 111, 98, 106, 41, 41, 41, 41, 32, 123, 32, 112, 101, 114, 114, 111, 114, 40, 34, 97, 108, 108, 111, 99, 91, 104, 93, 34, 41, 59, 32, 101, 120, 105, 116, 40, 50, 41, 59, 32, 125, 10, 32, 32, 32, 32, 104, 115, 105, 122, 101, 32, 61, 32, 115, 59, 32, 99, 120, 103, 95, 104, 109, 97, 115, 107, 32, 61, 32, 104, 109, 97, 115, 107, 50, 59, 32, 43, 43, 98, 117, 109, 112, 99, 111, 117, 110, 116, 59, 10, 32, 32, 125, 10, 32, 32, 99, 120, 103, 95, 104, 101, 97, 112, 32, 61, 32, 104, 101, 97, 112, 50, 59, 32, 104, 101, 97, 112, 50, 32, 61, 32, 112, 59, 10, 32, 32, 114, 101, 116, 117, 114, 110, 32, 99, 120, 103, 95, 104, 112, 59, 10, 125, 10, 10, 111, 98, 106, 32, 42, 99, 120, 109, 95, 114, 103, 99, 40, 111, 98, 106, 32, 42, 114, 101, 103, 115, 44, 32, 111, 98, 106, 32, 42, 114, 101, 103, 112, 41, 32, 10, 123, 10, 32, 32, 111, 98, 106, 32, 42, 112, 32, 61, 32, 99, 120, 103, 95, 114, 101, 103, 115, 59, 10, 32, 32, 97, 115, 115, 101, 114, 116, 40, 114, 101, 103, 112, 32, 60, 61, 32, 99, 120, 103, 95, 114, 101, 103, 115, 32, 43, 32, 82, 69, 71, 83, 95, 83, 73, 90, 69, 41, 59, 10, 32, 32, 119, 104, 105, 108, 101, 32, 40, 114, 101, 103, 115, 32, 60, 32, 114, 101, 103, 112, 41, 32, 42, 112, 43, 43, 32, 61, 32, 42, 114, 101, 103, 115, 43, 43, 59, 10, 32, 32, 114, 101, 116, 117, 114, 110, 32, 99, 120, 103, 95, 114, 101, 103, 115, 59, 10, 125, 10, 10, 118, 111, 105, 100, 32, 42, 99, 120, 109, 95, 99, 107, 110, 117, 108, 108, 40, 118, 111, 105, 100, 32, 42, 112, 44, 32, 99, 104, 97, 114, 32, 42, 109, 115, 103, 41, 10, 123, 10, 32, 32, 105, 102, 32, 40, 33, 112, 41, 32, 123, 32, 10, 32, 32, 32, 32, 112, 101, 114, 114, 111, 114, 40, 109, 115, 103, 41, 59, 32, 101, 120, 105, 116, 40, 50, 41, 59, 32, 10, 32, 32, 125, 10, 32, 32, 114, 101, 116, 117, 114, 110, 32, 112, 59, 10, 125, 10, 0 };
    cx__231009 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 105, 110, 116, 32, 109, 97, 105, 110, 40, 105, 110, 116, 32, 97, 114, 103, 99, 44, 32, 99, 104, 97, 114, 32, 42, 42, 97, 114, 103, 118, 41, 32, 123, 10, 32, 32, 105, 110, 116, 32, 114, 101, 115, 59, 32, 111, 98, 106, 32, 112, 99, 59, 10, 32, 32, 111, 98, 106, 32, 114, 101, 116, 99, 108, 91, 49, 93, 32, 61, 32, 123, 32, 48, 32, 125, 59, 32, 10, 32, 32, 77, 79, 68, 85, 76, 69, 40, 41, 59, 10, 32, 32, 99, 120, 103, 95, 114, 101, 103, 115, 91, 48, 93, 32, 61, 32, 99, 120, 95, 109, 97, 105, 110, 59, 10, 32, 32, 99, 120, 103, 95, 114, 101, 103, 115, 91, 49, 93, 32, 61, 32, 40, 111, 98, 106, 41, 114, 101, 116, 99, 108, 59, 10, 32, 32, 99, 120, 103, 95, 114, 101, 103, 115, 91, 50, 93, 32, 61, 32, 40, 111, 98, 106, 41, 97, 114, 103, 118, 59, 10, 32, 32, 97, 115, 115, 101, 114, 116, 40, 99, 120, 103, 95, 114, 99, 32, 61, 32, 51, 41, 59, 10, 32, 32, 112, 99, 32, 61, 32, 111, 98, 106, 112, 116, 114, 95, 102, 114, 111, 109, 95, 111, 98, 106, 40, 99, 120, 95, 109, 97, 105, 110, 41, 91, 48, 93, 59, 10, 32, 32, 119, 104, 105, 108, 101, 32, 40, 112, 99, 41, 32, 112, 99, 32, 61, 32, 40, 42, 40, 99, 120, 104, 111, 115, 116, 95, 116, 42, 41, 112, 99, 41, 40, 112, 99, 41, 59, 32, 10, 32, 32, 97, 115, 115, 101, 114, 116, 40, 99, 120, 103, 95, 114, 99, 32, 61, 61, 32, 51, 41, 59, 10, 32, 32, 114, 101, 115, 32, 61, 32, 40, 99, 120, 103, 95, 114, 101, 103, 115, 91, 50, 93, 32, 33, 61, 32, 48, 41, 59, 32, 10, 32, 32, 47, 42, 32, 102, 112, 114, 105, 110, 116, 102, 40, 115, 116, 100, 101, 114, 114, 44, 32, 34, 37, 100, 32, 99, 111, 108, 108, 101, 99, 116, 105, 111, 110, 115, 44, 32, 37, 100, 32, 114, 101, 97, 108, 108, 111, 99, 115, 92, 110, 34, 44, 32, 103, 99, 99, 111, 117, 110, 116, 44, 32, 98, 117, 109, 112, 99, 111, 117, 110, 116, 41, 59, 32, 42, 47, 10, 32, 32, 114, 101, 116, 117, 114, 110, 32, 114, 101, 115, 59, 32, 10, 125, 10, 0 };
    cx__231011 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 46, 10, 0 };
    cx__231037 = (hpushstr(0, newstring(s))); }
    cx__231040 = (hpushstr(0, newstring("no clause matches ~s")));
    cx__231042 = (hpushstr(0, newstring("Error: ")));
    cx__231046 = (hpushstr(0, newstring("Error in ~a: ")));
    { static char s[] = { 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 0 };
    cx__231070 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 32, 32, 32, 32, 0 };
    cx__231075 = (hpushstr(0, newstring(s))); }
    cx__231078 = (hpushstr(0, newstring(";")));
    { static char s[] = { 10, 32, 32, 32, 32, 103, 111, 116, 111, 32, 0 };
    cx__231083 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 61, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 59, 0 };
    cx__231097 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 32, 32, 32, 32, 47, 42, 32, 116, 97, 105, 108, 32, 99, 97, 108, 108, 32, 42, 47, 0 };
    cx__231114 = (hpushstr(0, newstring(s))); }
    cx__231117 = (hpushstr(0, newstring("return")));
    { static obj c[] = { obj_from_case(1) }; cx_intersectionq_231133 = (obj)c; }
    cx__231203 = (hpushstr(0, newstring(" obj_from_")));
    cx__231206 = (hpushstr(0, newstring("<no live reg info>")));
    { static char s[] = { 10, 32, 32, 125, 0 };
    cx__231262 = (hpushstr(0, newstring(s))); }
    cx__231265 = (hpushstr(0, newstring(": ;")));
    { static char s[] = { 10, 32, 32, 0 };
    cx__231270 = (hpushstr(0, newstring(s))); }
    cx__231284 = (hpushstr(0, newstring(" */")));
    { static char s[] = { 10, 32, 32, 47, 42, 32, 100, 101, 97, 100, 32, 99, 111, 100, 101, 58, 32, 0 };
    cx__231292 = (hpushstr(0, newstring(s))); }
    cx__231308 = (hpushstr(0, newstring(":")));
    { static char s[] = { 10, 32, 32, 32, 32, 111, 98, 106, 32, 0 };
    cx__231354 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 32, 32, 123, 32, 47, 42, 32, 108, 101, 116, 114, 101, 99, 32, 42, 47, 0 };
    cx__231371 = (hpushstr(0, newstring(s))); }
    cx__231525 = (hpushstr(0, newstring(" =")));
    { static char s[] = { 10, 32, 32, 123, 32, 47, 42, 32, 108, 101, 116, 32, 42, 47, 0 };
    cx__231572 = (hpushstr(0, newstring(s))); }
    cx__231624 = (hpushstr(0, newstring("(void)")));
    { static obj c[] = { obj_from_case(3) }; cx_begin_2Dexp_3F_231627 = (obj)c; }
    { static char s[] = { 10, 32, 32, 125, 32, 101, 108, 115, 101, 32, 123, 0 };
    cx__231718 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 10, 32, 32, 105, 102, 32, 40, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 41, 32, 123, 0 };
    cx__231725 = (hpushstr(0, newstring(s))); }
    { static char s[] = { 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 0 };
    cx__231861 = (hpushstr(0, newstring(s))); }
    cx__231936 = (hpushstr(0, newstring("<no cx regs here!>")));
    cx__231952 = (hpushstr(0, newstring("))")));
    { static char s[] = { 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 0 };
    cx__231962 = (hpushstr(0, newstring(s))); }
    cx__231991 = (hpushstr(0, newstring("obj(")));
    { static char s[] = { 111, 98, 106, 40, 98, 111, 111, 108, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 32, 63, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 32, 58, 32, 111, 98, 106, 95, 102, 114, 111, 109, 95, 36, 97, 114, 103, 41, 0 };
    cx__232020 = (hpushstr(0, newstring(s))); }
    cx__232032 = (hpushstr(0, newstring(")")));
    cx__232379 = (hpushstr(0, newstring("s_")));
    cx__232397 = (hpushstr(0, newstring("_v")));
    cx__232434 = (hpushstr(0, newstring("id->rand-ids: id not registered")));
    cx__232444 = (hpushstr(0, newstring("id->label: id not registered")));
    cx__232449 = (hpushstr(0, newstring("l")));
    cx__232480 = (hpushstr(0, newstring("v")));
    cx__232487 = (hpushstr(0, newstring("cx_")));
    cx__232493 = (hpushstr(0, newstring("cxs_")));
    cx__232500 = (hpushstr(0, newstring("unsuitable exp for stack-function-code-generate: ")));
    { static char s[] = { 10, 125, 10, 0 };
    cx__232509 = (hpushstr(0, newstring(s))); }
    cx__232522 = (hpushstr(0, newstring("{ ")));
    { static char s[] = { 41, 10, 0 };
    cx__232525 = (hpushstr(0, newstring(s))); }
    cx__232536 = (hpushstr(0, newstring(", obj ")));
    cx__232550 = (hpushstr(0, newstring("obj ")));
    cx__232553 = (hpushstr(0, newstring("void")));
    cx__232561 = (hpushstr(0, newstring("static obj ")));
    { static char s[] = { 32, 42, 47, 10, 0 };
    cx__232564 = (hpushstr(0, newstring(s))); }
    cx__232572 = (hpushstr(0, newstring("/* ")));
    { static obj c[] = { obj_from_case(5) }; cx_string_2Dstarts_2Dwith_3F = (obj)c; }
    { static obj c[] = { obj_from_case(6) }; cx_string_2Dends_2Dwith_3F = (obj)c; }
    { static obj c[] = { obj_from_case(7) }; cx_string_2Dcontains_2Dat_3F = (obj)c; }
    { static obj c[] = { obj_from_case(8) }; cx_string_2Dcontains_3F = (obj)c; }
    { static obj c[] = { obj_from_case(9) }; cx_substring_2Dbefore = (obj)c; }
    { static obj c[] = { obj_from_case(10) }; cx_substring_2Dafter = (obj)c; }
    { static obj c[] = { obj_from_case(11) }; cx_substring_2Dbefore_2Dsuffix = (obj)c; }
    { static obj c[] = { obj_from_case(12) }; cx_substring_2Dafter_2Dprefix = (obj)c; }
    { static obj c[] = { obj_from_case(13) }; cx_string_2Dmember_3F = (obj)c; }
    { static obj c[] = { obj_from_case(14) }; cx_string_2Dmember_2Dci_3F = (obj)c; }
    { static obj c[] = { obj_from_case(15) }; cx_string_2Dspan = (obj)c; }
    { static obj c[] = { obj_from_case(16) }; cx_string_2Dspan_2Dci = (obj)c; }
    { static obj c[] = { obj_from_case(17) }; cx_substring_2Dafter_2Dspan = (obj)c; }
    { static obj c[] = { obj_from_case(18) }; cx_string_2Dreverse = (obj)c; }
    { static obj c[] = { obj_from_case(19) }; cx_string_2Dupcase = (obj)c; }
    { static obj c[] = { obj_from_case(22) }; cx_primexp_2Dctype = (obj)c; }
    { static obj c[] = { obj_from_case(24) }; cx_prim_2Dctype = (obj)c; }
    { static obj c[] = { obj_from_case(28) }; cx_prim_2Dcexp_3F = (obj)c; }
    { static obj c[] = { obj_from_case(29) }; cx_typecheck_2Dprim_2Dctype = (obj)c; }
    { static obj c[] = { obj_from_case(32) }; cx_typeassert_2Dprim_2Dctype = (obj)c; }
    { static obj c[] = { obj_from_case(35) }; cx_c_2Dformat_2Dprim_2Dtext_2A = (obj)c; }
    { static obj c[] = { obj_from_case(66) }; cx_c_2Dstringify_2Dtext = (obj)c; }
    { static obj c[] = { obj_from_case(74) }; cx_c_2Dformat_2Dprim_2A = (obj)c; }
    { static obj c[] = { obj_from_case(76) }; cx_c_2Dformat_2Dprimexp_2A = (obj)c; }
    { static obj c[] = { obj_from_case(77) }; cx_c_2Dformat_2A = (obj)c; }
    { static obj c[] = { obj_from_case(78) }; cx_c_2Dmangle = (obj)c; }
    { static obj c[] = { obj_from_case(83) }; cx_c_2Dargref_2Dctype = (obj)c; }
    { static obj c[] = { obj_from_case(87) }; cx_c_2Dundecorate_2Dalvar = (obj)c; }
    { static obj c[] = { obj_from_case(90) }; cx_display_2Dtext = (obj)c; }
    { static obj c[] = { obj_from_case(92) }; cx_path_2Dstrip_2Ddirectory = (obj)c; }
    { static obj c[] = { obj_from_case(93) }; cx_path_2Dstrip_2Dextension = (obj)c; }
    { static obj c[] = { obj_from_case(94) }; cx_cleanup_2Dc_2Dcode_21 = (obj)c; }
    cx_code_2Dstandard_2Dincludes = (cx__23999);
    cx_code_2Dstandard_2Ddefinitions = (cx__231001);
    cx_code_2Dhost_2Dprologue = (cx__231003);
    cx_code_2Dhost_2Depilogue = (cx__231005);
    cx_code_2Dmodule = (cx__231007);
    cx_code_2Druntime = (cx__231009);
    cx_code_2Dmain = (cx__231011);
    { static obj c[] = { obj_from_case(95) }; cx_stack_2Dfunction_2Dcode_2Dgenerate = (obj)c; }
    { static obj c[] = { obj_from_case(242) }; cx_stack_2Dfunctions_2Dcode_2Dgenerate = (obj)c; }
    r[0] = obj_from_void(0);
    r[1+0] = r[0];
    pc = 0; /* exit from module init */
    r[1+1] = r[0];  
    r += 1; /* shift reg wnd */
    assert(rc = 2);
    goto jump;

case 1: /* intersectionq#1133 k s1 s2 */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_intersectionq_231133: 
s_intersectionq: /* k s1 s2 */
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    { /* memq */
    obj x = (r[3]), l = r[2];  
    for (; l != mknull(); l = cdr(l)) if (car(l) == x) break;
    r[3] = (l == mknull() ? obj_from_bool(0) : l); }
  if (bool_from_obj(r[3])) {
    r[3] = (cdr((r[1])));
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(2);
    r[4] = (hendblk(2+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_intersectionq;
  } else {
    r[3] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[3];  
    /* r[2] */    
    goto s_intersectionq;
  }
  }

case 2: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k s1 */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = (car((r[3])));
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

case 3: /* begin-exp?#1627 k exp */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_begin_2Dexp_3F_231627: /* k exp */
  if (bool_from_obj(cxs_let_2Dexp_3F_231575((r[1])))) {
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
    *--hp = obj_from_case(4);
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

case 4: /* clo ek r */
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

case 5: /* string-starts-with? k str sstr */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k str sstr */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_string_2Dstarts_2Dwith_3F((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 6: /* string-ends-with? k str sstr */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k str sstr */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_string_2Dends_2Dwith_3F((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 7: /* string-contains-at? k str pos sstr */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k str pos sstr */
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = (cxs_string_2Dcontains_2Dat_3F((r[1]), (r[2]), (r[3])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 8: /* string-contains? k str sstr */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k str sstr */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_string_2Dcontains_3F((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 9: /* substring-before k str sstr */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_substring_2Dbefore: /* k str sstr */
  if (((stringlen((r[2]))) <= (stringlen((r[1]))))) {
    r[3+0] = r[0];  
    r[3+1] = obj_from_fixnum(0);
    r[3+2] = r[2];  
    r[3+3] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
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

s_loop: /* k pos sstr str */
  if (((stringlen((r[2]))) <= ((stringlen((r[3]))) - fixnum_from_obj(r[1])))) {
  if (bool_from_obj(cxs_string_2Dcontains_2Dat_3F((r[3]), (r[1]), (r[2])))) {
    { /* substring */
    int *d = substring(stringdata((r[3])), (0), fixnum_from_obj(r[1]));
    r[4] = (hpushstr(4, d)); }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    r[1] = obj_from_fixnum(fixnum_from_obj(r[1]) + (1));
    /* r[2] */    
    /* r[3] */    
    goto s_loop;
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

case 10: /* substring-after k str sstr */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_substring_2Dafter: /* k str sstr */
    r[3] = obj_from_fixnum(stringlen((r[1])));
    r[4] = obj_from_fixnum(stringlen((r[2])));
  if ((fixnum_from_obj(r[4]) <= fixnum_from_obj(r[3]))) {
    r[5+0] = r[0];  
    r[5+1] = obj_from_fixnum(0);
    r[5+2] = r[1];  
    r[5+3] = r[2];  
    r[5+4] = r[4];  
    r[5+5] = r[3];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v6019;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop_v6019: /* k pos str sstr ssl sl */
  if ((fixnum_from_obj(r[4]) <= (fixnum_from_obj(r[5]) - fixnum_from_obj(r[1])))) {
  if (bool_from_obj(cxs_string_2Dcontains_2Dat_3F((r[2]), (r[1]), (r[3])))) {
    { /* substring */
    int *d = substring(stringdata((r[2])), (fixnum_from_obj(r[1]) + fixnum_from_obj(r[4])), fixnum_from_obj(r[5]));
    r[6] = (hpushstr(6, d)); }
    r[7+0] = r[0];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    /* r[0] */    
    r[1] = obj_from_fixnum(fixnum_from_obj(r[1]) + (1));
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v6019;
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

case 11: /* substring-before-suffix k str sstr */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_substring_2Dbefore_2Dsuffix: /* k str sstr */
  if (bool_from_obj(cxs_string_2Dends_2Dwith_3F((r[1]), (r[2])))) {
    { /* substring */
    int *d = substring(stringdata((r[1])), (0), ((stringlen((r[1]))) - (stringlen((r[2])))));
    r[3] = (hpushstr(3, d)); }
  } else {
    r[3] = obj_from_bool(0);
  }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 12: /* substring-after-prefix k str sstr */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_substring_2Dafter_2Dprefix: /* k str sstr */
  if (bool_from_obj(cxs_string_2Dstarts_2Dwith_3F((r[1]), (r[2])))) {
    { /* substring */
    int *d = substring(stringdata((r[1])), (stringlen((r[2]))), (stringlen((r[1]))));
    r[3] = (hpushstr(3, d)); }
  } else {
    r[3] = obj_from_bool(0);
  }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 13: /* string-member? k c str */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c str */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_string_2Dmember_3F((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 14: /* string-member-ci? k c str */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k c str */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_string_2Dmember_2Dci_3F((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 15: /* string-span k str cs */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k str cs */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_string_2Dspan((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 16: /* string-span-ci k str cs */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k str cs */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_string_2Dspan_2Dci((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 17: /* substring-after-span k str cs */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_substring_2Dafter_2Dspan: /* k str cs */
    r[3] = (cxs_string_2Dspan((r[1]), (r[2])));
  if ((fixnum_from_obj(r[3]) == 0)) {
    r[3] = r[1];  
  } else {
    { /* substring */
    int *d = substring(stringdata((r[1])), fixnum_from_obj(r[3]), (stringlen((r[1]))));
    r[3] = (hpushstr(4, d)); }
  }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 18: /* string-reverse k str */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k str */
    { /* string->list */
    int c = (stringlen((r[1])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 2); /* 2 live regs */
    s = stringchars((r[1])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[2] = (l); }
    { fixnum_t v6138_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6138_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6138_tmp);
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[2] = (o); } }
    { fixnum_t v6139_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6139_tmp = (n); }
    { /* list->string */
    int i, c = (v6139_tmp); 
    obj o = hpushstr(3, allocstring(c, ' ')); /* 3 live regs */
    obj l = r[2];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[2] = (o); } }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 19: /* string-upcase k str */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_string_2Dupcase: /* k str */
    { /* string->list */
    int c = (stringlen((r[1])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 2); /* 2 live regs */
    s = stringchars((r[1])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[2] = (l); }
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(21);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v5940;

s_loop_v5940: /* k id */
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
    *--hp = obj_from_case(20);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v5940;
  }

case 20: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = obj_from_char(toupper(char_from_obj(r[4])));
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

case 21: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { fixnum_t v6137_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6137_tmp = (n); }
    { /* list->string */
    int i, c = (v6137_tmp); 
    obj o = hpushstr(3, allocstring(c, ' ')); /* 3 live regs */
    obj l = r[1];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[3] = (o); } }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 22: /* primexp-ctype k prim */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_primexp_2Dctype: /* k prim */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(23);
    r[2] = (hendblk(1+1));
    r[3+0] = r[2];  
    r[3+1] = r[1];  
    r[3+2] = (cx__23208);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_substring_2Dbefore;

case 23: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
  if (bool_from_obj(r[1])) {
    r[3] = (cxs_string_2Dspan_2Dci((r[1]), (cx__23210)));
    r[3] = obj_from_bool((stringlen((r[1]))) == fixnum_from_obj(r[3]));
  if (bool_from_obj(r[3])) {
    r[3] = r[1];  
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 24: /* prim-ctype k prim */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k prim */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(25);
    r[2] = (hendblk(2+1));
    r[3+0] = r[2];  
    r[3+1] = r[1];  
    r[3+2] = (cx__23214);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_substring_2Dafter;

case 25: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r prim k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(26);
    r[4] = (hendblk(2+1));
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    /* r[1] */    
    r[2] = (cx__23216);
    goto gs_substring_2Dafter_2Dspan;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    /* r[2] */    
    /* r[3] */    
    goto s_l_v5901;
  }

case 26: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v5901: /* ek r prim k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(27);
    r[4] = (hendblk(2+1));
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    /* r[1] */    
    goto gs_primexp_2Dctype;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    /* r[2] */    
    /* r[3] */    
    goto s_l_v5903;
  }

case 27: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_l_v5903: /* ek r prim k */
  if (bool_from_obj(r[1])) {
    r[4+0] = r[3];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[3];  
    r[1] = r[2];  
    goto gs_primexp_2Dctype;
  }

case 28: /* prim-cexp? k prim */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_prim_2Dcexp_3F: /* k prim */
    r[2] = (cxs_string_2Dcontains_3F((r[1]), (cx__23214)));
    r[2] = obj_from_bool(!bool_from_obj(r[2]));
  if (bool_from_obj(r[2])) {
    /* r[0] */    
    /* r[1] */    
    goto gs_primexp_2Dctype;
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

case 29: /* typecheck-prim-ctype k prim */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k prim */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(30);
    r[2] = (hendblk(1+1));
    r[3+0] = r[2];  
    r[3+1] = r[1];  
    r[3+2] = (cx__23224);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_substring_2Dafter;

case 30: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(31);
    r[3] = (hendblk(1+1));
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    /* r[1] */    
    r[2] = (cx__23226);
    goto gs_substring_2Dbefore_2Dsuffix;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    /* r[2] */    
    goto s_l_v5877;
  }

case 31: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_l_v5877: /* ek r k */
  if (bool_from_obj(r[1])) {
    r[3] = (cxs_string_2Dspan_2Dci((r[1]), (cx__23210)));
    r[3] = obj_from_bool((stringlen((r[1]))) == fixnum_from_obj(r[3]));
  if (bool_from_obj(r[3])) {
    r[3] = r[1];  
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 32: /* typeassert-prim-ctype k prim */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k prim */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(33);
    r[2] = (hendblk(1+1));
    r[3+0] = r[2];  
    r[3+1] = r[1];  
    r[3+2] = (cx__23231);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_substring_2Dafter;

case 33: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(34);
    r[3] = (hendblk(1+1));
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    /* r[1] */    
    r[2] = (cx__23233);
    goto gs_substring_2Dbefore_2Dsuffix;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    /* r[2] */    
    goto s_l_v5859;
  }

case 34: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_l_v5859: /* ek r k */
  if (bool_from_obj(r[1])) {
    r[3] = (cxs_string_2Dspan_2Dci((r[1]), (cx__23210)));
    r[3] = obj_from_bool((stringlen((r[1]))) == fixnum_from_obj(r[3]));
  if (bool_from_obj(r[3])) {
    r[3] = r[1];  
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 35: /* c-format-prim-text* k ltext rtext fmt all-args */
    assert(rc == 6);
    r += 1; /* shift reg. wnd */
gs_c_2Dformat_2Dprim_2Dtext_2A: /* k ltext rtext fmt all-args */
    { /* string->list */
    int c = (stringlen((r[3])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    s = stringchars((r[3])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[5] = (l); }
    hreserve(hbsz(0+1), 6); /* 6 live regs */
    *--hp = obj_from_case(38);
    r[6] = (hendblk(0+1));
    hreserve(hbsz(0+1), 7); /* 7 live regs */
    *--hp = obj_from_case(36);
    r[7] = (hendblk(0+1));
    hreserve(hbsz(0+1), 8); /* 8 live regs */
    *--hp = obj_from_case(37);
    r[8] = (hendblk(0+1));
    r[9+0] = r[0];  
    r[9+1] = r[5];  
    r[9+2] = r[4];  
    r[9+3] = (mknull());
    r[9+4] = r[6];  
    r[9+5] = r[2];  
    r[9+6] = r[1];  
    r[9+7] = r[7];  
    r[9+8] = r[8];  
    r[9+9] = r[4];  
    r[9+10] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5633;

case 36: /* clo k args */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k args */
  if ((isnull((r[1])))) {
    r[2+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = r[0];  
    r[2+2] = (cx__23584);
    r[2+3] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
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

case 37: /* clo k lst chars */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k lst chars */
    /* r[0] */    
    /* r[1] */    
    /* r[2] */    
    goto s_loop_v5825;

s_loop_v5825: /* k lst chars */
  if (((isnull((r[2]))) && (isnull((r[1]))))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[1])))) {
    { bool_t v6136_tmp;
    r[3] = (car((r[1])));
    v6136_tmp = (is_char_obj(r[3]));
    r[3] = obj_from_bool(!(v6136_tmp)); }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isnull((r[2])))) {
  if ((ispair((r[1])))) {
    r[3] = (car((r[1])));
    r[3] = obj_from_bool(isalpha(char_from_obj(r[3])));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isnull((r[2])))) {
  if ((ispair((r[1])))) {
    r[3] = (car((r[1])));
    r[3] = obj_from_bool(isdigit(char_from_obj(r[3])));
  } else {
    r[3] = obj_from_bool(0);
  }
  } else {
    r[3] = obj_from_bool(0);
  }
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isnull((r[2])))) {
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isnull((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    r[4] = (car((r[2])));
    r[3] = obj_from_bool(char_from_obj(r[3]) == char_from_obj(r[4]));
  if (bool_from_obj(r[3])) {
    r[3] = (cdr((r[1])));
    r[4] = (cdr((r[2])));
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v5825;
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
  }
  }
  }
  }
  }

case 38: /* clo k flist2 args2 text */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k flist2 args2 text */
  if ((ispair((r[2])))) {
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(39);
    r[4] = (hendblk(2+1));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[6+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = (cx__23591);
    r[6+3] = r[5];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 39: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  k text */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 40: /* clo k flist args buf k */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4];
    r[1+9] = p[5];
    r[1+10] = p[6]; }
    r += 1; /* shift reg. wnd */
s_loop_v5633: /* k flist args buf k rtext ltext expect-arg match-name all-args fmt */
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(40);
    r[11] = (hendblk(6+1));
  if ((isnull((r[1])))) {
    { fixnum_t v6135_tmp;
    { /* length */
    int n; obj l = r[3];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6135_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6135_tmp);
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    l = r[3];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[12] = (o); } }
    r[13+0] = r[4];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = r[0];  
    r[13+2] = (mknull());
    r[13+3] = r[2];  
    r[13+4] = (r[12]);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    { bool_t v6134_tmp;
    r[12] = (car((r[1])));
    v6134_tmp = (char_from_obj(r[12]) == (36));
    r[12] = obj_from_bool(!(v6134_tmp)); }
  if (bool_from_obj(r[12])) {
    r[12] = (cdr((r[1])));
    r[13] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[3];  
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    /* r[0] */    
    r[1] = (r[12]);
    /* r[2] */    
    r[3] = (r[13]);
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    r[10] = (r[10]);
    goto s_loop_v5633;
  } else {
    r[12] = (cdr((r[1])));
    r[12] = obj_from_bool(isnull((r[12])));
  if (bool_from_obj(r[12])) {
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (mknull());
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    r[13+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = r[0];  
    r[13+2] = (cx__23539);
    r[13+3] = (r[12]);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(11+1), 12); /* 12 live regs */
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = r[9];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[11]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_case(41);
    r[12] = (hendblk(11+1));
    r[13+0] = r[8];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = (r[12]);
    r[13+2] = (cdr((r[1])));
    r[13+3] = (cx__23248);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }
  }

case 41: /* clo ek r */
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
    /* ek r flist rtext ltext expect-arg match-name loop k buf all-args args k */
  if (bool_from_obj(r[1])) {
    { /* length */
    int n; obj l = (r[10]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    r[13] = obj_from_fixnum(n); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[7];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[12]);
    r[14+2] = r[1];  
    r[14+3] = (r[11]);
    r[14+4] = (r[13]);
    r[14+5] = r[8];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
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
    *--hp = obj_from_case(42);
    r[13] = (hendblk(11+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cdr((r[2])));
    r[3] = (cx__23259);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 42: /* clo ek r */
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
    /* ek r flist rtext ltext expect-arg match-name loop k buf all-args args k */
  if (bool_from_obj(r[1])) {
    { fixnum_t v6133_tmp;
    { fixnum_t v6132_tmp;
    { /* length */
    int n; obj l = (r[10]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6133_tmp = (n); }
    { fixnum_t v6131_tmp;
    { /* length */
    int n; obj l = (r[11]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6131_tmp = (n); }
    v6132_tmp = ((v6131_tmp) + (1)); }
    r[13] = obj_from_fixnum((v6133_tmp) - (v6132_tmp)); } }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[7];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[12]);
    r[14+2] = r[1];  
    r[14+3] = (r[11]);
    r[14+4] = (r[13]);
    r[14+5] = r[8];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
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
    *--hp = obj_from_case(43);
    r[13] = (hendblk(11+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cdr((r[2])));
    r[3] = (cx__23274);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 43: /* clo ek r */
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
    /* ek r flist rtext ltext expect-arg match-name loop k buf all-args args k */
  if (bool_from_obj(r[1])) {
    { fixnum_t v6130_tmp;
    { fixnum_t v6129_tmp;
    { /* length */
    int n; obj l = (r[10]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6130_tmp = (n); }
    { fixnum_t v6128_tmp;
    { /* length */
    int n; obj l = (r[11]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6128_tmp = (n); }
    v6129_tmp = ((v6128_tmp) + (-1)); }
    r[13] = obj_from_fixnum((v6130_tmp) - (v6129_tmp)); } }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[7];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[12]);
    r[14+2] = r[1];  
    r[14+3] = (r[11]);
    r[14+4] = (r[13]);
    r[14+5] = r[8];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
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
    *--hp = obj_from_case(44);
    r[13] = (hendblk(11+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cdr((r[2])));
    r[3] = (cx__23289);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 44: /* clo ek r */
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
    /* ek r flist rtext ltext expect-arg match-name loop k buf all-args args k */
  if (bool_from_obj(r[1])) {
    { fixnum_t v6127_tmp;
    { fixnum_t v6126_tmp;
    { /* length */
    int n; obj l = (r[10]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6127_tmp = (n); }
    { /* length */
    int n; obj l = (r[11]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6126_tmp = (n); }
    r[13] = obj_from_fixnum((v6127_tmp) - (v6126_tmp)); } }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[7];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[12]);
    r[14+2] = r[1];  
    r[14+3] = (r[11]);
    r[14+4] = (r[13]);
    r[14+5] = r[8];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(10+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[9];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(45);
    r[13] = (hendblk(10+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cdr((r[2])));
    r[3] = (cx__23300);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 45: /* clo ek r */
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
    /* ek r flist rtext ltext expect-arg match-name buf loop k args k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(7+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[1];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(46);
    r[12] = (hendblk(7+1));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[7];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = r[2];  
    *--hp = obj_from_case(53);
    r[12] = (hendblk(10+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cdr((r[2])));
    r[3] = (cx__23309);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 46: /* clo ek  */
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
    /* ek  match-name buf loop k args r k */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(47);
    r[9] = (hendblk(7+1));
    r[10] = (car((r[6])));
    r[10] = obj_from_bool(isstring((r[10])));
  if (bool_from_obj(r[10])) {
    r[0] = r[9];  
    r[1] = (car((r[6])));
    goto gs_primexp_2Dctype;
  } else {
    r[0] = obj_from_ktrap();
    r[1] = obj_from_bool(0);
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    goto s_l_v5738;
  }

case 47: /* clo ek r */
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
s_l_v5738: /* ek r match-name buf loop k args r k */
  if (bool_from_obj(r[1])) {
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__23420)));
    r[9] = (hpushstr(9, d)); }
    { /* string->list */
    int c = (stringlen((r[9])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    s = stringchars((r[9])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[9] = (l); }
    { fixnum_t v6124_tmp;
    { /* length */
    int n; obj l = r[9];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6124_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6124_tmp);
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    l = r[9];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[9] = (o); } }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[1])) {
    { /* string->list */
    int c = (stringlen((cx__23428)));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 10); /* 10 live regs */
    s = stringchars((cx__23428)); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[10] = (l); }
    { fixnum_t v6125_tmp;
    { /* length */
    int n; obj l = (r[10]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6125_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6125_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = (r[10]); /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[10] = (o); } }
  } else {
    r[10] = obj_from_bool(0);
  }
    hreserve(hbsz(9+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (r[10]);
    *--hp = r[2];  
    *--hp = obj_from_case(48);
    r[11] = (hendblk(9+1));
  if (bool_from_obj(r[9])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[3];  
    r[3] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[2];  
    r[12+3] = (r[10]);
    r[12+4] = r[1];  
    r[12+5] = r[3];  
    r[12+6] = r[4];  
    r[12+7] = r[5];  
    r[12+8] = r[6];  
    r[12+9] = r[7];  
    r[12+10] = r[8];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5740;
  }

case 48: /* clo ek r */
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
s_l_v5740: /* ek r match-name vchars r buf loop k args r k */
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = r[1];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(49);
    r[11] = (hendblk(8+1));
  if (bool_from_obj(r[3])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[5];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[4];  
    r[12+3] = r[5];  
    r[12+4] = r[6];  
    r[12+5] = r[7];  
    r[12+6] = r[8];  
    r[12+7] = r[9];  
    r[12+8] = (r[10]);
    r[12+9] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5742;
  }

case 49: /* clo ek r */
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
s_l_v5742: /* ek r r buf loop k args r k r */
    r[10] = (bool_from_obj(r[9]) ? (r[9]) : (r[1]));
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(50);
    r[11] = (hendblk(8+1));
  if (bool_from_obj(r[10])) {
    { /* string-append */
    int *d = stringcat(stringdata((r[2])), stringdata((cx__23208)));
    r[12] = (hpushstr(12, d)); }
    r[0] = (r[11]);
    r[1] = (car((r[6])));
    r[2] = (r[12]);
    goto gs_substring_2Dafter;
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[3];  
    r[12+3] = r[4];  
    r[12+4] = r[5];  
    r[12+5] = r[1];  
    r[12+6] = r[6];  
    r[12+7] = r[7];  
    r[12+8] = r[8];  
    r[12+9] = (r[10]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5744;
  }

case 50: /* clo ek r */
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
s_l_v5744: /* ek r buf loop k r args r k sbuf */
    hreserve(hbsz(9+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(51);
    r[10] = (hendblk(9+1));
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(1+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = obj_from_case(52);
    r[11] = (hendblk(1+1));
    r[0] = (r[11]);
    /* r[1] */    
    r[2] = (cx__23448);
    goto gs_substring_2Dafter_2Dspan;
  } else {
    r[11+0] = obj_from_ktrap();
    r[11+1] = obj_from_bool(0);
    r[11+2] = r[2];  
    r[11+3] = r[3];  
    r[11+4] = r[4];  
    r[11+5] = r[5];  
    r[11+6] = r[6];  
    r[11+7] = r[7];  
    r[11+8] = r[8];  
    r[11+9] = r[1];  
    r[11+10] = r[9];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5752;
  }

case 51: /* clo ek r */
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
s_l_v5752: /* ek r buf loop k r args r k r sbuf */
  if (bool_from_obj(bool_from_obj(r[10]) ? (bool_from_obj(r[1]) ? (r[1]) : (r[9])) : obj_from_bool(0))) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (bool_from_obj(r[5]) ? (cx__23456) : obj_from_char(40));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (bool_from_obj(r[1]) ? (r[1]) : (r[9]));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[12+0] = r[3];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[8];  
    r[12+2] = r[7];  
    r[12+3] = (cdr((r[6])));
    r[12+4] = (r[11]);
    r[12+5] = r[4];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    r[11] = (car((r[6])));
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = r[2];  
    *--hp = (r[11]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[12+0] = r[3];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[8];  
    r[12+2] = r[7];  
    r[12+3] = (cdr((r[6])));
    r[12+4] = (r[11]);
    r[12+5] = r[4];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 52: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[0] = r[2];  
    /* r[1] */    
    r[2] = (cx__23447);
    goto gs_substring_2Dafter_2Dprefix;

case 53: /* clo ek r */
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
    /* ek r flist match-name rtext ltext expect-arg loop k buf args k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(6+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[1];  
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(54);
    r[12] = (hendblk(6+1));
    r[0] = r[6];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(9+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(55);
    r[12] = (hendblk(9+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (cdr((r[2])));
    r[3] = (cx__23314);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 54: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop k buf args r k */
    r[8] = (car((r[5])));
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[4];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[9+0] = r[2];  
    pc = objptr_from_obj(r[9+0])[0];
    r[9+1] = r[7];  
    r[9+2] = r[6];  
    r[9+3] = (cdr((r[5])));
    r[9+4] = r[8];  
    r[9+5] = r[3];  
    r += 9; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 55: /* clo ek r */
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
    /* ek r flist match-name rtext loop k buf ltext args k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[12+0] = r[5];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[10]);
    r[12+2] = r[1];  
    r[12+3] = r[9];  
    r[12+4] = (r[11]);
    r[12+5] = r[6];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[4];  
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(56);
    r[11] = (hendblk(8+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = (cdr((r[2])));
    r[3] = (cx__23325);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 56: /* clo ek r */
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
    /* ek r flist match-name loop k buf args rtext k */
  if (bool_from_obj(r[1])) {
    r[0] = r[9];  
    /* r[1] */    
    r[2] = r[3];  
    r[3] = r[4];  
    r[4] = r[5];  
    r[5] = r[6];  
    r[6] = r[7];  
    r[7] = r[8];  
    goto s_skipws;
  } else {
    r[10] = (cdr((r[2])));
    r[10] = (car((r[10])));
    r[10] = obj_from_bool(char_from_obj(r[10]) == (36));
  if (bool_from_obj(r[10])) {
    r[10] = (cdr((r[2])));
    r[10] = (cdr((r[10])));
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (cx__23401);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    r[3] = r[7];  
    r[4] = (r[11]);
    /* r[5] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    r[10] = (cdr((r[2])));
    r[10] = (car((r[10])));
    r[10] = obj_from_bool(char_from_obj(r[10]) == (125));
  if (bool_from_obj(r[10])) {
    r[10] = (cdr((r[2])));
    r[10] = (cdr((r[10])));
    { fixnum_t v6123_tmp;
    { /* length */
    int n; obj l = r[6];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6123_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6123_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = r[6];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[11] = (o); } }
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (r[10]);
    r[3] = r[7];  
    r[4] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[10] = (cdr((r[2])));
    r[10] = (car((r[10])));
    r[10] = obj_from_bool(char_from_obj(r[10]) == (123));
  if (bool_from_obj(r[10])) {
  if ((isnull((r[7])))) {
    r[10] = (cdr((r[2])));
    r[10] = (cdr((r[10])));
    hreserve(hbsz(3+1), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(63);
    r[11] = (hendblk(3+1));
    r[12+0] = r[4];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[9];  
    r[12+2] = (r[10]);
    r[12+3] = (cx__23361);
    r[12+4] = (mknull());
    r[12+5] = (r[11]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    r[10] = (cdr((r[2])));
    r[10] = (cdr((r[10])));
    hreserve(hbsz(5+1), 11); /* 11 live regs */
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = obj_from_case(64);
    r[11] = (hendblk(5+1));
    r[12+0] = r[4];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = r[9];  
    r[12+2] = (r[10]);
    r[12+3] = r[7];  
    r[12+4] = (mknull());
    r[12+5] = (r[11]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }
  } else {
    { fixnum_t v6122_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6122_tmp = (n); }
    { /* list->string */
    int i, c = (v6122_tmp); 
    obj o = hpushstr(10, allocstring(c, ' ')); /* 10 live regs */
    obj l = r[2];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[10] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (mknull());
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[11+0])[0];
    r[11+1] = r[9];  
    r[11+2] = (cx__23344);
    r[11+3] = (r[10]);
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }
  }
  }

s_skipws: /* k flist1 match-name loop k buf args rtext */
  if ((isnull((r[1])))) {
    r[8] = obj_from_bool(isnull((r[1])));
  } else {
    { bool_t v6121_tmp;
    r[8] = (car((r[1])));
    v6121_tmp = (isspace(char_from_obj(r[8])));
    r[8] = obj_from_bool(!(v6121_tmp)); }
  }
  if (bool_from_obj(r[8])) {
    { fixnum_t v6120_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6120_tmp = (n); }
    { /* list->string */
    int i, c = (v6120_tmp); 
    obj o = hpushstr(8, allocstring(c, ' ')); /* 8 live regs */
    obj l = r[1];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[8] = (o); } }
    hreserve(hbsz(9+1), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[0];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = obj_from_case(57);
    r[9] = (hendblk(9+1));
    r[0] = r[9];  
    r[1] = r[8];  
    goto gs_primexp_2Dctype;
  } else {
    r[8] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[8];  
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    goto s_skipws;
  }

case 57: /* clo ek r */
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
    /* ek r match-name fstr flist1 loop k buf args k rtext */
  if (bool_from_obj(r[1])) {
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__23420)));
    r[11] = (hpushstr(11, d)); }
    { /* string->list */
    int c = (stringlen((r[11])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    s = stringchars((r[11])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[11] = (l); }
    { fixnum_t v6117_tmp;
    { /* length */
    int n; obj l = (r[11]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6117_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6117_tmp);
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    l = (r[11]); /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[11] = (o); } }
  } else {
    r[11] = obj_from_bool(0);
  }
  if (bool_from_obj(r[1])) {
    { /* string->list */
    int c = (stringlen((cx__23428)));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 12); /* 12 live regs */
    s = stringchars((cx__23428)); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[12] = (l); }
    { fixnum_t v6118_tmp;
    { /* length */
    int n; obj l = (r[12]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6118_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6118_tmp);
    hreserve(hbsz(3)*c, 13); /* 13 live regs */
    l = (r[12]); /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[12] = (o); } }
  } else {
    r[12] = obj_from_bool(0);
  }
  if (bool_from_obj(r[1])) {
  if ((isstring((r[10])))) {
    { /* string->list */
    int c = (stringlen((r[10])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 13); /* 13 live regs */
    s = stringchars((r[10])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[13] = (l); }
    { fixnum_t v6119_tmp;
    { /* length */
    int n; obj l = (r[13]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6119_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6119_tmp);
    hreserve(hbsz(3)*c, 14); /* 14 live regs */
    l = (r[13]); /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[13] = (o); } }
  } else {
    r[13] = obj_from_bool(0);
  }
  } else {
    r[13] = obj_from_bool(0);
  }
    hreserve(hbsz(12+1), 14); /* 14 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = (r[10]);
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = r[2];  
    *--hp = obj_from_case(58);
    r[14] = (hendblk(12+1));
  if (bool_from_obj(r[13])) {
  if (bool_from_obj(r[11])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[14]);
    r[2] = (r[13]);
    r[3] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[15+0] = obj_from_ktrap();
    r[15+1] = obj_from_bool(0);
    r[15+2] = r[2];  
    r[15+3] = (r[12]);
    r[15+4] = (r[13]);
    r[15+5] = r[1];  
    r[15+6] = r[3];  
    r[15+7] = (r[10]);
    r[15+8] = r[4];  
    r[15+9] = r[5];  
    r[15+10] = r[6];  
    r[15+11] = r[7];  
    r[15+12] = r[8];  
    r[15+13] = r[9];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5680;
  }
  } else {
    r[15+0] = obj_from_ktrap();
    r[15+1] = obj_from_bool(0);
    r[15+2] = r[2];  
    r[15+3] = (r[12]);
    r[15+4] = (r[13]);
    r[15+5] = r[1];  
    r[15+6] = r[3];  
    r[15+7] = (r[10]);
    r[15+8] = r[4];  
    r[15+9] = r[5];  
    r[15+10] = r[6];  
    r[15+11] = r[7];  
    r[15+12] = r[8];  
    r[15+13] = r[9];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5680;
  }

case 58: /* clo ek r */
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
    r[1+12] = p[11];
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
s_l_v5680: /* ek r match-name vchars rtb r fstr rtext flist1 loop k buf args k */
    hreserve(hbsz(10+1), 14); /* 14 live regs */
    *--hp = r[1];  
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(59);
    r[14] = (hendblk(10+1));
  if (bool_from_obj(r[4])) {
  if (bool_from_obj(r[3])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[14]);
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[15+0] = obj_from_ktrap();
    r[15+1] = obj_from_bool(0);
    r[15+2] = r[5];  
    r[15+3] = r[6];  
    r[15+4] = r[7];  
    r[15+5] = r[8];  
    r[15+6] = r[9];  
    r[15+7] = (r[10]);
    r[15+8] = (r[11]);
    r[15+9] = (r[12]);
    r[15+10] = (r[13]);
    r[15+11] = r[1];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5682;
  }
  } else {
    r[15+0] = obj_from_ktrap();
    r[15+1] = obj_from_bool(0);
    r[15+2] = r[5];  
    r[15+3] = r[6];  
    r[15+4] = r[7];  
    r[15+5] = r[8];  
    r[15+6] = r[9];  
    r[15+7] = (r[10]);
    r[15+8] = (r[11]);
    r[15+9] = (r[12]);
    r[15+10] = (r[13]);
    r[15+11] = r[1];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5682;
  }

case 59: /* clo ek r */
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
s_l_v5682: /* ek r r fstr rtext flist1 loop k buf args k r */
    r[12] = (bool_from_obj(r[11]) ? (r[11]) : (r[1]));
    hreserve(hbsz(9+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(60);
    r[13] = (hendblk(9+1));
  if (bool_from_obj(r[12])) {
    { /* string-append */
    int *d = stringcat(stringdata((r[2])), stringdata((cx__23208)));
    r[14] = (hpushstr(14, d)); }
    r[0] = (r[13]);
    r[1] = r[3];  
    r[2] = (r[14]);
    goto gs_substring_2Dafter;
  } else {
    r[14+0] = obj_from_ktrap();
    r[14+1] = obj_from_bool(0);
    r[14+2] = r[4];  
    r[14+3] = r[5];  
    r[14+4] = r[6];  
    r[14+5] = r[7];  
    r[14+6] = r[1];  
    r[14+7] = r[8];  
    r[14+8] = r[9];  
    r[14+9] = (r[10]);
    r[14+10] = (r[12]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5684;
  }

case 60: /* clo ek r */
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
s_l_v5684: /* ek r rtext flist1 loop k r buf args k srtb */
    hreserve(hbsz(10+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(61);
    r[11] = (hendblk(10+1));
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(1+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = obj_from_case(62);
    r[12] = (hendblk(1+1));
    r[0] = (r[12]);
    /* r[1] */    
    r[2] = (cx__23448);
    goto gs_substring_2Dafter_2Dspan;
  } else {
    r[12+0] = obj_from_ktrap();
    r[12+1] = obj_from_bool(0);
    r[12+2] = r[2];  
    r[12+3] = r[3];  
    r[12+4] = r[4];  
    r[12+5] = r[5];  
    r[12+6] = r[6];  
    r[12+7] = r[7];  
    r[12+8] = r[8];  
    r[12+9] = r[9];  
    r[12+10] = r[1];  
    r[12+11] = (r[10]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5692;
  }

case 61: /* clo ek r */
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
s_l_v5692: /* ek r rtext flist1 loop k r buf args k r srtb */
  if (bool_from_obj(bool_from_obj(r[11]) ? (bool_from_obj(r[1]) ? (r[1]) : (r[10])) : obj_from_bool(0))) {
    r[12] = (bool_from_obj(r[1]) ? (r[1]) : (r[10]));
    { /* string->list */
    int c = (stringlen((r[12])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 13); /* 13 live regs */
    s = stringchars((r[12])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[12] = (l); }
    { fixnum_t v6116_tmp;
    { /* length */
    int n; obj l = (r[11]);
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6116_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6116_tmp);
    hreserve(hbsz(3)*c, 13); /* 13 live regs */
    l = (r[11]); /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[13] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = r[7];  
    *--hp = (r[13]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (bool_from_obj(r[6]) ? (cx__23456) : obj_from_char(40));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[13] = (hendblk(3)); }
    r[14+0] = r[4];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = r[9];  
    r[14+2] = (r[12]);
    r[14+3] = r[8];  
    r[14+4] = (r[13]);
    r[14+5] = r[5];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = r[7];  
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    r[13+0] = r[4];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = r[9];  
    r[13+2] = r[3];  
    r[13+3] = r[8];  
    r[13+4] = (r[12]);
    r[13+5] = r[5];  
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 62: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[0] = r[2];  
    /* r[1] */    
    r[2] = (cx__23447);
    goto gs_substring_2Dafter_2Dprefix;

case 63: /* clo k flist2 args2 text */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* k flist2 args2 text loop k buf */
    r[7+0] = r[4];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[0];  
    r[7+2] = r[1];  
    r[7+3] = (mknull());
    r[7+4] = r[6];  
    r[7+5] = r[5];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 64: /* clo k flist2 args2 text */
    assert(rc == 5);
    { obj* p = objptr_from_obj(r[0]);
    r[1+4] = p[1];
    r[1+5] = p[2];
    r[1+6] = p[3];
    r[1+7] = p[4];
    r[1+8] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* k flist2 args2 text args loop k buf flist */
    hreserve(hbsz(7+1), 9); /* 9 live regs */
    *--hp = r[0];  
    *--hp = r[8];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(65);
    r[9] = (hendblk(7+1));
  if (((r[4]) == (r[2]))) {
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = (cx__23351);
    r[3] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[10+0] = obj_from_ktrap();
    r[10+1] = obj_from_void(0);
    r[10+2] = r[5];  
    r[10+3] = r[6];  
    r[10+4] = r[7];  
    r[10+5] = r[3];  
    r[10+6] = r[2];  
    r[10+7] = r[8];  
    r[10+8] = r[0];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v5653;
  }

case 65: /* clo ek  */
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
s_l_v5653: /* ek  loop k buf text args2 flist k */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
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

case 66: /* c-stringify-text k text */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_c_2Dstringify_2Dtext: /* k text */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(67);
    r[2] = (hendblk(1+1));
    r[0] = r[2];  
    /* r[1] */    
    goto s_flatten;

case 67: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_string_2Dappend_2A);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 68: /* clo k text */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
s_flatten: /* k text */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = r[1];  
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isstring((r[1])))) {
    { /* cons */ 
    hreserve(hbsz(3), 2); /* 2 live regs */
    *--hp = (mknull());
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
  } else {
  if ((is_fixnum_obj(r[1]))) {
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(69);
    r[2] = (hendblk(1+1));
  if ((is_fixnum_obj(r[1]))) {
    r[3+0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r[3+3] = obj_from_fixnum(10);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[3+0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
  if ((is_char_obj(r[1]))) {
    { /* string */
    obj o = hpushstr(2, allocstring(1, ' '));
    char *s = stringchars(o);
    *s++ = char_from_obj(r[1]);
    r[2] = (o); }
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
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
  } else {
  if ((issymbol((r[1])))) {
    r[2] = (hpushstr(2, newstring(symbolname(getsymbol((r[1]))))));
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
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
  } else {
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(68);
    r[2] = (hendblk(0+1));
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(70);
    r[2] = (hendblk(3+1));
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(73);
    r[2] = (hendblk(2+1));
    r[3+0] = (cx_list_3F);
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

case 69: /* clo ek r */
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

case 70: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r flatten k text */
  if (bool_from_obj(r[1])) {
    { fixnum_t v6115_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6115_tmp = (n); }
    { /* list->string */
    int i, c = (v6115_tmp); 
    obj o = hpushstr(5, allocstring(c, ' ')); /* 5 live regs */
    obj l = r[4];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[5] = (o); } }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[4])))) {
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[2];  
    *--hp = obj_from_case(71);
    r[5] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cdr((r[4])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
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
    r[2] = (cx__23609);
    r[3] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }

case 71: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r flatten text k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(72);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
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
    { fixnum_t v6114_tmp;
    { /* length */
    int n; obj l = r[1];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6114_tmp = (n); }
    { /* append */
    obj t, l, o, *p, *d; int c = (v6114_tmp);
    hreserve(hbsz(3)*c, 4); /* 4 live regs */
    l = r[1];   t = r[3];   /* gc-safe */
    o = t; p = &o; 
    for (; l != mknull(); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(3); p = d; }  
    r[4] = (o); } }
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
    /* ek r text k */
  if (bool_from_obj(r[1])) {
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v5584;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop_v5584: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isnull((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
    r[2] = obj_from_bool(is_char_obj(r[2]));
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_loop_v5584;
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
  }

case 74: /* c-format-prim* k ltext rtext fmt all-args */
    assert(rc == 6);
    r += 1; /* shift reg. wnd */
gs_c_2Dformat_2Dprim_2A: /* k ltext rtext fmt all-args */
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(75);
    r[5] = (hendblk(1+1));
    r[0] = r[5];  
    /* r[1] */    
    /* r[2] */    
    /* r[3] */    
    /* r[4] */    
    goto gs_c_2Dformat_2Dprim_2Dtext_2A;

case 75: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[0] = r[2];  
    /* r[1] */    
    goto gs_c_2Dstringify_2Dtext;

case 76: /* c-format-primexp* k ltext fmt all-args */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
gs_c_2Dformat_2Dprimexp_2A: /* k ltext fmt all-args */
    r[4+0] = r[0];  
    r[4+1] = r[1];  
    r[4+2] = (cx__23662);
    r[4+3] = r[2];  
    r[4+4] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_c_2Dformat_2Dprim_2A;

case 77: /* c-format* k fmt all-args */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_c_2Dformat_2A: /* k fmt all-args */
    r[3+0] = r[0];  
    r[3+1] = (cx__23667);
    r[3+2] = (cx__23666);
    r[3+3] = r[1];  
    r[3+4] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_c_2Dformat_2Dprim_2A;

case 78: /* c-mangle k str pfx */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_c_2Dmangle: /* k str pfx */
    { /* string->list */
    int c = (stringlen((r[1])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    s = stringchars((r[1])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[3] = (l); }
    r[4+0] = r[0];  
    r[4+1] = r[3];  
    r[4+2] = (mknull());
    r[4+3] = r[2];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5512;

case 79: /* clo k lst text */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5512: /* k lst text pfx */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(79);
    r[4] = (hendblk(1+1));
  if ((isnull((r[1])))) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[0];  
    *--hp = obj_from_case(80);
    r[5] = (hendblk(2+1));
    { fixnum_t v6113_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6113_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6113_tmp);
    hreserve(hbsz(3)*c, 6); /* 6 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[6] = (o); } }
    r[0] = r[5];  
    r[1] = r[6];  
    goto gs_c_2Dstringify_2Dtext;
  } else {
    r[5] = (car((r[1])));
    r[5] = obj_from_bool(isalpha(char_from_obj(r[5])));
  if (bool_from_obj(r[5])) {
    /* r[5] */    
  } else {
    r[6] = (car((r[1])));
    r[5] = obj_from_bool(isdigit(char_from_obj(r[6])));
  }
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[1])));
    r[6] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[5];  
    r[2] = r[6];  
    /* r[3] */    
    goto s_loop_v5512;
  } else {
    hreserve(hbsz(4+1), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(81);
    r[5] = (hendblk(4+1));
    r[6] = (car((r[1])));
    r[6] = obj_from_fixnum((fixnum_t)char_from_obj(r[6]));
    r[7+0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[5];  
    r[7+2] = r[6];  
    r[7+3] = obj_from_fixnum(16);
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }

case 80: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k pfx */
    { /* string-append */
    int *d = stringcat(stringdata((r[3])), stringdata((r[1])));
    r[4] = (hpushstr(4, d)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 81: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop lst k text */
  if (((stringlen((r[1]))) < (2))) {
    { /* string-append */
    int *d = stringcat(stringdata((cx__23683)), stringdata((r[1])));
    r[6] = (hpushstr(6, d)); }
  } else {
    r[6] = r[1];  
  }
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(82);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = r[6];  
    goto gs_string_2Dupcase;

case 82: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop lst k text */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__23690);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    r[3] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 83: /* c-argref-ctype k fmt n */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k fmt n */
    /* r[0] */    
    /* r[1] */    
    /* r[2] */    
    goto s_loop_v5485;

case 84: /* clo k s n */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
s_loop_v5485: /* k s n */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(84);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = obj_from_case(85);
    r[3] = (hendblk(4+1));
    r[0] = r[3];  
    /* r[1] */    
    r[2] = (cx__23401);
    goto gs_substring_2Dafter;

case 85: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r s n loop k */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(86);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    r[1] = r[2];  
    r[2] = (cx__23401);
    goto gs_substring_2Dbefore;

case 86: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r n loop k r */
  if (((!bool_from_obj(r[1])) || (!bool_from_obj(r[5])))) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_string_2Dstarts_2Dwith_3F((r[5]), (cx__23736)))) {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_string_2Dstarts_2Dwith_3F((r[5]), (cx__23735)))) {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
    r[3] = obj_from_fixnum(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if (bool_from_obj(cxs_string_2Dstarts_2Dwith_3F((r[5]), (cx__23734)))) {
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = r[5];  
    r[6+3] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if (bool_from_obj(cxs_string_2Dstarts_2Dwith_3F((r[5]), (cx__23733)))) {
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = r[5];  
    r[6+3] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[6] = (cxs_string_2Dstarts_2Dwith_3F((r[5]), (cx__23731)));
    r[6] = obj_from_bool(!bool_from_obj(r[6]));
  if (bool_from_obj(r[6])) {
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = r[5];  
    r[6+3] = r[2];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((fixnum_from_obj(r[2]) > (0))) {
    r[6+0] = r[3];  
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[4];  
    r[6+2] = r[5];  
    r[6+3] = obj_from_fixnum(fixnum_from_obj(r[2]) - (1));
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[6] = (cxs_string_2Dends_2Dwith_3F((r[1]), (cx__23420)));
    r[6] = obj_from_bool(!bool_from_obj(r[6]));
  if (bool_from_obj(r[6])) {
    r[6] = obj_from_bool(0);
  } else {
    { /* substring */
    int *d = substring(stringdata((r[1])), (0), ((stringlen((r[1]))) - (6)));
    r[6] = (hpushstr(6, d)); }
    { /* string->list */
    int c = (stringlen((r[6])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 7); /* 7 live regs */
    s = stringchars((r[6])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[7] = (l); }
    { fixnum_t v6109_tmp;
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6109_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6109_tmp);
    hreserve(hbsz(3)*c, 8); /* 8 live regs */
    l = r[7];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[7] = (o); } }
    { fixnum_t v6110_tmp;
    { /* length */
    int n; obj l = r[7];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6110_tmp = (n); }
    { /* list->string */
    int i, c = (v6110_tmp); 
    obj o = hpushstr(8, allocstring(c, ' ')); /* 8 live regs */
    obj l = r[7];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[6] = (o); } }
    r[7] = (cxs_string_2Dspan_2Dci((r[6]), (cx__23210)));
    { /* substring */
    int *d = substring(stringdata((r[6])), (0), fixnum_from_obj(r[7]));
    r[7] = (hpushstr(8, d)); }
    { /* string->list */
    int c = (stringlen((r[7])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 8); /* 8 live regs */
    s = stringchars((r[7])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[8] = (l); }
    { fixnum_t v6111_tmp;
    { /* length */
    int n; obj l = r[8];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6111_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6111_tmp);
    hreserve(hbsz(3)*c, 9); /* 9 live regs */
    l = r[8];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[8] = (o); } }
    { fixnum_t v6112_tmp;
    { /* length */
    int n; obj l = r[8];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6112_tmp = (n); }
    { /* list->string */
    int i, c = (v6112_tmp); 
    obj o = hpushstr(9, allocstring(c, ' ')); /* 9 live regs */
    obj l = r[8];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[6] = (o); } }
  }
    r[7+0] = r[4];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  }
  }
  }
  }
  }
  }

case 87: /* c-undecorate-alvar k sym */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_c_2Dundecorate_2Dalvar: /* k sym */
    r[2] = (hpushstr(2, newstring(symbolname(getsymbol((r[1]))))));
  if (bool_from_obj(cxs_string_2Dstarts_2Dwith_3F((r[2]), (cx__23690)))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(88);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = r[2];  
    r[2] = (cx__23690);
    goto gs_substring_2Dafter;
  } else {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 88: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    hreserve(hbsz(2+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(89);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    /* r[1] */    
    r[2] = (cx__23690);
    goto gs_substring_2Dbefore;

case 89: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = (bool_from_obj(r[1]) ? (r[1]) : (r[3]));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 90: /* display-text k o */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_display_2Dtext: /* k o */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cx__23761);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((ispair((r[1])))) {
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(91);
    r[2] = (hendblk(2+1));
    r[0] = r[2];  
    r[1] = (car((r[1])));
    goto gs_display_2Dtext;
  } else {
  if ((isvector((r[1])))) {
    { /* vector->list */
    obj v, l = mknull(); int c = (vectorlen((r[1])));
    hreserve(hbsz(3)*c, 2); /* 2 live regs */
    v = r[1];   /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = hblkref(v, 1+c);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[2] = (l); }
    /* r[0] */    
    r[1] = r[2];  
    goto gs_display_2Dtext;
  } else {
    r[2+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = r[0];  
    r[2+2] = r[1];  
    r[2+3] = obj_from_bool(1);
    r[2+4] = (cx__2Acurrent_2Doutput_2Dport_2A);
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }

case 91: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  o k */
    r[0] = r[3];  
    r[1] = (cdr((r[2])));
    goto gs_display_2Dtext;

case 92: /* path-strip-directory k filename */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k filename */
    { /* string->list */
    int c = (stringlen((r[1])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 2); /* 2 live regs */
    s = stringchars((r[1])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[2] = (l); }
    { fixnum_t v6108_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6108_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6108_tmp);
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[2] = (o); } }
    /* r[0] */    
    r[1] = r[2];  
    r[2] = (mknull());
    goto s_loop_v5442;

s_loop_v5442: /* k l r */
  if ((isnull((r[1])))) {
    { fixnum_t v6107_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6107_tmp = (n); }
    { /* list->string */
    int i, c = (v6107_tmp); 
    obj o = hpushstr(3, allocstring(c, ' ')); /* 3 live regs */
    obj l = r[2];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[3] = (o); } }
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (car((r[1])));
    r[3] = (ismemv((r[3]), (cx__23777)));
  if (bool_from_obj(r[3])) {
    { fixnum_t v6106_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6106_tmp = (n); }
    { /* list->string */
    int i, c = (v6106_tmp); 
    obj o = hpushstr(3, allocstring(c, ' ')); /* 3 live regs */
    obj l = r[2];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[3] = (o); } }
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[3] = (cdr((r[1])));
    r[4] = (car((r[1])));
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[3];  
    r[2] = r[4];  
    goto s_loop_v5442;
  }
  }

case 93: /* path-strip-extension k filename */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k filename */
    { /* string->list */
    int c = (stringlen((r[1])));
    char *s; obj l = mknull();
    hreserve(hbsz(3)*c, 2); /* 2 live regs */
    s = stringchars((r[1])); /* gc-safe */
    while (c-- > 0) { *--hp = l; *--hp = obj_from_char(s[c]);
    *--hp = obj_from_size(PAIR_BTAG); l = hendblk(3); }
    r[2] = (l); }
    { fixnum_t v6103_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6103_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6103_tmp);
    hreserve(hbsz(3)*c, 3); /* 3 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[2] = (o); } }
    r[3] = (ismemv(obj_from_char(46), (r[2])));
  if (bool_from_obj(r[3])) {
    r[4] = (cdr((r[3])));
    { fixnum_t v6104_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6104_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6104_tmp);
    hreserve(hbsz(3)*c, 5); /* 5 live regs */
    l = r[4];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[4] = (o); } }
    { fixnum_t v6105_tmp;
    { /* length */
    int n; obj l = r[4];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6105_tmp = (n); }
    { /* list->string */
    int i, c = (v6105_tmp); 
    obj o = hpushstr(5, allocstring(c, ' ')); /* 5 live regs */
    obj l = r[4];   /* gc-safe */
    char *s = stringchars(o);
    for (i = 0; i < c; ++i, l = cdr(l)) s[i] = char_from_obj(car(l));
    r[2] = (o); } }
  } else {
    r[2] = r[1];  
  }
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = r[2];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 94: /* cleanup-c-code! k str */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k str */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_cleanup_2Dc_2Dcode_21((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 95: /* stack-function-code-generate k self-id input-exp */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_stack_2Dfunction_2Dcode_2Dgenerate: /* k self-id input-exp */
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(96);
    r[3] = (hendblk(0+1));
    hreserve(hbsz(0+1), 4); /* 4 live regs */
    *--hp = obj_from_case(97);
    r[4] = (hendblk(0+1));
    hreserve(hbsz(0+1), 5); /* 5 live regs */
    *--hp = obj_from_case(100);
    r[5] = (hendblk(0+1));
    hreserve(hbsz(1), 6); /* 6 live regs */
    *--hp = obj_from_void(0);
    r[6] = (hendblk(1));
    hreserve(hbsz(1+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = obj_from_case(112);
    r[7] = (hendblk(1+1));
    hreserve(hbsz(1+1), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = obj_from_case(113);
    r[8] = (hendblk(1+1));
    (void)(objptr_from_obj(r[6])[0] = (mknull()));
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231758((r[2])))) {
    /* r[0] */    
    /* r[1] */    
    r[2] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231735((r[2])));
    goto gs_stack_2Dfunction_2Dcode_2Dgenerate;
  } else {
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (3))) {
    r[9] = (vectorref((r[2]), (0)));
    r[9] = obj_from_bool((r[9]) == (mksymbol(internsym("lambda-exp"))));
  } else {
    r[9] = obj_from_bool(0);
  }
  } else {
    r[9] = obj_from_bool(0);
  }
  if (bool_from_obj(r[9])) {
    r[9] = (vectorref((r[2]), (1)));
    r[10] = (vectorref((r[2]), (2)));
    hreserve(hbsz(7+1), 11); /* 11 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = obj_from_case(130);
    r[11] = (hendblk(7+1));
    hreserve(hbsz(9+1), 12); /* 12 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = (r[10]);
    *--hp = r[5];  
    *--hp = (r[11]);
    *--hp = obj_from_case(119);
    r[11] = (hendblk(9+1));
    r[0] = (r[11]);
    /* r[1] */    
    r[2] = r[9];  
    r[3] = r[6];  
    goto s_l_v5274;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[2];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[10+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[0];  
    r[10+2] = (cx__232500);
    r[10+3] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  }

case 96: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k id */
    r[2] = (cdr((r[1])));
    r[2] = (car((r[2])));
    r[2] = (hpushstr(3, newstring(symbolname(getsymbol((r[2]))))));
    r[3+0] = r[0];  
    r[3+1] = r[2];  
    r[3+2] = (cx__232493);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_c_2Dmangle;

case 97: /* clo k id */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k id */
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = obj_from_case(98);
    r[2] = (hendblk(2+1));
    r[3] = (cdr((r[1])));
    r[3] = (car((r[3])));
    r[0] = r[2];  
    r[1] = r[3];  
    goto gs_c_2Dundecorate_2Dalvar;

case 98: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_case(99);
    r[4] = (hendblk(2+1));
    r[5] = (cdr((r[2])));
    r[5] = (cdr((r[5])));
    r[5] = (car((r[5])));
    r[5] = obj_from_bool(is_fixnum_obj(r[5]));
  if (bool_from_obj(r[5])) {
    r[5] = (cdr((r[2])));
    r[5] = (cdr((r[5])));
    r[5] = (car((r[5])));
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[5] = (cdr((r[2])));
    r[5] = (cdr((r[5])));
    r[5] = (car((r[5])));
    r[0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 99: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__23690)));
    r[4] = (hpushstr(4, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__232480)), stringdata((r[4])));
    r[5] = (hpushstr(5, d)); }
    r[0] = r[3];  
    r[1] = r[2];  
    r[2] = r[5];  
    goto gs_c_2Dmangle;

case 100: /* clo k exp id */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k exp id */
    /* r[0] */    
    /* r[1] */    
    /* r[2] */    
    goto s_tc_3F;

case 101: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_tc_3F: /* k exp id */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(101);
    r[3] = (hendblk(1+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (2)));
    r[5] = (vectorref((r[1]), (3)));
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[6];  
    r[2] = r[3];  
    goto s_loop_v5377;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231758((r[1])))) {
    r[4] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231735((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_tc_3F;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_231575((r[1])))) {
    r[4] = (vectorref((r[1]), (1)));
    r[4] = (vectorref((r[4]), (2)));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_tc_3F;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (2)));
    r[5] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_case(104);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[5];  
    /* r[2] */    
    goto s_tc_3F;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (2))) {
    r[5] = (vectorref((r[4]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[4]), (1)));
    r[4] = obj_from_bool((r[5]) == (r[2]));
  } else {
    r[4] = obj_from_bool(0);
  }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[4]))));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_case(109);
    r[5] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    r[3] = (cx__231046);
    r[4] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cx__231042);
    r[3] = obj_from_bool(1);
    /* r[4] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }
  }
  }
  }
  }

case 102: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5377: /* k id tc? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(102);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(103);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
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

case 103: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 104: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r lams tc? k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(106);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v5362;
  }

s_loop_v5362: /* k id */
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
    *--hp = obj_from_case(105);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v5362;
  }

case 105: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (2)));
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

case 106: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r tc? k */
    r[0] = r[3];  
    /* r[1] */    
    /* r[2] */    
    goto s_loop_v5347;

case 107: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v5347: /* k id tc? */
  if ((!(isnull((r[1]))))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(107);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(108);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
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

case 108: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[5+0] = r[4];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 109: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(110);
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
    r[7+3] = (cx__231040);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 110: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(111);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__231037);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 111: /* clo ek  */
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

case 112: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k id label-alist */
    r[3] = (objptr_from_obj(r[2])[0]);
    { /* assq */
    obj x = (r[1]), l = (r[3]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[3] = (l == mknull() ? obj_from_bool(0) : p); }
  if (bool_from_obj(r[3])) {
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[0];  
    r[5+2] = (cx__232444);
    r[5+3] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 113: /* clo k id ids */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_l_v5274: /* k id ids label-alist */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(114);
    r[4] = (hendblk(4+1));
    r[5] = (cdr((r[1])));
    r[5] = (car((r[5])));
    r[0] = r[4];  
    r[1] = r[5];  
    goto gs_c_2Dundecorate_2Dalvar;

case 114: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id ids label-alist */
    hreserve(hbsz(4+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(115);
    r[6] = (hendblk(4+1));
    r[0] = r[6];  
    /* r[1] */    
    r[2] = (cx__232379);
    goto gs_c_2Dmangle;

case 115: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id ids label-alist */
    r[6] = (objptr_from_obj(r[5])[0]);
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(116);
    r[7] = (hendblk(4+1));
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_case(117);
    r[7] = (hendblk(3+1));
    r[8+0] = r[7];  
    r[8+1] = r[6];  
    r[8+2] = r[1];  
    r += 8; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5291;

case 116: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id ids label-alist */
    r[6] = (objptr_from_obj(r[5])[0]);
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[6];  
    *--hp = r[7];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[6] = (objptr_from_obj(r[5])[0] = (r[6]));
    r[7+0] = r[2];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = obj_from_ktrap();
    r[7+2] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v5291: /* k id r */
  if ((!(isnull((r[1]))))) {
    r[3] = (car((r[1])));
    r[4] = (cdr((r[3])));
    r[4] = (car((r[4])));
    r[3] = obj_from_bool(strcmp(stringchars((r[2])), stringchars((r[4]))) == 0);
  if (bool_from_obj(r[3])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_loop_v5291;
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

case 117: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id k r */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(118);
    r[5] = (hendblk(2+1));
    r[6] = (cdr((r[2])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[6] = obj_from_bool(is_fixnum_obj(r[6]));
  if (bool_from_obj(r[6])) {
    r[6] = (cdr((r[2])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    r[3] = obj_from_fixnum(10);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[6] = (cdr((r[2])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    r[0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 118: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* string-append */
    int *d = stringcat(stringdata((cx__232397)), stringdata((r[1])));
    r[4] = (hpushstr(4, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((r[3])), stringdata((r[4])));
    r[5] = (hpushstr(5, d)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 119: /* clo ek  */
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
    /* ek  code-gen-body cg-contains-tail-call? body id->label cvar-id->c-name ids gvar-id->c-name k self-id */
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(120);
    r[11] = (hendblk(8+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[4];  
    r[3] = (cx__231117);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 120: /* clo ek r */
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
    /* ek r cg-contains-tail-call? body id->label cvar-id->c-name ids gvar-id->c-name k self-id */
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__232509);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    hreserve(hbsz(6+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(121);
    r[11] = (hendblk(6+1));
    hreserve(hbsz(3+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = obj_from_case(128);
    r[11] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[3];  
    r[3] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 121: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name ids gvar-id->c-name k self-id r */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__232522);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (cx__232525);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    hreserve(hbsz(4+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(122);
    r[11] = (hendblk(4+1));
  if ((isnull((r[3])))) {
    r[0] = obj_from_ktrap();
    r[1] = (cx__232553);
    r[2] = r[4];  
    r[3] = r[5];  
    r[4] = r[6];  
    r[5] = (r[10]);
    goto s_l_v5243;
  } else {
    r[12] = (cdr((r[3])));
    hreserve(hbsz(3+1), 13); /* 13 live regs */
    *--hp = (r[11]);
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(126);
    r[13] = (hendblk(3+1));
    r[0] = (r[13]);
    r[1] = (r[12]);
    /* r[2] */    
    goto s_loop_v5225;
  }

case 122: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
s_l_v5243: /* ek r gvar-id->c-name k self-id r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__23208);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(123);
    r[8] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 123: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r k self-id r */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__232561);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__232564);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[8] = (cdr((r[3])));
    r[8] = (car((r[8])));
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[7];  
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__232572);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v5225: /* k id cvar-id->c-name */
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
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(124);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v5225;
  }

case 124: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(125);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 125: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (cx__232536);
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

case 126: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name ids k */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(127);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 127: /* clo ek r */
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
    *--hp = (cx__232550);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 128: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id->label self-id k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(129);
    r[5] = (hendblk(1+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 129: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__231308);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__231270);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 130: /* clo k exp tgt */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5];
    r[1+8] = p[6];
    r[1+9] = p[7]; }
    r += 1; /* shift reg. wnd */
    /* k exp tgt label-alist register-label! cg-contains-tail-call? id->label gvar-id->c-name cvar-id->c-name self-id */
    hreserve(hbsz(1+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = obj_from_case(231);
    r[10] = (hendblk(1+1));
    hreserve(hbsz(2+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(215);
    r[11] = (hendblk(2+1));
    r[12+0] = r[0];  
    r[12+1] = r[1];  
    r[12+2] = r[2];  
    r[12+3] = r[3];  
    r[12+4] = r[4];  
    r[12+5] = r[5];  
    r[12+6] = r[9];  
    r[12+7] = r[6];  
    r[12+8] = r[8];  
    r[12+9] = (r[11]);
    r[12+10] = (r[10]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_cg_2Dbody;

case 131: /* clo k exp tgt */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1];
    r[1+4] = p[2];
    r[1+5] = p[3];
    r[1+6] = p[4];
    r[1+7] = p[5];
    r[1+8] = p[6];
    r[1+9] = p[7];
    r[1+10] = p[8]; }
    r += 1; /* shift reg. wnd */
s_cg_2Dbody: /* k exp tgt label-alist register-label! cg-contains-tail-call? self-id id->label cvar-id->c-name cg-cexp cg-cexp? */
    hreserve(hbsz(8+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(131);
    r[11] = (hendblk(8+1));
    hreserve(hbsz(8+1), 12); /* 12 live regs */
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(210);
    r[12] = (hendblk(8+1));
    hreserve(hbsz(13+1), 13); /* 13 live regs */
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = (r[12]);
    *--hp = r[3];  
    *--hp = obj_from_case(132);
    r[11] = (hendblk(13+1));
    hreserve(hbsz(4+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(208);
    r[11] = (hendblk(4+1));
    r[12+0] = (r[10]);
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[11]);
    r[12+2] = r[1];  
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 132: /* clo ek r */
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
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13]; }
    r += 1; /* shift reg. wnd */
    /* ek r label-alist cg-let register-label! cg-contains-tail-call? self-id id->label cvar-id->c-name cg-cexp? cg-body cg-cexp exp k tgt */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 15); /* 15 live regs */
    *--hp = (r[14]);
    *--hp = (r[13]);
    *--hp = obj_from_case(133);
    r[15] = (hendblk(2+1));
    r[0] = (r[11]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[15]);
    r[2] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231758((r[12])))) {
    r[0] = (r[10]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231735((r[12])));
    r[3] = (r[14]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isvector((r[12])))) {
  if (((vectorlen((r[12]))) == (4))) {
    r[15] = (vectorref((r[12]), (0)));
    r[15] = obj_from_bool((r[15]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[15] = obj_from_bool(0);
  }
  } else {
    r[15] = obj_from_bool(0);
  }
  if (bool_from_obj(r[15])) {
    r[15] = (vectorref((r[12]), (1)));
    r[16] = (vectorref((r[12]), (2)));
    r[17] = (vectorref((r[12]), (3)));
    hreserve(hbsz(7+1), 18); /* 18 live regs */
    *--hp = (r[13]);
    *--hp = (r[15]);
    *--hp = (r[11]);
    *--hp = (r[16]);
    *--hp = (r[14]);
    *--hp = (r[10]);
    *--hp = (r[17]);
    *--hp = obj_from_case(135);
    r[18] = (hendblk(7+1));
    r[0] = r[9];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[18]);
    r[2] = (r[15]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(13+1), 15); /* 15 live regs */
    *--hp = (r[12]);
    *--hp = (r[13]);
    *--hp = (r[10]);
    *--hp = (r[14]);
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(141);
    r[15] = (hendblk(13+1));
    r[0] = (r[15]);
    r[1] = (r[12]);
    goto gs_begin_2Dexp_3F_231627;
  }
  }
  }

case 133: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k tgt */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(134);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = (cx__231861);
    r[2] = r[4];  
    goto gs_c_2Dformat_2A;

case 134: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k tgt */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__231078);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__231075);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 135: /* clo ek r */
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
    /* ek r else-exp cg-body tgt then-exp cg-cexp test-exp k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(136);
    r[9] = (hendblk(6+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    /* r[2] */    
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    hreserve(hbsz(6+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(140);
    r[9] = (hendblk(6+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  }

case 136: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-body tgt then-exp cg-cexp test-exp k */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = (cx__231262);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (cx__231718);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(137);
    r[10] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 137: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp test-exp k r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(138);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 138: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(139);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = (cx__231725);
    r[2] = r[4];  
    goto gs_c_2Dformat_2A;

case 139: /* clo ek r */
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

case 140: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-body tgt else-exp then-exp test-exp k */
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("tmp")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(2+1), 11); /* 11 live regs */
    *--hp = r[8];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[11] = (hendblk(2+1)); }
    { /* vector */
    hreserve(hbsz(4+1), 12); /* 12 live regs */
    *--hp = r[4];  
    *--hp = r[5];  
    *--hp = (r[11]);
    *--hp = (mksymbol(internsym("if-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[11] = (hendblk(4+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = r[9];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[12] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 13); /* 13 live regs */
    *--hp = (r[10]);
    *--hp = (r[12]);
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[9] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[9];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 141: /* clo ek r */
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
    r[1+12] = p[11];
    r[1+13] = p[12];
    r[1+14] = p[13]; }
    r += 1; /* shift reg. wnd */
    /* ek r label-alist cg-let register-label! cg-contains-tail-call? self-id id->label cvar-id->c-name cg-cexp? cg-cexp tgt cg-body k exp */
  if (bool_from_obj(r[1])) {
    r[15] = (vectorref((r[14]), (2)));
    r[15] = (car((r[15])));
    r[16] = (vectorref((r[14]), (1)));
    r[16] = (vectorref((r[16]), (2)));
    hreserve(hbsz(3+1), 17); /* 17 live regs */
    *--hp = (r[13]);
    *--hp = (r[15]);
    *--hp = (r[12]);
    *--hp = obj_from_case(142);
    r[17] = (hendblk(3+1));
    r[0] = (r[12]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[17]);
    r[2] = (r[16]);
    r[3] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_231575((r[14])))) {
    r[15] = (vectorref((r[14]), (1)));
    r[15] = (vectorref((r[15]), (1)));
    r[16] = (vectorref((r[14]), (2)));
    r[17] = (vectorref((r[14]), (1)));
    r[17] = (vectorref((r[17]), (2)));
    hreserve(hbsz(7+1), 18); /* 18 live regs */
    *--hp = (r[13]);
    *--hp = (r[15]);
    *--hp = (r[16]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = (r[12]);
    *--hp = obj_from_case(144);
    r[18] = (hendblk(7+1));
    r[0] = (r[12]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[18]);
    r[2] = (r[17]);
    r[3] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if (bool_from_obj(cxs_loop_2Dexp_3F_231437((r[14])))) {
    r[15] = (vectorref((r[14]), (1)));
    r[16] = (vectorref((r[15]), (1)));
    r[15] = (car((r[16])));
    r[16] = (vectorref((r[14]), (1)));
    r[17] = (vectorref((r[16]), (2)));
    r[16] = (car((r[17])));
    r[17] = (vectorref((r[14]), (2)));
    { /* vector */
    hreserve(hbsz(2+1), 18); /* 18 live regs */
    *--hp = (r[15]);
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[18] = (hendblk(2+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 19); /* 19 live regs */
    *--hp = (r[17]);
    *--hp = (r[18]);
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[18] = (hendblk(3+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 19); /* 19 live regs */
    *--hp = (mknull());
    *--hp = (r[16]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[19] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 20); /* 20 live regs */
    *--hp = (mknull());
    *--hp = (r[15]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[20] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(4+1), 21); /* 21 live regs */
    *--hp = (r[18]);
    *--hp = (r[19]);
    *--hp = (r[20]);
    *--hp = (mksymbol(internsym("fix-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[18] = (hendblk(4+1)); }
    r[0] = (r[12]);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = (r[18]);
    r[3] = (r[11]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
  if ((isvector((r[14])))) {
  if (((vectorlen((r[14]))) == (4))) {
    r[15] = (vectorref((r[14]), (0)));
    r[15] = obj_from_bool((r[15]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[15] = obj_from_bool(0);
  }
  } else {
    r[15] = obj_from_bool(0);
  }
  if (bool_from_obj(r[15])) {
    r[15] = (vectorref((r[14]), (1)));
    r[16] = (vectorref((r[14]), (2)));
    r[17] = (vectorref((r[14]), (3)));
    hreserve(hbsz(12+1), 18); /* 18 live regs */
    *--hp = (r[13]);
    *--hp = (r[16]);
    *--hp = r[8];  
    *--hp = (r[17]);
    *--hp = (r[11]);
    *--hp = (r[12]);
    *--hp = (r[15]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = (r[14]);
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(157);
    r[18] = (hendblk(12+1));
  if ((strcmp(stringchars((r[11])), stringchars((cx__231117))) == 0)) {
    r[19+0] = obj_from_ktrap();
    r[19+1] = obj_from_bool(0);
    r[19+2] = r[4];  
    r[19+3] = r[5];  
    r[19+4] = (r[14]);
    r[19+5] = r[6];  
    r[19+6] = r[7];  
    r[19+7] = (r[15]);
    r[19+8] = (r[12]);
    r[19+9] = (r[11]);
    r[19+10] = (r[17]);
    r[19+11] = r[8];  
    r[19+12] = (r[16]);
    r[19+13] = (r[13]);
    r += 19; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v4899;
  } else {
    hreserve(hbsz(1+1), 19); /* 19 live regs */
    *--hp = (r[18]);
    *--hp = obj_from_case(173);
    r[19] = (hendblk(1+1));
    hreserve(hbsz(1+1), 20); /* 20 live regs */
    *--hp = (r[19]);
    *--hp = obj_from_case(174);
    r[19] = (hendblk(1+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[19]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  }
  } else {
  if ((isvector((r[14])))) {
  if (((vectorlen((r[14]))) == (4))) {
    r[15] = (vectorref((r[14]), (0)));
    r[15] = obj_from_bool((r[15]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[15] = obj_from_bool(0);
  }
  } else {
    r[15] = obj_from_bool(0);
  }
  if (bool_from_obj(r[15])) {
    r[15] = (vectorref((r[14]), (1)));
    r[16] = (vectorref((r[14]), (2)));
    r[17] = (vectorref((r[14]), (3)));
    hreserve(hbsz(7+1), 18); /* 18 live regs */
    *--hp = (r[11]);
    *--hp = (r[13]);
    *--hp = (r[16]);
    *--hp = (r[17]);
    *--hp = (r[10]);
    *--hp = (r[15]);
    *--hp = r[3];  
    *--hp = obj_from_case(179);
    r[18] = (hendblk(7+1));
    r[0] = (r[18]);
    r[1] = (r[17]);
    r[2] = r[9];  
    goto s_loop_v4863;
  } else {
  if ((isvector((r[14])))) {
  if (((vectorlen((r[14]))) == (3))) {
    r[15] = (vectorref((r[14]), (0)));
    r[15] = obj_from_bool((r[15]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[15] = obj_from_bool(0);
  }
  } else {
    r[15] = obj_from_bool(0);
  }
  if (bool_from_obj(r[15])) {
    r[15] = (vectorref((r[14]), (1)));
    r[16] = (vectorref((r[14]), (2)));
    r[17] = (vectorref((r[15]), (1)));
    r[18] = (objptr_from_obj(r[2])[0]);
    { /* assq */
    obj x = (r[17]), l = (r[18]), p = mknull();
    for (; l != mknull(); l = cdr(l)) { p = car(l); if (car(p) == x) break; }
    r[18] = (l == mknull() ? obj_from_bool(0) : p); }
    hreserve(hbsz(10+1), 19); /* 19 live regs */
    *--hp = (r[11]);
    *--hp = (r[13]);
    *--hp = (r[16]);
    *--hp = r[8];  
    *--hp = (r[10]);
    *--hp = (r[17]);
    *--hp = r[7];  
    *--hp = (r[14]);
    *--hp = (r[15]);
    *--hp = r[3];  
    *--hp = obj_from_case(190);
    r[19] = (hendblk(10+1));
  if (bool_from_obj(r[18])) {
    r[20] = (cdr((r[18])));
    r[20] = (cdr((r[20])));
    r[0] = obj_from_ktrap();
    r[1] = (r[20]);
    r[2] = r[3];  
    r[3] = (r[15]);
    r[4] = (r[14]);
    r[5] = r[7];  
    r[6] = (r[17]);
    r[7] = (r[10]);
    /* r[8] */    
    r[9] = (r[16]);
    r[10] = (r[13]);
    r[11] = (r[11]);
    goto s_l_v4681;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 20); /* 20 live regs */
    *--hp = (mknull());
    *--hp = (r[17]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[20] = (hendblk(3)); }
    r[0] = (cx_c_2Derror_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[19]);
    r[2] = (cx__232434);
    r[3] = (r[20]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }
  } else {
    r[15] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[15]))));
    hreserve(hbsz(3+1), 16); /* 16 live regs */
    *--hp = (r[13]);
    *--hp = (r[15]);
    *--hp = (r[14]);
    *--hp = obj_from_case(205);
    r[16] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 17); /* 17 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[17] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[16]);
    r[2] = (r[15]);
    r[3] = (cx__231046);
    r[4] = (r[17]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[16]);
    r[2] = (cx__231042);
    r[3] = obj_from_bool(1);
    r[4] = (r[15]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }
  }
  }
  }

case 142: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-body exp1 k */
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(143);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    r[3] = (cx__231624);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 143: /* clo ek r */
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

case 144: /* clo ek r */
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
    /* ek r cg-body cvar-id->c-name cg-cexp? cg-cexp rands ids k */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__231262);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    hreserve(hbsz(7+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(149);
    r[10] = (hendblk(7+1));
    r[11+0] = (r[10]);
    r[11+1] = r[7];  
    r[11+2] = r[6];  
    r[11+3] = r[3];  
    r[11+4] = r[4];  
    r[11+5] = r[2];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5069;

s_loop_v5069: /* k id id cvar-id->c-name cg-cexp? cg-body */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    r[7] = (cdr((r[2])));
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(145);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = r[6];  
    r[2] = r[7];  
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v5069;
  }

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
    /* ek r cvar-id->c-name cg-cexp? cg-body k id id */
    r[8] = (car((r[7])));
    r[9] = (car((r[6])));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(146);
    r[10] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 146: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp? cg-body exp k r */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(147);
    r[7] = (hendblk(2+1));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(148);
    r[7] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 147: /* clo ek r */
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

case 148: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-body r exp k */
  if (bool_from_obj(r[1])) {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* string-append */
    int *d = stringcat(stringdata((r[3])), stringdata((cx__231525)));
    r[6] = (hpushstr(6, d)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    r[3] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 149: /* clo ek r */
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
    /* ek r cvar-id->c-name cg-cexp? cg-cexp rands ids k r */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[7];  
    *--hp = obj_from_case(156);
    r[10] = (hendblk(2+1));
    r[11+0] = (r[10]);
    r[11+1] = r[6];  
    r[11+2] = r[5];  
    r[11+3] = r[2];  
    r[11+4] = r[3];  
    r[11+5] = r[4];  
    r += 11; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v5022;

s_loop_v5022: /* k id id cvar-id->c-name cg-cexp? cg-cexp */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[6] = (cdr((r[1])));
    r[7] = (cdr((r[2])));
    hreserve(hbsz(6+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(150);
    r[8] = (hendblk(6+1));
    r[0] = r[8];  
    r[1] = r[6];  
    r[2] = r[7];  
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    goto s_loop_v5022;
  }

case 150: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5];
    r[1+7] = p[6]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name cg-cexp? cg-cexp k id id */
    r[8] = (car((r[7])));
    r[9] = (car((r[6])));
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[1];  
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(151);
    r[10] = (hendblk(5+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 151: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp? cg-cexp exp k r */
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(152);
    r[7] = (hendblk(2+1));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[1];  
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(153);
    r[7] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 152: /* clo ek r */
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

case 153: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp exp k r */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(154);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = (cx__231078);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__231354);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 154: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(155);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = (cx__231097);
    r[2] = r[4];  
    goto gs_c_2Dformat_2A;

case 155: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__231354);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 156: /* clo ek r */
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
    *--hp = (cx__231572);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 157: /* clo ek r */
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
    r[1+12] = p[11];
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
s_l_v4899: /* ek r register-label! cg-contains-tail-call? exp self-id id->label ids cg-body tgt body cvar-id->c-name lams k */
    hreserve(hbsz(12+1), 14); /* 14 live regs */
    *--hp = r[1];  
    *--hp = (r[13]);
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
    *--hp = obj_from_case(160);
    r[14] = (hendblk(12+1));
    r[15+0] = (r[14]);
    r[15+1] = r[7];  
    r[15+2] = (r[12]);
    r[15+3] = r[2];  
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v4995;

case 158: /* clo k id id */
    assert(rc == 4);
    { obj* p = objptr_from_obj(r[0]);
    r[1+3] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v4995: /* k id id register-label! */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = ((0) ? obj_from_bool(0) : obj_from_void(0));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[4] = (car((r[1])));
    r[5] = (car((r[2])));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(158);
    r[6] = (hendblk(1+1));
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[6];  
    *--hp = obj_from_case(159);
    r[6] = (hendblk(4+1));
    r[7+0] = r[3];  
    pc = objptr_from_obj(r[7+0])[0];
    r[7+1] = r[6];  
    r[7+2] = r[4];  
    r[7+3] = (vectorref((r[5]), (1)));
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  }

case 159: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek  loop id id k */
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cdr((r[4])));
    r[3] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 160: /* clo ek  */
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
    r[1+12] = p[11];
    r[1+13] = p[12]; }
    r += 1; /* shift reg. wnd */
    /* ek  cg-contains-tail-call? exp self-id id->label ids cg-body tgt body cvar-id->c-name lams k r */
    hreserve(hbsz(7+1), 14); /* 14 live regs */
    *--hp = (r[13]);
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(166);
    r[14] = (hendblk(7+1));
    r[15+0] = (r[14]);
    r[15+1] = r[6];  
    r[15+2] = (r[11]);
    r[15+3] = r[2];  
    r[15+4] = r[3];  
    r[15+5] = r[4];  
    r[15+6] = r[7];  
    r[15+7] = r[8];  
    r[15+8] = r[5];  
    r[15+9] = (r[13]);
    r += 15; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v4954;

s_loop_v4954: /* k id id cg-contains-tail-call? exp self-id cg-body tgt id->label r */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[10] = (cdr((r[1])));
    r[11] = (cdr((r[2])));
    hreserve(hbsz(10+1), 12); /* 12 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(161);
    r[12] = (hendblk(10+1));
    r[0] = (r[12]);
    r[1] = (r[10]);
    r[2] = (r[11]);
    /* r[3] */    
    /* r[4] */    
    /* r[5] */    
    /* r[6] */    
    /* r[7] */    
    /* r[8] */    
    /* r[9] */    
    goto s_loop_v4954;
  }

case 161: /* clo ek r */
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
    /* ek r cg-contains-tail-call? exp self-id cg-body tgt id->label r k id id */
    r[12] = (car((r[11])));
    r[13] = (car((r[10])));
    hreserve(hbsz(2+1), 14); /* 14 live regs */
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = obj_from_case(162);
    r[14] = (hendblk(2+1));
    hreserve(hbsz(8+1), 15); /* 15 live regs */
    *--hp = r[8];  
    *--hp = (r[14]);
    *--hp = (r[12]);
    *--hp = r[7];  
    *--hp = (r[13]);
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(163);
    r[14] = (hendblk(8+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[14]);
    r[2] = r[3];  
    r[3] = (r[12]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 162: /* clo ek r */
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

case 163: /* clo ek r */
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
    /* ek r self-id cg-body tgt lam id->label id k r */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(4+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = obj_from_case(164);
    r[10] = (hendblk(4+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[10]);
    r[2] = (vectorref((r[5]), (2)));
    r[3] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__231284);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[11] = (cdr((r[2])));
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
    *--hp = (cx__231292);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    r[0] = r[8];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 164: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r id->label id k r */
  if (bool_from_obj(r[5])) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = (cx__231078);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__231083);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
  } else {
    r[6] = (mknull());
  }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(165);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 165: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__231308);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (cx__231270);
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

case 166: /* clo ek r */
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
    /* ek r cg-body tgt body cvar-id->c-name lams k r */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = (cx__231262);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
  if (bool_from_obj(r[8])) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__231265);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__231270);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
  } else {
    r[10] = (mknull());
  }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
  if (bool_from_obj(r[8])) {
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = (mknull());
    *--hp = (cx__231078);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[8];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = (cx__231083);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
  } else {
    r[10] = (mknull());
  }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[9];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    hreserve(hbsz(4+1), 11); /* 11 live regs */
    *--hp = (r[10]);
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(167);
    r[11] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[11]);
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 167: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name lams k r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(172);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4912;

s_loop_v4912: /* k id cvar-id->c-name */
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
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(168);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4912;
  }

case 168: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name k id */
    r[5] = (car((r[4])));
    r[6] = (vectorref((r[5]), (1)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(171);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    /* r[2] */    
    goto s_loop_v4921;

s_loop_v4921: /* k id cvar-id->c-name */
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
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(169);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4921;
  }

case 169: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(170);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 170: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__231078);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (cx__231354);
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

case 171: /* clo ek r */
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

case 172: /* clo ek r */
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
    *--hp = (cx__231371);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 173: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* string-append */
    int *d = stringcat(stringdata((cx__232449)), stringdata((r[1])));
    r[3] = (hpushstr(3, d)); }
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 174: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
  if ((is_fixnum_obj(r[1]))) {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(175);
    r[3] = (hendblk(1+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  } else {
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(176);
    r[3] = (hendblk(1+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  }

case 175: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_fixnum_2D_3Estring);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r[3+3] = obj_from_fixnum(10);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 176: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = (cx_flonum_2D_3Estring);
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = r[1];  
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 177: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v4863: /* k id cg-cexp? */
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
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(177);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(178);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 178: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 179: /* clo ek r */
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
    /* ek r cg-let effect cg-cexp rands prim k tgt */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(5+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(180);
    r[9] = (hendblk(5+1));
    r[0] = r[9];  
    r[1] = r[6];  
    goto gs_prim_2Dcexp_3F;
  } else {
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = obj_from_case(189);
    r[9] = (hendblk(2+1));
    r[10+0] = r[2];  
    pc = objptr_from_obj(r[10+0])[0];
    r[10+1] = r[7];  
    r[10+2] = obj_from_bool(0);
    r[10+3] = r[5];  
    r[10+4] = r[8];  
    r[10+5] = r[9];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 180: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp rands prim k tgt */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(183);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4838;
  } else {
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(187);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4809;
  }

s_loop_v4838: /* k id cg-cexp */
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
    *--hp = obj_from_case(181);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4838;
  }

case 181: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(182);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 182: /* clo ek r */
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

case 183: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r prim k tgt */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(184);
    r[5] = (hendblk(2+1));
    r[6+0] = r[5];  
    r[6+1] = (cx__231206);
    r[6+2] = (cx__23662);
    r[6+3] = r[2];  
    r[6+4] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_c_2Dformat_2Dprim_2A;

case 184: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k tgt */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = (cx__231078);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (cx__23216);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__231075);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v4809: /* k id cg-cexp */
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
    *--hp = obj_from_case(185);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4809;
  }

case 185: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(186);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 186: /* clo ek r */
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

case 187: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r prim tgt k */
    hreserve(hbsz(1+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = obj_from_case(188);
    r[5] = (hendblk(1+1));
    { /* string-append */
    int *d = stringcat(stringdata((r[3])), stringdata((cx__231203)));
    r[6] = (hpushstr(6, d)); }
    r[7+0] = r[5];  
    r[7+1] = (cx__231206);
    r[7+2] = r[6];  
    r[7+3] = r[2];  
    r[7+4] = r[1];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_c_2Dformat_2Dprim_2A;

case 188: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* cons */ 
    hreserve(hbsz(3), 3); /* 3 live regs */
    *--hp = (mknull());
    *--hp = (cx__231078);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[3] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = (cx__231075);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 189: /* clo k rands */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* k rands prim effect */
    { /* vector */
    hreserve(hbsz(4+1), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = (mksymbol(internsym("primapp-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[4] = (hendblk(4+1)); }
    r[5+0] = r[0];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 190: /* clo ek r */
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
s_l_v4681: /* ek r cg-let rator exp id->label rator-id cg-cexp cvar-id->c-name rands k tgt */
    hreserve(hbsz(11+1), 12); /* 12 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(191);
    r[12] = (hendblk(11+1));
    hreserve(hbsz(3+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = r[1];  
    *--hp = r[9];  
    *--hp = obj_from_case(201);
    r[12] = (hendblk(3+1));
    r[0] = (r[12]);
    r[1] = r[9];  
    goto s_loop_v4711;

case 191: /* clo ek r */
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
    /* ek r cg-let rator exp id->label rator-id cg-cexp cvar-id->c-name rands r k tgt */
  if (bool_from_obj(r[1])) {
  if ((strcmp(stringchars((r[12])), stringchars((cx__231117))) == 0)) {
    hreserve(hbsz(5+1), 13); /* 13 live regs */
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = obj_from_case(192);
    r[13] = (hendblk(5+1));
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(2+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = obj_from_case(198);
    r[13] = (hendblk(2+1));
    r[0] = r[7];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[13]);
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
    hreserve(hbsz(1+1), 13); /* 13 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(200);
    r[13] = (hendblk(1+1));
    r[14+0] = r[2];  
    pc = objptr_from_obj(r[14+0])[0];
    r[14+1] = (r[11]);
    r[14+2] = obj_from_bool(1);
    r[14+3] = r[9];  
    r[14+4] = (r[12]);
    r[14+5] = (r[13]);
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;
  }

case 192: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp cvar-id->c-name rands r k */
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = (cx__231078);
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
    *--hp = (cx__231083);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[8] = (hendblk(3)); }
    hreserve(hbsz(2+1), 9); /* 9 live regs */
    *--hp = r[8];  
    *--hp = r[6];  
    *--hp = obj_from_case(197);
    r[9] = (hendblk(2+1));
    r[10+0] = r[9];  
    r[10+1] = r[5];  
    r[10+2] = r[4];  
    r[10+3] = r[2];  
    r[10+4] = r[3];  
    r += 10; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v4744;

s_loop_v4744: /* k id id cg-cexp cvar-id->c-name */
  if (((isnull((r[1]))) || (isnull((r[2]))))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = (mknull());
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[5] = (cdr((r[1])));
    r[6] = (cdr((r[2])));
    hreserve(hbsz(5+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(193);
    r[7] = (hendblk(5+1));
    r[0] = r[7];  
    r[1] = r[5];  
    r[2] = r[6];  
    /* r[3] */    
    /* r[4] */    
    goto s_loop_v4744;
  }

case 193: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp cvar-id->c-name k id id */
    r[7] = (car((r[6])));
    r[8] = (car((r[5])));
    hreserve(hbsz(4+1), 9); /* 9 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = r[7];  
    *--hp = r[3];  
    *--hp = obj_from_case(194);
    r[9] = (hendblk(4+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    r[2] = r[8];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 194: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name rand-id k r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(195);
    r[7] = (hendblk(4+1));
    r[0] = r[7];  
    r[1] = (cx__231097);
    r[2] = r[6];  
    goto gs_c_2Dformat_2A;

case 195: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cvar-id->c-name rand-id k r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = obj_from_case(196);
    r[7] = (hendblk(3+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 196: /* clo ek r */
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
    *--hp = r[5];  
    *--hp = (cx__231075);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[3];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 197: /* clo ek r */
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
    *--hp = (cx__231114);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 198: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k tgt */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(199);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = (cx__231070);
    r[2] = r[4];  
    goto gs_c_2Dformat_2A;

case 199: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k tgt */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = (cx__231075);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 200: /* clo k rands */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* k rands rator */
    { /* vector */
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[3] = (hendblk(3+1)); }
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = r[3];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v4711: /* k id */
  if ((isnull((r[1])))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = obj_from_bool(isnull((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[2] = (car((r[1])));
  if ((isvector((r[2])))) {
  if (((vectorlen((r[2]))) == (2))) {
    r[3] = (vectorref((r[2]), (0)));
    r[2] = obj_from_bool((r[3]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[2] = obj_from_bool(0);
  }
  } else {
    r[2] = obj_from_bool(0);
  }
  if (bool_from_obj(r[2])) {
    r[2] = (cdr((r[1])));
    /* r[0] */    
    r[1] = r[2];  
    goto s_loop_v4711;
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
  }

case 201: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r rands r k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(203);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = r[2];  
    goto s_loop_v4696;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

s_loop_v4696: /* k id */
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
    *--hp = obj_from_case(202);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v4696;
  }

case 202: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    r[4] = (vectorref((r[4]), (1)));
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

case 203: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r r k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(204);
    r[4] = (hendblk(1+1));
    r[0] = r[4];  
    /* r[1] */    
    /* r[2] */    
    goto gs_intersectionq_231133;

case 204: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    r[3+0] = r[2];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = obj_from_bool(isnull((r[1])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 205: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(206);
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
    r[7+3] = (cx__231040);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 206: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(207);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__231037);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 207: /* clo ek  */
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

case 208: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-contains-tail-call? self-id exp k */
  if (bool_from_obj(r[1])) {
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = obj_from_case(209);
    r[6] = (hendblk(1+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[4];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;
  } else {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 209: /* clo ek r */
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

case 210: /* clo k wrap-all? rands tgt make-body */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4];
    r[1+9] = p[5];
    r[1+10] = p[6];
    r[1+11] = p[7];
    r[1+12] = p[8]; }
    r += 1; /* shift reg. wnd */
    /* k wrap-all? rands tgt make-body label-alist register-label! cg-contains-tail-call? self-id id->label cvar-id->c-name cg-cexp cg-cexp? */
    hreserve(hbsz(8+1), 13); /* 13 live regs */
    *--hp = (r[12]);
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(131);
    r[13] = (hendblk(8+1));
    r[14+0] = r[0];  
    r[14+1] = r[2];  
    r[14+2] = (mknull());
    r[14+3] = (mknull());
    r[14+4] = (mknull());
    r[14+5] = (r[12]);
    r[14+6] = r[1];  
    r[14+7] = r[4];  
    r[14+8] = (r[13]);
    r[14+9] = r[3];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_loop_v4609;

case 211: /* clo k rands out-rands ids exps */
    assert(rc == 6);
    { obj* p = objptr_from_obj(r[0]);
    r[1+5] = p[1];
    r[1+6] = p[2];
    r[1+7] = p[3];
    r[1+8] = p[4];
    r[1+9] = p[5]; }
    r += 1; /* shift reg. wnd */
s_loop_v4609: /* k rands out-rands ids exps cg-cexp? wrap-all? make-body cg-body tgt */
  if ((isnull((r[1])))) {
    hreserve(hbsz(5+1), 10); /* 10 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = r[0];  
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = obj_from_case(212);
    r[10] = (hendblk(5+1));
    { fixnum_t v6102_tmp;
    { /* length */
    int n; obj l = r[2];  
    for (n = 0; l != mknull(); ++n, l = cdr(l)) ;
    v6102_tmp = (n); }
    { /* reverse */
    obj l, o = mknull(); int c = (v6102_tmp);
    hreserve(hbsz(3)*c, 11); /* 11 live regs */
    l = r[2];   /* gc-safe */
    for (; l != mknull(); l = cdr(l)) { *--hp = o; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(3); }  
    r[11] = (o); } }
    r[12+0] = r[7];  
    pc = objptr_from_obj(r[12+0])[0];
    r[12+1] = (r[10]);
    r[12+2] = (r[11]);
    r += 12; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[10] = (car((r[1])));
    r[11] = (cdr((r[1])));
    hreserve(hbsz(5+1), 12); /* 12 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(211);
    r[12] = (hendblk(5+1));
    hreserve(hbsz(7+1), 13); /* 13 live regs */
    *--hp = r[0];  
    *--hp = (r[11]);
    *--hp = (r[10]);
    *--hp = r[2];  
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = (r[12]);
    *--hp = obj_from_case(213);
    r[12] = (hendblk(7+1));
  if ((!bool_from_obj(r[6]))) {
    r[0] = r[5];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = (r[12]);
    r[2] = (r[10]);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    hreserve(hbsz(5+1), 13); /* 13 live regs */
    *--hp = r[9];  
    *--hp = r[8];  
    *--hp = r[7];  
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_case(211);
    r[13] = (hendblk(5+1));
    r[14+0] = obj_from_ktrap();
    r[14+1] = obj_from_bool(0);
    r[14+2] = (r[13]);
    r[14+3] = r[4];  
    r[14+4] = r[3];  
    r[14+5] = r[2];  
    r[14+6] = (r[10]);
    r[14+7] = (r[11]);
    r[14+8] = r[0];  
    r += 14; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto s_l_v4611;
  }
  }

case 212: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4];
    r[1+6] = p[5]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-body tgt k exps ids */
    { /* vector */
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[1];  
    *--hp = r[6];  
    *--hp = (mksymbol(internsym("lambda-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    { /* vector */
    hreserve(hbsz(3+1), 8); /* 8 live regs */
    *--hp = r[5];  
    *--hp = r[7];  
    *--hp = (mksymbol(internsym("app-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[7] = (hendblk(3+1)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = r[7];  
    /* r[3] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 4);
    goto jump;

case 213: /* clo ek r */
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
s_l_v4611: /* ek r loop exps ids out-rands rand rands k */
  if (bool_from_obj(r[1])) {
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
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
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(214);
    r[9] = (hendblk(7+1));
    r[0] = (cx_timestamp);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[9];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 2);
    goto jump;
  }

case 214: /* clo ek r */
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
    /* ek r loop exps rand ids out-rands rands k */
    { /* cons */ 
    hreserve(hbsz(3), 9); /* 9 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (mksymbol(internsym("tmp")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (mksymbol(internsym("var")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[9] = (hendblk(3)); }
    { /* vector */
    hreserve(hbsz(2+1), 10); /* 10 live regs */
    *--hp = r[9];  
    *--hp = (mksymbol(internsym("var-exp")));
    *--hp = obj_from_size(VECTOR_BTAG);
    r[10] = (hendblk(2+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[6];  
    *--hp = (r[10]);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[10] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 11); /* 11 live regs */
    *--hp = r[5];  
    *--hp = r[9];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[11] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 12); /* 12 live regs */
    *--hp = r[3];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[12] = (hendblk(3)); }
    r[13+0] = r[2];  
    pc = objptr_from_obj(r[13+0])[0];
    r[13+1] = r[8];  
    r[13+2] = r[7];  
    r[13+3] = (r[10]);
    r[13+4] = (r[11]);
    r[13+5] = (r[12]);
    r += 13; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 6);
    goto jump;

case 215: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
s_cg_2Dcexp: /* k exp gvar-id->c-name cvar-id->c-name */
    hreserve(hbsz(2+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(215);
    r[4] = (hendblk(2+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    hreserve(hbsz(1+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(216);
    r[6] = (hendblk(1+1));
    r[7] = (cdr((r[5])));
    r[7] = (cdr((r[7])));
    r[7] = (car((r[7])));
    r[7] = obj_from_bool(fixnum_from_obj(r[7]) < (0));
  if (bool_from_obj(r[7])) {
    r[7] = (cdr((r[5])));
    r[7] = (car((r[7])));
    r[7] = (hpushstr(8, newstring(symbolname(getsymbol((r[7]))))));
    r[0] = r[6];  
    r[1] = r[7];  
    r[2] = (cx__232487);
    goto gs_c_2Dmangle;
  } else {
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    r[7] = (vectorref((r[1]), (3)));
    hreserve(hbsz(4+1), 8); /* 8 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[6];  
    *--hp = obj_from_case(217);
    r[8] = (hendblk(4+1));
    r[0] = r[8];  
    r[1] = r[7];  
    /* r[2] */    
    /* r[3] */    
    goto s_cg_2Dcexp;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231758((r[1])))) {
    r[5] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231735((r[1])));
    /* r[0] */    
    r[1] = r[5];  
    /* r[2] */    
    /* r[3] */    
    goto s_cg_2Dcexp;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (1)));
    r[6] = (vectorref((r[1]), (2)));
    hreserve(hbsz(3+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[2];  
    *--hp = obj_from_case(223);
    r[7] = (hendblk(3+1));
    r[0] = r[7];  
    r[1] = r[6];  
    r[2] = r[4];  
    goto s_loop_v4553;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[5] = (vectorref((r[1]), (0)));
    r[5] = obj_from_bool((r[5]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[5] = obj_from_bool(0);
  }
  } else {
    r[5] = obj_from_bool(0);
  }
  if (bool_from_obj(r[5])) {
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (3)));
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = obj_from_case(227);
    r[7] = (hendblk(2+1));
    r[0] = r[7];  
    r[1] = r[6];  
    r[2] = r[4];  
    goto s_loop_v4524;
  } else {
    r[5] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[5]))));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_case(228);
    r[6] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    r[3] = (cx__231046);
    r[4] = r[7];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = (cx__231042);
    r[3] = obj_from_bool(1);
    r[4] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }
  }
  }

case 216: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* string-append */
    int *d = stringcat(stringdata((r[1])), stringdata((cx__232032)));
    r[3] = (hpushstr(3, d)); }
    { /* string-append */
    int *d = stringcat(stringdata((cx__231991)), stringdata((r[3])));
    r[4] = (hpushstr(4, d)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[4];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 217: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r then-exp cg-cexp test-exp k */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(4+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(218);
    r[7] = (hendblk(4+1));
    r[0] = r[3];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 218: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3];
    r[1+5] = p[4]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp test-exp k r */
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = r[5];  
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    hreserve(hbsz(2+1), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[4];  
    *--hp = obj_from_case(219);
    r[7] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[7];  
    r[2] = r[3];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 219: /* clo ek r */
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
    r[0] = r[2];  
    r[1] = (cx__232020);
    r[2] = r[4];  
    goto gs_c_2Dformat_2A;

s_loop_v4553: /* k id cg-cexp */
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
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = r[2];  
    *--hp = obj_from_case(220);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4553;
  }

case 220: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp k id */
    r[5] = (car((r[4])));
    hreserve(hbsz(2+1), 6); /* 6 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(221);
    r[6] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[6];  
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 221: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(222);
    r[5] = (hendblk(2+1));
    r[0] = r[5];  
    r[1] = (cx__231962);
    r[2] = r[4];  
    goto gs_c_2Dformat_2A;

case 222: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k r */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = (mknull());
    *--hp = r[1];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = (cx__23447);
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

case 223: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r gvar-id->c-name rator k */
  if ((isnull((r[1])))) {
    r[5] = r[1];  
  } else {
    r[5] = (cdr((r[1])));
    r[6] = (car((r[1])));
    r[6] = (cdr((r[6])));
    r[6] = (car((r[6])));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[5];  
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
  }
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = (cx__231952);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = r[6];  
    *--hp = (cx__23208);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    hreserve(hbsz(2+1), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = obj_from_case(224);
    r[8] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[8];  
    r[2] = (vectorref((r[3]), (1)));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 224: /* clo ek r */
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
    *--hp = (cx__231991);
    *--hp = obj_from_size(PAIR_BTAG); 
    r[5] = (hendblk(3)); }
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = r[5];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

s_loop_v4524: /* k id cg-cexp */
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
    *--hp = obj_from_case(225);
    r[4] = (hendblk(3+1));
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4524;
  }

case 225: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp id k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[4];  
    *--hp = obj_from_case(226);
    r[5] = (hendblk(2+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (car((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;

case 226: /* clo ek r */
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

case 227: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r prim k */
    r[4+0] = r[3];  
    r[4+1] = (cx__231936);
    r[4+2] = r[2];  
    r[4+3] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_c_2Dformat_2Dprimexp_2A;

case 228: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(229);
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
    r[7+3] = (cx__231040);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 229: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(230);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__231037);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 230: /* clo ek  */
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

case 231: /* clo k exp */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_cg_2Dcexp_3F: /* k exp self-id */
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(231);
    r[3] = (hendblk(1+1));
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (2))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(1);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("if-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
    r[6] = (vectorref((r[1]), (3)));
    { /* cons */ 
    hreserve(hbsz(3), 7); /* 7 live regs */
    *--hp = (mknull());
    *--hp = r[6];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[5];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    { /* cons */ 
    hreserve(hbsz(3), 8); /* 8 live regs */
    *--hp = r[7];  
    *--hp = r[4];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[7] = (hendblk(3)); }
    /* r[0] */    
    r[1] = r[7];  
    r[2] = r[3];  
    goto s_loop_v4482;
  } else {
  if (bool_from_obj(cxs_degenerate_2Dlet_2Dexp_3F_231758((r[1])))) {
    r[4] = (cxs_degenerate_2Dlet_2Dexp_2D_3Ebody_231735((r[1])));
    /* r[0] */    
    r[1] = r[4];  
    /* r[2] */    
    goto s_cg_2Dcexp_3F;
  } else {
  if (bool_from_obj(cxs_let_2Dexp_3F_231575((r[1])))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("fix-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (3))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("app-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (1)));
    r[5] = (vectorref((r[1]), (2)));
  if ((isvector((r[4])))) {
  if (((vectorlen((r[4]))) == (2))) {
    r[6] = (vectorref((r[4]), (0)));
    r[6] = obj_from_bool((r[6]) == (mksymbol(internsym("var-exp"))));
  } else {
    r[6] = obj_from_bool(0);
  }
  } else {
    r[6] = obj_from_bool(0);
  }
  if (bool_from_obj(r[6])) {
    r[6] = (vectorref((r[4]), (1)));
    r[6] = obj_from_bool((r[6]) == (r[2]));
  if (bool_from_obj(r[6])) {
    /* r[0] */    
    r[1] = r[5];  
    r[2] = r[3];  
    goto s_loop_v4471;
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
  if ((isvector((r[1])))) {
  if (((vectorlen((r[1]))) == (4))) {
    r[4] = (vectorref((r[1]), (0)));
    r[4] = obj_from_bool((r[4]) == (mksymbol(internsym("primapp-exp"))));
  } else {
    r[4] = obj_from_bool(0);
  }
  } else {
    r[4] = obj_from_bool(0);
  }
  if (bool_from_obj(r[4])) {
    r[4] = (vectorref((r[1]), (2)));
    r[5] = (vectorref((r[1]), (3)));
    hreserve(hbsz(3+1), 6); /* 6 live regs */
    *--hp = r[0];  
    *--hp = r[5];  
    *--hp = r[3];  
    *--hp = obj_from_case(236);
    r[6] = (hendblk(3+1));
    r[0] = r[6];  
    r[1] = r[4];  
    goto gs_prim_2Dcexp_3F;
  } else {
    r[4] = (cx__2Acurrent_2Derror_2Dport_2A);
    (void)(fputc('\n', oportdata((r[4]))));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[0];  
    *--hp = r[4];  
    *--hp = r[1];  
    *--hp = obj_from_case(239);
    r[5] = (hendblk(3+1));
  if (bool_from_obj(mksymbol(internsym("variant-case")))) {
    { /* cons */ 
    hreserve(hbsz(3), 6); /* 6 live regs */
    *--hp = (mknull());
    *--hp = (mksymbol(internsym("variant-case")));
    *--hp = obj_from_size(PAIR_BTAG); 
    r[6] = (hendblk(3)); }
    r[0] = (cx_fprintf_2A);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = r[4];  
    r[3] = (cx__231046);
    r[4] = r[6];  
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  } else {
    r[0] = (cx_write_2F3);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cx__231042);
    r[3] = obj_from_bool(1);
    /* r[4] */    
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;
  }
  }
  }
  }
  }
  }
  }
  }

case 232: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v4482: /* k id cg-cexp? */
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
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(232);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(233);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 233: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 234: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v4471: /* k id cg-cexp? */
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
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(234);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(235);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 235: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 236: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r cg-cexp? rands k */
  if (bool_from_obj(r[1])) {
    r[0] = r[4];  
    r[1] = r[3];  
    /* r[2] */    
    goto s_loop_v4456;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 237: /* clo k id */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
s_loop_v4456: /* k id cg-cexp? */
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
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(237);
    r[3] = (hendblk(1+1));
    hreserve(hbsz(3+1), 4); /* 4 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_case(238);
    r[3] = (hendblk(3+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (car((r[1])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 238: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r loop id k */
  if (bool_from_obj(r[1])) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[4];  
    r[2] = (cdr((r[3])));
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  } else {
    r[0] = r[4];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 3);
    goto jump;
  }

case 239: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek  exp ep k */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(240);
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
    r[7+3] = (cx__231040);
    r[7+4] = r[6];  
    r += 7; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 240: /* clo ek  */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek  ep k */
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[3];  
    *--hp = obj_from_case(241);
    r[4] = (hendblk(1+1));
    r[5+0] = (cx_write_2F3);
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = (cx__231037);
    r[5+3] = obj_from_bool(1);
    r[5+4] = r[2];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 241: /* clo ek  */
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

case 242: /* stack-functions-code-generate k id&exp-list */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k id&exp-list */
    /* r[0] */    
    /* r[1] */    
    goto s_loop_v4404;

s_loop_v4404: /* k id */
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
    *--hp = obj_from_case(243);
    r[3] = (hendblk(2+1));
    r[0] = r[3];  
    r[1] = r[2];  
    goto s_loop_v4404;
  }

case 243: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k id */
    r[4] = (car((r[3])));
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = r[4];  
    *--hp = obj_from_case(244);
    r[5] = (hendblk(3+1));
    r[0] = (cx_beta_2Dsubstitute);
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[5];  
    r[2] = (cdr((r[4])));
    r[3] = obj_from_bool(0);
    r[4] = obj_from_bool(0);
    rreserve(MAX_LIVEREGS);
    assert(rc = 5);
    goto jump;

case 244: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r id&exp k r */
    hreserve(hbsz(2+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = obj_from_case(245);
    r[5] = (hendblk(2+1));
    r[6+0] = r[5];  
    r[6+1] = (car((r[2])));
    r[6+2] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_stack_2Dfunction_2Dcode_2Dgenerate;

case 245: /* clo ek r */
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

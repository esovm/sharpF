/* tfun.sf */
/* Generated by #F $Id: c.sf 22 2008-04-26 19:28:22Z esl@acm.org $ */
#ifdef PROFILE
#define host host_module_tfun
#endif
#define MODULE module_tfun
#define LOAD() 

/* standard includes */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

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
extern void cxm_check(int x, char *msg);
extern void *cxm_cknull(void *p, char *msg);
extern int cxg_rc;

/* extra definitions */
/* immediate object representation */
#define isimm(o, t) (((o) & 0xff) == (((t) << 1) | 1))
#define getimmu(o, t) (int)(((o) >> 8) & 0xffffff)
#define getimms(o, t) (int)(((((o) >> 8) & 0xffffff) ^ 0x800000) - 0x800000)
#define mkimm(o, t) ((((o) & 0xffffff) << 8) | ((t) << 1) | 1)
extern int istagged(obj o, int t);
#define cktagged(o, t) (o)
#define taggedlen(o, t) (hblklen(o)-1) 
#define taggedref(o, t, i) (&hblkref(o, (i)+1))
/* booleans */
#define TRUE_ITAG 0
typedef int bool_t;
#define is_bool_obj(o) (!((o) & ~(obj)1))
#define is_bool_bool(b) ((b), 1)
#define void_from_bool(b) (void)(b)
#define obj_from_bool(b) ((b) ? mkimm(0, TRUE_ITAG) : 0)
/* fixnums */
#define FIXNUM_ITAG 1
typedef int fixnum_t;
#define is_fixnum_obj(o) (isimm(o, FIXNUM_ITAG))
#define is_fixnum_fixnum(i) ((i), 1)
#define fixnum_from_obj(o) (getimms(o, FIXNUM_ITAG))
#define fixnum_from_fixnum(i) (i)
#define void_from_fixnum(i) (void)(i)
#define obj_from_fixnum(i) mkimm(i, FIXNUM_ITAG)
#define FIXNUM_MIN -8388608
#define FIXNUM_MAX 8388607
/* null */
#define NULL_ITAG 2
#define mknull() mkimm(0, NULL_ITAG)
#define isnull(o) ((o) == mkimm(0, NULL_ITAG))
/* pairs and lists */
#define PAIR_BTAG 1
#define ispair(o) istagged(o, PAIR_BTAG)
#define car(o) *taggedref(o, PAIR_BTAG, 0)
#define cdr(o) *taggedref(o, PAIR_BTAG, 1)

/* cx globals */
obj cx_l12; /* l12 */
obj cx_l18; /* l18 */
obj cx_l6; /* l6 */
obj cx_length; /* length */
obj cx_listn; /* listn */
obj cx_ltak; /* ltak */
obj cx_shorterp; /* shorterp */
obj cx_tak; /* tak */

/* helper functions */
/* tak */
static obj cxs_tak(obj v3_x, obj v2_y, obj v1_z)
{ 
  s_tak:
  if ((fixnum_from_obj(v2_y) < fixnum_from_obj(v3_x))) {
  { /* let */
    obj v79_tmp = obj_from_obj(cxs_tak(obj_from_fixnum(fixnum_from_obj(v1_z) - (1)), (v3_x), (v2_y)));
    obj v78_tmp = obj_from_obj(cxs_tak(obj_from_fixnum(fixnum_from_obj(v2_y) - (1)), (v1_z), (v3_x)));
    obj v77_tmp = obj_from_obj(cxs_tak(obj_from_fixnum(fixnum_from_obj(v3_x) - (1)), (v2_y), (v1_z)));
    /* tail call */
    v3_x = (v77_tmp);
    v2_y = (v78_tmp);
    v1_z = (v79_tmp);
    goto s_tak;
  }
  } else {
    return (v1_z);
  }
}

/* length */
static obj cxs_length(obj v21_l)
{ 
  { /* letrec */
    obj v24_l;
    obj v23_n;
  { /* let */
    obj v76_tmp = obj_from_fixnum(0);
    obj v75_tmp = (v21_l);
    /* tail call */
    v24_l = (v75_tmp);
    v23_n = (v76_tmp);
    goto s_length_2Daux;
  }
  s_length_2Daux:
  if ((ispair((v24_l)))) {
  { /* let */
    obj v74_tmp = obj_from_fixnum(fixnum_from_obj(v23_n) + (1));
    obj v73_tmp = (cdr((v24_l)));
    /* tail call */
    v24_l = (v73_tmp);
    v23_n = (v74_tmp);
    goto s_length_2Daux;
  }
  } else {
    return (v23_n);
  }
  }
}

/* shorterp */
static obj cxs_shorterp(obj v34_x, obj v33_y)
{ 
  s_shorterp:
  if ((ispair((v33_y)))) {
  if ((isnull((v34_x)))) {
    return obj_from_bool(isnull((v34_x)));
  } else {
  { /* let */
    obj v72_tmp = (cdr((v33_y)));
    obj v71_tmp = (cdr((v34_x)));
    /* tail call */
    v34_x = (v71_tmp);
    v33_y = (v72_tmp);
    goto s_shorterp;
  }
  }
  } else {
    return obj_from_bool(0);
  }
}

/* ltak */
static obj cxs_ltak(obj v46_x, obj v45_y, obj v44_z)
{ 
  s_ltak:
  if ((!bool_from_obj(cxs_shorterp((v45_y), (v46_x))))) {
    return (v44_z);
  } else {
  { /* let */
    obj v70_tmp = obj_from_obj(cxs_ltak((cdr((v44_z))), (v46_x), (v45_y)));
    obj v69_tmp = obj_from_obj(cxs_ltak((cdr((v45_y))), (v44_z), (v46_x)));
    obj v68_tmp = obj_from_obj(cxs_ltak((cdr((v46_x))), (v45_y), (v44_z)));
    /* tail call */
    v46_x = (v68_tmp);
    v45_y = (v69_tmp);
    v44_z = (v70_tmp);
    goto s_ltak;
  }
  }
}

/* gc roots */
static obj *globv[] = {
  &cx_l12,
  &cx_l18,
  &cx_l6,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[10] = {
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 9
static obj host(obj pc)
{
  register obj *r = cxg_regs;
  register obj *hp = cxg_hp;
  register int rc = cxg_rc;
jump: 
  switch (case_from_obj(pc)) {

case 0: /* load module */
    { static obj c[] = { obj_from_case(1) }; cx_tak = (obj)c; }
    { static obj c[] = { obj_from_case(2) }; cx_listn = (obj)c; }
    { static obj c[] = { obj_from_case(4) }; cx_length = (obj)c; }
    { static obj c[] = { obj_from_case(5) }; cx_shorterp = (obj)c; }
    hreserve(hbsz(0+1), 0); /* 0 live regs */
    *--hp = obj_from_case(6);
    r[0] = (hendblk(0+1));
    r[1+0] = r[0];  
    r[1+1] = obj_from_fixnum(18);
    r += 1; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_listn;

case 1: /* tak k x y z */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k x y z */
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = (cxs_tak((r[1]), (r[2]), (r[3])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;

case 2: /* listn k n */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_listn: /* k n */
  if (((0) == fixnum_from_obj(r[1]))) {
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (mknull());
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;
  } else {
    hreserve(hbsz(2+1), 2); /* 2 live regs */
    *--hp = r[1];  
    *--hp = r[0];  
    *--hp = obj_from_case(3);
    r[2] = (hendblk(2+1));
    r[0] = r[2];  
    r[1] = obj_from_fixnum(fixnum_from_obj(r[1]) - (1));
    goto gs_listn;
  }

case 3: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2]; }
    r += 1; /* shift reg. wnd */
    /* ek r k n */
    { /* cons */ 
    hreserve(hbsz(3), 4); /* 4 live regs */
    *--hp = r[1];  
    *--hp = r[3];  
    *--hp = obj_from_size(PAIR_BTAG); 
    r[4] = (hendblk(3)); }
    r[5+0] = r[2];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = obj_from_ktrap();
    r[5+2] = r[4];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;

case 4: /* length k l */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k l */
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = obj_from_ktrap();
    r[2+2] = (cxs_length((r[1])));
    r += 2; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;

case 5: /* shorterp k x y */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
    /* k x y */
    r[3+0] = r[0];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = obj_from_ktrap();
    r[3+2] = (cxs_shorterp((r[1]), (r[2])));
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;

case 6: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_l18 = r[1];  
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(7);
    r[2] = (hendblk(0+1));
    r[0] = r[2];  
    r[1] = obj_from_fixnum(12);
    goto gs_listn;

case 7: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_l12 = r[1];  
    hreserve(hbsz(0+1), 2); /* 2 live regs */
    *--hp = obj_from_case(8);
    r[2] = (hendblk(0+1));
    r[0] = r[2];  
    r[1] = obj_from_fixnum(6);
    goto gs_listn;

case 8: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_l6 = r[1];  
    { static obj c[] = { obj_from_case(9) }; cx_ltak = (obj)c; }
    r[2] = obj_from_void(0);
    r[3+0] = r[0];
    pc = 0; /* exit from module init */
    r[3+1] = r[2];  
    r += 3; /* shift reg wnd */
    rc = 2;
    goto jump;

case 9: /* ltak k x y z */
    assert(rc == 5);
    r += 1; /* shift reg. wnd */
    /* k x y z */
    r[4+0] = r[0];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = obj_from_ktrap();
    r[4+2] = (cxs_ltak((r[1]), (r[2]), (r[3])));
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;

default: /* inter-host call */
    cxg_hp = hp;
    cxm_rgc(r, r + MAX_LIVEREGS);
    cxg_rc = rc;
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
    cxg_rc = 0;
    while (pc) pc = (*(cxhost_t*)pc)(pc); 
    assert(cxg_rc == 2);
  }
}

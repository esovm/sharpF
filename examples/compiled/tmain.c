/* tmain.sf */
/* Generated by #F $Id: c.sf 22 2008-04-26 19:28:22Z esl@acm.org $ */
#ifdef PROFILE
#define host host_module_tmain
#endif
#define MODULE module_tmain
#define LOAD() module_tfun(); 
extern void module_tfun(void); /* tfun.sf */

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
extern obj cx_l12; /* l12 */
extern obj cx_l18; /* l18 */
extern obj cx_l6; /* l6 */
extern obj cx_length; /* length */
extern obj cx_ltak; /* ltak */
extern obj cx_tak; /* tak */
obj cx_main; /* main */
obj cx_runltak; /* runltak */
obj cx_runtak; /* runtak */

/* gc roots */
static cxroot_t root = { 0, NULL, NULL };

/* entry points */
static obj host(obj);
static obj cases[9] = {
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,
};

/* host procedure */
#define MAX_LIVEREGS 10
static obj host(obj pc)
{
  register obj *r = cxg_regs;
  register obj *hp = cxg_hp;
  register int rc = cxg_rc;
jump: 
  switch (case_from_obj(pc)) {

case 0: /* load module */
    { static obj c[] = { obj_from_case(1) }; cx_runtak = (obj)c; }
    { static obj c[] = { obj_from_case(3) }; cx_runltak = (obj)c; }
    { static obj c[] = { obj_from_case(6) }; cx_main = (obj)c; }
    r[0] = obj_from_void(0);
    r[1+0] = r[0];
    pc = 0; /* exit from module init */
    r[1+1] = r[0];  
    r += 1; /* shift reg wnd */
    rc = 2;
    goto jump;

case 1: /* runtak k n r */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_runtak: /* k n r */
  if ((fixnum_from_obj(r[1]) == (0))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;
  } else {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(2);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_tak);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = obj_from_fixnum(18);
    r[4+3] = obj_from_fixnum(12);
    r[4+4] = obj_from_fixnum(6);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 5;
    goto jump;
  }

case 2: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r n k */
    r[5+0] = r[4];  
    r[5+1] = obj_from_fixnum(fixnum_from_obj(r[3]) - (1));
    r[5+2] = obj_from_fixnum(fixnum_from_obj(r[2]) + fixnum_from_obj(r[1]));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_runtak;

case 3: /* runltak k n r */
    assert(rc == 4);
    r += 1; /* shift reg. wnd */
gs_runltak: /* k n r */
  if ((fixnum_from_obj(r[1]) == (0))) {
    /* r[0] */    
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    /* r[2] */    
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;
  } else {
    hreserve(hbsz(3+1), 3); /* 3 live regs */
    *--hp = r[0];  
    *--hp = r[1];  
    *--hp = r[2];  
    *--hp = obj_from_case(4);
    r[3] = (hendblk(3+1));
    r[4+0] = (cx_ltak);
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = (cx_l18);
    r[4+3] = (cx_l12);
    r[4+4] = (cx_l6);
    r += 4; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 5;
    goto jump;
  }

case 4: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r n k */
    hreserve(hbsz(3+1), 5); /* 5 live regs */
    *--hp = r[4];  
    *--hp = r[3];  
    *--hp = r[2];  
    *--hp = obj_from_case(5);
    r[5] = (hendblk(3+1));
    r[6+0] = (cx_length);
    pc = objptr_from_obj(r[6+0])[0];
    r[6+1] = r[5];  
    r[6+2] = r[1];  
    r += 6; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    rc = 3;
    goto jump;

case 5: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1];
    r[1+3] = p[2];
    r[1+4] = p[3]; }
    r += 1; /* shift reg. wnd */
    /* ek r r n k */
    r[5+0] = r[4];  
    r[5+1] = obj_from_fixnum(fixnum_from_obj(r[3]) - (1));
    r[5+2] = obj_from_fixnum(fixnum_from_obj(r[2]) + fixnum_from_obj(r[1]));
    r += 5; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_runltak;

case 6: /* main k argv */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* k argv */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(7);
    r[2] = (hendblk(1+1));
    r[3+0] = r[2];  
    r[3+1] = obj_from_fixnum(1000);
    r[3+2] = obj_from_fixnum(0);
    r += 3; /* shift reg wnd */
    rreserve(MAX_LIVEREGS);
    goto gs_runtak;

case 7: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    (void)(printf("%d", fixnum_from_obj(r[1])));
    (void)(putchar('\n'));
    hreserve(hbsz(1+1), 3); /* 3 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(8);
    r[3] = (hendblk(1+1));
    r[0] = r[3];  
    r[1] = obj_from_fixnum(1000);
    r[2] = obj_from_fixnum(0);
    goto gs_runltak;

case 8: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    (void)(printf("%d", fixnum_from_obj(r[1])));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(putchar('\n'));
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

/* basic runtime */
#define HEAP_SIZE 131072 /* 2^17 */

obj *cxg_heap = NULL;
cxoint_t cxg_hmask = 0;
obj *cxg_hp = NULL;
static cxroot_t cxg_root = { 0, NULL, NULL };
cxroot_t *cxg_rootp = &cxg_root;
obj cxg_regs[REGS_SIZE];
int cxg_rc = 0;

static obj *cxg_heap2 = NULL;
static size_t cxg_hsize = 0; 
static cxoint_t cxg_hmask2 = 0;
static int cxg_gccount = 0, cxg_bumpcount = 0;

static obj *toheap2(obj* p, obj *hp, obj *h1, cxoint_t m1, obj *h2, cxoint_t m2)
{
  obj o = *p, *op, fo, *fop;
  if (((char*)(o) - (char*)h1) & m1) return hp;
  fo = (op = objptr_from_obj(o))[-1]; assert(fo);
  if (notaptr(fo)) {
    fop = op + size_from_obj(fo); while (fop >= op) *--hp = *--fop;
    *p = *fop = obj_from_objptr(hp+1);
  } else if (((char*)(fo) - (char*)h2) & m2) {
    *--hp = *op--; *--hp = *op;
    *p = *op = obj_from_objptr(hp+1);
  } else *p = fo;
  return hp;
}

static void finalize(obj *hp1, obj *he1, obj *h2, cxoint_t m2)
{
  while (hp1 < he1) {
    obj fo = *hp1++; assert(fo);
    if (notaptr(fo)) hp1 += size_from_obj(fo);
    else if (((char*)(fo) - (char*)h2) & m2) ((cxtype_t*)fo)->free((void*)*hp1++);
    else if (notaptr(fo = objptr_from_obj(fo)[-1])) hp1 += size_from_obj(fo);
    else ++hp1;
  } assert(hp1 == he1);
}

static obj *relocate(cxroot_t *pr, obj *regs, obj *regp, 
obj *he2, obj *he1, obj *hp, obj *h1, cxoint_t m1, obj *h2, cxoint_t m2) 
{
  obj *p, *hp1 = hp; hp = he2;
  for (p = regs; p < regp; ++p) hp = toheap2(p, hp, h1, m1, h2, m2);
  for (; pr; pr = pr->next) {
    obj **pp = pr->globv; int c = pr->globc;
    while (c-- > 0) hp = toheap2(*pp++, hp, h1, m1, h2, m2);
  }
  for (p = he2; p > hp; --p) hp = toheap2(p-1, hp, h1, m1, h2, m2);
  if (he1) finalize(hp1, he1, h2, m2);
  return hp;
}

obj *cxm_hgc(obj *regs, obj *regp, obj *hp, size_t needs) 
{
  obj *h1 = cxg_heap, *h2 = cxg_heap2; cxoint_t m1 = cxg_hmask, m2 = cxg_hmask2;
  size_t hs = cxg_hsize; cxroot_t *pr = cxg_rootp;

  obj *h = h1, *he1 = h1 + hs, *he2 = h2 + hs; 
  ++cxg_gccount;
  if (h1) hp = relocate(pr, regs, regp, he2, he1, hp, h1, m1, h2, m2), 
    needs += (h2 + hs - hp)*2; /* make heap half empty */
  else hp = h2 + hs;
  if (hs < needs) {
    size_t s = HEAP_SIZE; while (s < needs) s *= 2;
    m2 = 1 | ~(s*sizeof(obj)-1);
    if (!(h = realloc(h1, s*sizeof(obj)))) { perror("alloc[h]"); exit(2); }
    h1 = h2; h2 = h; he2 = h2 + s; he1 = 0; /* no finalize flag */
    if (h1) hp = relocate(pr, regs, regp, he2, he1, hp, h1, m1, h2, m2);
    else hp = h2 + s;
    if (!(h = realloc(h1, s*sizeof(obj)))) { perror("alloc[h]"); exit(2); }
    hs = s; m1 = m2; ++cxg_bumpcount;
  }
  h1 = h2; h2 = h;

  cxg_heap = h1; cxg_hmask = m1; cxg_heap2 = h2; cxg_hmask2 = m2;
  cxg_hsize = hs; return cxg_hp = hp;
}

obj *cxm_rgc(obj *regs, obj *regp) 
{
  obj *p = cxg_regs;
  assert(regp <= cxg_regs + REGS_SIZE);
  while (regs < regp) *p++ = *regs++;
  return cxg_regs;
}

void cxm_check(int x, char *msg)
{
  if (!x) { perror(msg); exit(2); }
}

void *cxm_cknull(void *p, char *msg)
{
  if (!p) { perror(msg); exit(2); }
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
  cxg_rc = 3;
  pc = objptr_from_obj(cx_main)[0];
  while (pc) pc = (*(cxhost_t*)pc)(pc); 
  assert(cxg_rc == 3);
  res = (cxg_regs[2] != 0); 
  /* fprintf(stderr, "%d collections, %d reallocs\n", cxg_gccount, cxg_bumpcount); */
  return res; 
}

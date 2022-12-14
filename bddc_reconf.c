//
// bddc_reconf.c
//
// Copyright (c) 2020 -- 2022 Jun Kawahara and project "Fusion of Computer Science, Engineering
//     and Mathematics Approaches for Expanding Combinatorial Reconfiguration"
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "SAPPOROBDD/src/BDDc/bddc.c"
#include "bddc_reconf.h"

#define BC_REMOVEE    40
#define BC_ADDE       41
#define BC_SWAPE      42

#include <assert.h>

bddp remove_element(bddp f)
{
    struct B_NodeTable *fp;
    struct B_CacheTable *cachep;
    bddp f0, f1, r0, r1, h, key;
    bddvar var;

    if (f == bddnull) {
        return bddnull;
    }

    if (f == bddfalse || f == bddtrue) {
        return bddfalse;
    }

    fp = B_NP(f);
    f0 = B_GET_BDDP(fp->f0);
    f0 = B_NOT(f0);
    f1 = B_GET_BDDP(fp->f1);
    var = B_VAR_NP(fp);

    /* Checking Cache */
    if (B_RFC_ONE_NP(fp)) {
        key = bddnull;
    } else {
        key = B_CACHEKEY(BC_REMOVEE, f, (bddp)bddempty);
        cachep = Cache + key;
        if (cachep->op == BC_REMOVEE &&
            f == B_GET_BDDP(cachep->f) &&
            (bddp)bddempty == B_GET_BDDP(cachep->g)) {
            /* Hit */
            h = B_GET_BDDP(cachep->h);
            if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
            return h;
        }
    }

    h = remove_element(f0);
    if (h == bddnull) {
        return bddnull;
    }
    r0 = bddunion(h, f1);
    bddfree(h);
    if (r0 == bddnull) {
        return bddnull;
    }
    r1 = remove_element(f1);
    if (r1 == bddnull) {
        bddfree(r0);
        return bddnull;
    }
    h = getzbddp(var, r0, r1);
    if (h == bddnull) {
        bddfree(r0);
        bddfree(r1);
        return bddnull;
    }

    if (key != bddnull && h != bddnull) {
        cachep = Cache + key;
        cachep->op = BC_REMOVEE;
        B_SET_BDDP(cachep->f, f);
        B_SET_BDDP(cachep->g, (bddp)bddempty);
        B_SET_BDDP(cachep->h, h);
    }

    return h;
}

bddp remove_specified_element(bddp f, int* elements)
{
    struct B_NodeTable *fp;
    struct B_CacheTable *cachep;
    bddp f0, f1, r0, r1, h, key;
    bddvar var, lev;

    if (f == bddnull) {
        return bddnull;
    }

    if (f == bddfalse || f == bddtrue) {
        return bddfalse;
    }

    fp = B_NP(f);
    f0 = B_GET_BDDP(fp->f0);
    f0 = B_NOT(f0);
    f1 = B_GET_BDDP(fp->f1);
    var = B_VAR_NP(fp);
    lev = Var[var].lev;

    while (*elements >= 0 && bddlevofvar(*elements) > lev) {
        ++elements;
    }
    if (*elements < 0) {
        return bddfalse;
    }

    /* Checking Cache */
    /*if (B_RFC_ONE_NP(fp)) {
        key = bddnull;
    } else {
        key = B_CACHEKEY(BC_REMOVEE, f, (bddp)bddempty);
        cachep = Cache + key;
        if (cachep->op == BC_REMOVEE &&
            f == B_GET_BDDP(cachep->f) &&
            (bddp)bddempty == B_GET_BDDP(cachep->g)) {*/
            /* Hit */
    /*        h = B_GET_BDDP(cachep->h);
            if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
            return h;
        }
    }*/

    if (bddlevofvar(*elements) == lev) {
        ++elements;
        h = remove_specified_element(f0, elements);
        if (h == bddnull) {
            return bddnull;
        }
        r0 = bddunion(h, f1);
        bddfree(h);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = remove_specified_element(f1, elements);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    } else {
        r0 = remove_specified_element(f0, elements);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = remove_specified_element(f1, elements);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    }
    h = getzbddp(var, r0, r1);
    if (h == bddnull) {
        bddfree(r0);
        bddfree(r1);
        return bddnull;
    }

    /*if (key != bddnull && h != bddnull) {
        cachep = Cache + key;
        cachep->op = BC_REMOVEE;
        B_SET_BDDP(cachep->f, f);
        B_SET_BDDP(cachep->g, (bddp)bddempty);
        B_SET_BDDP(cachep->h, h);
    }*/

    return h;
}

bddp add_element(bddp f, int n)
{
    struct B_NodeTable *fp;
    struct B_CacheTable *cachep;
    bddp f0, f1, r0, r1, h, key;
    bddvar flev;

    if (f == bddnull) {
        return bddnull;
    }

    if (f == bddfalse) {
        return bddfalse;
    }

    if (f == bddtrue) {
        fp = NULL; // suppress warning
        flev = 0;
    } else {
        fp = B_NP(f);
        flev = Var[B_VAR_NP(fp)].lev;
    }

    assert((int)flev <= n);

    if (n == 0) {
        assert(f == bddtrue);
        return bddfalse;
    }

    /* Checking Cache */
    if (f == bddtrue) {
        key = bddnull;
    } else if (B_RFC_ONE_NP(fp)) {
        key = bddnull;
    } else {
        key = B_CACHEKEY(BC_ADDE, f, (bddp)n);
        cachep = Cache + key;
        if (cachep->op == BC_ADDE &&
            f == B_GET_BDDP(cachep->f) &&
            (bddp)n == B_GET_BDDP(cachep->g)) {
            /* Hit */
            h = B_GET_BDDP(cachep->h);
            if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
            return h;
        }
    }

    if ((int)flev == n) {
        f0 = B_GET_BDDP(fp->f0);
        assert(B_NEG(f0)); // because f is a ZDD node
        f1 = B_GET_BDDP(fp->f1);
        if (!B_NEG(f)) {
            f0 = B_NOT(f0);
        }

        r0 = add_element(f0, n - 1);
        if (r0 == bddnull) {
            return bddnull;
        }
        h = add_element(f1, n - 1);
        if (h == bddnull) {
            bddfree(r0);
            return bddnull;
        }
        r1 = bddunion(f0, h);
        bddfree(h);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    } else {
        assert((int)flev < n);
        r0 = add_element(f, n - 1);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = bddcopy(f);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    }
    h = getzbddp(bddvaroflev(n), r0, r1);
    if (h == bddnull) {
        bddfree(r0);
        bddfree(r1);
        return bddnull;
    }

    if (key != bddnull && h != bddnull) {
        cachep = Cache + key;
        cachep->op = BC_ADDE;
        B_SET_BDDP(cachep->f, f);
        B_SET_BDDP(cachep->g, (bddp)n);
        B_SET_BDDP(cachep->h, h);
    }

    return h;
}

bddp add_specified_element(bddp f, int* elements)
{
    struct B_NodeTable *fp;
    struct B_CacheTable *cachep;
    bddp f0, f1, r0, r1, h, key;
    bddvar flev, v;

    if (f == bddnull) {
        return bddnull;
    }

    if (f == bddfalse) {
        return bddfalse;
    }

    if (f == bddtrue) {
        fp = NULL; // suppress warning
        flev = 0;
    } else {
        fp = B_NP(f);
        flev = Var[B_VAR_NP(fp)].lev;
    }

    if (*elements < 0) {
        return bddfalse;
    }

    /* Checking Cache */
    /*if (f == bddtrue) {
        key = bddnull;
    } else if (B_RFC_ONE_NP(fp)) {
        key = bddnull;
    } else {
        key = B_CACHEKEY(BC_ADDE, f, (bddp)n);
        cachep = Cache + key;
        if (cachep->op == BC_ADDE &&
            f == B_GET_BDDP(cachep->f) &&
            (bddp)n == B_GET_BDDP(cachep->g)) {
          *//* Hit *//*
            h = B_GET_BDDP(cachep->h);
            if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
            return h;
        }
    }*/

    if (bddlevofvar(*elements) < flev) {
        f0 = B_GET_BDDP(fp->f0);
        assert(B_NEG(f0)); // because f is a ZDD node
        f1 = B_GET_BDDP(fp->f1);
        if (!B_NEG(f)) {
            f0 = B_NOT(f0);
        }

        v = B_VAR_NP(fp);
        r0 = add_specified_element(f0, elements);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = add_specified_element(f1, elements);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    } else if (bddlevofvar(*elements) == flev) {
        f0 = B_GET_BDDP(fp->f0);
        assert(B_NEG(f0)); // because f is a ZDD node
        f1 = B_GET_BDDP(fp->f1);
        if (!B_NEG(f)) {
            f0 = B_NOT(f0);
        }

        v = *elements;
        ++elements;
        r0 = add_specified_element(f0, elements);
        if (r0 == bddnull) {
            return bddnull;
        }
        h = add_specified_element(f1, elements);
        if (h == bddnull) {
            bddfree(r0);
            return bddnull;
        }
        r1 = bddunion(f0, h);
        bddfree(h);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    } else {
        v = *elements;
        ++elements;
        r0 = add_specified_element(f, elements);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = bddcopy(f);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    }
    h = getzbddp(v, r0, r1);
    if (h == bddnull) {
        bddfree(r0);
        bddfree(r1);
        return bddnull;
    }

    /*if (key != bddnull && h != bddnull) {
        cachep = Cache + key;
        cachep->op = BC_ADDE;
        B_SET_BDDP(cachep->f, f);
        B_SET_BDDP(cachep->g, (bddp)n);
        B_SET_BDDP(cachep->h, h);
    }*/

    return h;
}


bddp swap_element(bddp f, int n)
{
    struct B_NodeTable *fp;
    struct B_CacheTable *cachep;
    bddp f0, f1, r0, r1, h, h2, key;
    bddvar flev;

    if (f == bddnull) {
        return bddnull;
    }

    if (f == bddfalse || f == bddtrue) {
        return bddfalse;
    }

    fp = B_NP(f);
    flev = Var[B_VAR_NP(fp)].lev;

    assert((int)flev <= n);

    /* Checking Cache */
    if (B_RFC_ONE_NP(fp)) {
        key = bddnull;
    } else {
        key = B_CACHEKEY(BC_SWAPE, f, (bddp)n);
        cachep = Cache + key;
        if (cachep->op == BC_SWAPE &&
            f == B_GET_BDDP(cachep->f) &&
            (bddp)n == B_GET_BDDP(cachep->g)) {
            /* Hit */
            h = B_GET_BDDP(cachep->h);
            if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
            return h;
        }
    }

    if ((int)flev == n) {
        f0 = B_GET_BDDP(fp->f0);
        f0 = B_NOT(f0); // remove the ZDD flag
        f1 = B_GET_BDDP(fp->f1);

        h = swap_element(f0, n - 1);
        if (h == bddnull) {
            return bddnull;
        }
        h2 = add_element(f1, n - 1);
        if (h2 == bddnull) {
            bddfree(h);
            return bddnull;
        }
        r0 = bddunion(h, h2);
        bddfree(h);
        bddfree(h2);
        if (r0 == bddnull) {
            return bddnull;
        }
        h = swap_element(f1, n - 1);
        if (h == bddnull) {
            bddfree(r0);
            return bddnull;
        }
        h2 = remove_element(f0);
        if (h2 == bddnull) {
            bddfree(h);
            bddfree(r0);
            return bddnull;
        }
        r1 = bddunion(h, h2);
        bddfree(h);
        bddfree(h2);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    } else {
        assert((int)flev < n);
        r0 = swap_element(f, n - 1);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = remove_element(f);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    }
    h = getzbddp(bddvaroflev(n), r0, r1);
    if (h == bddnull) {
        bddfree(r0);
        bddfree(r1);
        return bddnull;
    }

    if (key != bddnull && h != bddnull) {
        cachep = Cache + key;
        cachep->op = BC_SWAPE;
        B_SET_BDDP(cachep->f, f);
        B_SET_BDDP(cachep->g, (bddp)n);
        B_SET_BDDP(cachep->h, h);
    }

    return h;
}

bddp swap_specified_element(bddp f, int* elements)
{
    struct B_NodeTable *fp;
    struct B_CacheTable *cachep;
    bddp f0, f1, r0, r1, h, h2, key;
    bddvar flev, v;

    if (f == bddnull) {
        return bddnull;
    }

    if (f == bddfalse || f == bddtrue) {
        return bddfalse;
    }

    if (*elements < 0) {
        return bddfalse;
    }

    fp = B_NP(f);
    flev = Var[B_VAR_NP(fp)].lev;

    /* Checking Cache */
    /*if (B_RFC_ONE_NP(fp)) {
        key = bddnull;
    } else {
        key = B_CACHEKEY(BC_SWAPE, f, (bddp)n);
        cachep = Cache + key;
        if (cachep->op == BC_SWAPE &&
            f == B_GET_BDDP(cachep->f) &&
            (bddp)n == B_GET_BDDP(cachep->g)) {
    *//* Hit *//*
            h = B_GET_BDDP(cachep->h);
            if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
            return h;
        }
        }*/

    if (bddlevofvar(*elements) < flev) {
        f0 = B_GET_BDDP(fp->f0);
        f0 = B_NOT(f0); // remove the ZDD flag
        f1 = B_GET_BDDP(fp->f1);

        v = B_VAR_NP(fp);
        r0 = swap_specified_element(f0, elements);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = swap_specified_element(f1, elements);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    } else if (bddlevofvar(*elements) == flev) {
        f0 = B_GET_BDDP(fp->f0);
        f0 = B_NOT(f0); // remove the ZDD flag
        f1 = B_GET_BDDP(fp->f1);

        v = *elements;
        ++elements;
        h = swap_specified_element(f0, elements);
        if (h == bddnull) {
            return bddnull;
        }
        h2 = add_specified_element(f1, elements);
        if (h2 == bddnull) {
            bddfree(h);
            return bddnull;
        }
        r0 = bddunion(h, h2);
        bddfree(h);
        bddfree(h2);
        if (r0 == bddnull) {
            return bddnull;
        }
        h = swap_specified_element(f1, elements);
        if (h == bddnull) {
            bddfree(r0);
            return bddnull;
        }
        h2 = remove_specified_element(f0, elements);
        if (h2 == bddnull) {
            bddfree(h);
            bddfree(r0);
            return bddnull;
        }
        r1 = bddunion(h, h2);
        bddfree(h);
        bddfree(h2);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    } else {
        v = *elements;
        ++elements;
        r0 = swap_specified_element(f, elements);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = remove_specified_element(f, elements);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    }
    h = getzbddp(v, r0, r1);
    if (h == bddnull) {
        bddfree(r0);
        bddfree(r1);
        return bddnull;
    }

    /*if (key != bddnull && h != bddnull) {
        cachep = Cache + key;
        cachep->op = BC_SWAPE;
        B_SET_BDDP(cachep->f, f);
        B_SET_BDDP(cachep->g, (bddp)n);
        B_SET_BDDP(cachep->h, h);
        }*/

    return h;
}

bddp swap_adjacent_element(bddp f, int* elements, int** adjacent)
{
    struct B_NodeTable *fp;
    struct B_CacheTable *cachep;
    bddp f0, f1, r0, r1, h, h2, key;
    bddvar flev, v;
    int* p;

    if (f == bddnull) {
        return bddnull;
    }

    if (f == bddfalse || f == bddtrue) {
        return bddfalse;
    }

    if (*elements < 0) {
        return bddfalse;
    }

    fp = B_NP(f);
    flev = Var[B_VAR_NP(fp)].lev;

    /* Checking Cache */
    /*if (B_RFC_ONE_NP(fp)) {
        key = bddnull;
    } else {
        key = B_CACHEKEY(BC_SWAPE, f, (bddp)n);
        cachep = Cache + key;
        if (cachep->op == BC_SWAPE &&
            f == B_GET_BDDP(cachep->f) &&
            (bddp)n == B_GET_BDDP(cachep->g)) {
    *//* Hit *//*
            h = B_GET_BDDP(cachep->h);
            if(!B_CST(h) && h != bddnull) { fp = B_NP(h); B_RFC_INC_NP(fp); }
            return h;
        }
        }*/

    if (bddlevofvar(*elements) < flev) {
        f0 = B_GET_BDDP(fp->f0);
        f0 = B_NOT(f0); // remove the ZDD flag
        f1 = B_GET_BDDP(fp->f1);

        v = B_VAR_NP(fp);
        r0 = swap_adjacent_element(f0, elements, adjacent);
        if (r0 == bddnull) {
            return bddnull;
        }
        r1 = swap_adjacent_element(f1, elements, adjacent);
        if (r1 == bddnull) {
            bddfree(r0);
            return bddnull;
        }
    } else {
        v = *elements;
        ++elements;
        p = adjacent[v];
        while (*p >= 0 && bddlevofvar(*p) > flev) {
            ++p;
        }

        if (bddlevofvar(v) == flev) {

            f0 = B_GET_BDDP(fp->f0);
            f0 = B_NOT(f0); // remove the ZDD flag
            f1 = B_GET_BDDP(fp->f1);

            h = swap_adjacent_element(f0, elements, adjacent);
            if (h == bddnull) {
                return bddnull;
            }
            h2 = add_specified_element(f1, p);
            if (h2 == bddnull) {
                bddfree(h);
                return bddnull;
            }
            r0 = bddunion(h, h2);
            bddfree(h);
            bddfree(h2);
            if (r0 == bddnull) {
                return bddnull;
            }
            h = swap_adjacent_element(f1, elements, adjacent);
            if (h == bddnull) {
                bddfree(r0);
                return bddnull;
            }
            h2 = remove_specified_element(f0, p);
            if (h2 == bddnull) {
                bddfree(h);
                bddfree(r0);
                return bddnull;
            }
            r1 = bddunion(h, h2);
            bddfree(h);
            bddfree(h2);
            if (r1 == bddnull) {
                bddfree(r0);
                return bddnull;
            }
        } else {
            r0 = swap_adjacent_element(f, elements, adjacent);
            if (r0 == bddnull) {
                return bddnull;
            }
            r1 = remove_specified_element(f, p);
            if (r1 == bddnull) {
                bddfree(r0);
                return bddnull;
            }
        }
    }
    h = getzbddp(v, r0, r1);
    if (h == bddnull) {
        bddfree(r0);
        bddfree(r1);
        return bddnull;
    }

    /*if (key != bddnull && h != bddnull) {
        cachep = Cache + key;
        cachep->op = BC_SWAPE;
        B_SET_BDDP(cachep->f, f);
        B_SET_BDDP(cachep->g, (bddp)n);
        B_SET_BDDP(cachep->h, h);
        }*/

    return h;
}

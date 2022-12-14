//
// RandomSample.hpp
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

#include "sbdd_helper/SBDD_helper.h"
#include "BigInteger.hpp"

typedef std::map<bddp, BigInteger> ZBDD_CountMap;

inline BigInteger ZBDD_CountSolutionsInner(bddp f, ZBDD_CountMap* cmap)
{
    if (f == bddfalse) {
        return BigInteger(0);
    } else if (f == bddtrue) {
        return BigInteger(1);
    } else {
        if (cmap->count(f) != 0) {
            return (*cmap)[f];
        } else {
            //std::cerr << "# " << f << std::endl;
            //std::cerr << "# " << bddgetchild0zraw_nc(f) << ", " << bddgetchild1zraw_nc(f) <<  std::endl;
            BigInteger bi = ZBDD_CountSolutionsInner(bddgetchild0zraw(f), cmap)
                + ZBDD_CountSolutionsInner(bddgetchild1zraw(f), cmap);
            if (bddisnegative(f)) {
                bi += 1;
            }
            //std::cerr << ":" << f << ", " << bi << std::endl;
            cmap->insert(std::make_pair(f, bi));
            return bi;
        }
    }
}

inline BigInteger ZBDD_CountSolutions(const ZBDD& f, ZBDD_CountMap* cmap = NULL)
{
    bddp p = f.GetID();
    if (p == bddempty) {
        return BigInteger(0);
    } else if (p == bddsingle) {
        return BigInteger(1);
    } else {
        if (cmap != NULL) {
            return ZBDD_CountSolutionsInner(p, cmap);
        } else {
            std::map<bddp, BigInteger> cmap_temp;
            return ZBDD_CountSolutionsInner(p, &cmap_temp);
        }
    }
}

inline int ZBDD_GetIthSetInner(bddp f, const BigInteger& index, const ZBDD_CountMap& cmap,
                               std::set<bddvar>* result, bool is_zero)
{
    //std::cerr << "f = " << f << std::endl;

    if (index.IsZero()) {
        return -1;
    }

    if (f == bddempty) {
        // invalid index
        return -1;
    } else if (f == bddsingle) {
        return 1;
    }
    bddp hi = bddgetchild1z(f);
    BigInteger hinum;

    if (hi == bddempty) {
        hinum = BigInteger(0);
    } else if (hi == bddsingle) {
        hinum = BigInteger(1);
    } else {
        hinum = cmap.at(hi);
    }

    if (!is_zero && bddisnegative(f)) {
        if (index.Equals(1)) {
            return 1;
        }
        hinum += 1;
        if (index <= hinum) {
            result->insert(bddgetvar(f));
            return ZBDD_GetIthSetInner(hi, index - BigInteger(1), cmap, result, false);
        } else {
            return ZBDD_GetIthSetInner(bddgetchild0z(f), index - hinum, cmap, result, true);
        }
    } else {
        if (index <= hinum) {
            result->insert(bddgetvar(f));
            return ZBDD_GetIthSetInner(hi, index, cmap, result, false);
        } else {
            return ZBDD_GetIthSetInner(bddgetchild0z(f), index - hinum, cmap, result, true);
        }
    }
}

// return -1 if index is out of range, otherwise 1
inline int ZBDD_GetIthSet(const ZBDD& f, const BigInteger& index, const ZBDD_CountMap& cmap, std::set<bddvar>* result_set)
{
    assert(result_set != NULL);

    bddp p = f.GetID();
    if (p == bddempty) {
        return -1;
    } else if (p == bddsingle) {
        if (index.Equals(1)) {
            result_set->clear();
            return 1;
        } else {
            return -1;
        }
        //} else if (index > cmap.at(p)) { // out of range, need to impl. (need '>' for BigInteger)
        //        return -1;
    } else {
        result_set->clear();
        return ZBDD_GetIthSetInner(p, index, cmap, result_set, false);
    }
}

inline std::set<bddvar> ZBDD_SampleRandomly(const ZBDD& f, const ZBDD_CountMap& cmap, BigIntegerRandom& random)
{
    std::set<bddvar> result_set;
    BigInteger index(random.GetRand(cmap.at(f.GetID())));
    index += 1;
    //std::cout << cmap.at(f.GetID()).GetString() << ", index: " << index.GetString() << std::endl;
    ZBDD_GetIthSet(f, index, cmap, &result_set);
    return result_set;
}

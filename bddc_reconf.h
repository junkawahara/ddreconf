//
// bddc_reconf.h
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

#ifndef bddc_reconf_h
#define bddc_reconf_h

#ifdef BDD_CPP
  extern "C" {
#endif /* BDD_CPP */

extern bddp   remove_element B_ARG((bddp f));
extern bddp   remove_specified_element B_ARG((bddp f, int* elements));
extern bddp   add_element B_ARG((bddp f, int n));
extern bddp   add_specified_element B_ARG((bddp f, int* elements));
extern bddp   swap_element B_ARG((bddp f, int n));
extern bddp   swap_specified_element B_ARG((bddp f, int* elements));
extern bddp   swap_adjacent_element B_ARG((bddp f, int* elements, int** adjacent));

#ifdef BDD_CPP
  }
#endif /* BDD_CPP */

#endif /* bddc_reconf_h */

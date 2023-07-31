//
// Reconf.hpp
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

#ifndef RECONF_HPP
#define RECONF_HPP

#include <vector>
#include <list>
#include <algorithm>
#include <random>
#include "tdzdd/spec/SizeConstraint.hpp"

#include "BigInteger.hpp"

enum Mode {ST, LONGEST};

class Reconf {
private:
    int num_elements_;
    enum Mode mode_;
    enum Model model_;
    ZBDD solution_space_zdd_;
    std::vector<ZBDD> F_;
    std::vector<ZBDD> Fg_; // only used for the bidirection mode
    BigIntegerRandom& random_;
    bool is_edge_variable_;
    bool is_zdd_store_;
    std::string zdd_dirname_;
    bool show_info_;
    bool show_info_verbose_;
    bool is_gc_;
    Graph graph_;

public:
    Reconf(BigIntegerRandom& random, int num_elements,
           const Graph& graph,
           bool is_edge_variable, bool show_info, bool is_gc)
        : num_elements_(num_elements),
          model_(TJ),
          random_(random),
          is_edge_variable_(is_edge_variable), is_zdd_store_(false),
          show_info_(show_info),
          show_info_verbose_(false), is_gc_(is_gc), graph_(graph) {}

    void setNumElements(int num_elements)
    {
        num_elements_ = num_elements;
    }

    void setSolutionSpaceZdd(const ZBDD& solution_space_zdd)
    {
        solution_space_zdd_ = solution_space_zdd;
    }

    void setSolutionSpaceZddTar(const ZBDD& solution_space_zdd, int k)
    {
        IntRange at_least_k(k, num_elements_);
        SizeConstraint sc(num_elements_, at_least_k);
        DdStructure<2> dd(sc);
        ZBDD sczdd = dd.evaluate(ToZBDD());
        solution_space_zdd_ = solution_space_zdd & sczdd;
    }

    void setModelTar()
    {
        model_ = TAR;
    }

    void setZddDir(const std::string& zdd_dirname)
    {
        is_zdd_store_ = true;
        zdd_dirname_ = zdd_dirname;
    }

    void findReconfSeq(const std::set<bddvar>& start_set,
                       const std::set<bddvar>& goal_set)
    {
        mode_ = ST;
        F_.clear();
        int step = reconfigure(start_set, goal_set);
        if (step < 0) {
            std::cout << "a NO" << std::endl;
        } else {
            std::cout << "a YES" << std::endl;
            std::list<std::set<bddvar> > sequence;
            backtrack(start_set, goal_set, F_, &sequence);
            outputSequence(sequence, std::cout);
        }
    }

    void findReconfSeqBoth(const std::set<bddvar>& start_set,
                           const std::set<bddvar>& goal_set)
    {
        mode_ = ST;
        F_.clear();
        int step = reconfigureBoth(start_set, goal_set);
        if (step < 0) {
            std::cout << "a NO" << std::endl;
        } else {
            std::cout << "a YES" << std::endl;
            if (step >= 2) {
                std::set<bddvar> middle_set = pickRandomly(F_.back() & Fg_.back(), random_);
                std::list<std::set<bddvar> > first_sequence;
                backtrack(start_set, middle_set, F_, &first_sequence);
                std::list<std::set<bddvar> > second_sequence;
                backtrack(goal_set, middle_set, Fg_, &second_sequence);
                std::list<std::set<bddvar> >::const_reverse_iterator itor;
                itor = second_sequence.rbegin();
                ++itor; // ignore the first element
                for ( ; itor != second_sequence.rend(); ++itor) {
                    first_sequence.push_back(*itor);
                }
                outputSequence(first_sequence, std::cout);
            } else {
                std::list<std::set<bddvar> > sequence;
                backtrack(start_set, goal_set, F_, &sequence);
                outputSequence(sequence, std::cout);
            }
        }
    }

    void findReconfSeqAndWidth(const std::set<bddvar>& start_set,
                               const std::set<bddvar>& goal_set)
    {
        mode_ = ST;
        F_.clear();
        int step = reconfigureForWidth(start_set, goal_set);
        if (step < 0) {
            std::cout << "a NO" << std::endl;
        } else {
            std::cout << "a YES" << std::endl;
            std::list<std::set<bddvar> > sequence;
            backtrack(start_set, goal_set, F_, &sequence);
            outputSequence(sequence, std::cout);
        }
    }

    void findReconfLongestSeq(const std::set<bddvar>& start_set)
    {
        mode_ = LONGEST;
        F_.clear();
        // The second argument is empty because it is not used
        // when computing the longest sequence
        int step = reconfigure(start_set, std::set<bddvar>());
        std::set<bddvar> goal_set = pickRandomly(F_[step], random_);
        std::list<std::set<bddvar> > sequence;
        backtrack(start_set, goal_set, F_, &sequence);
        outputSequence(sequence, std::cout);
    }

    // return -1 if the reconf seq is not found
    // otherwise, the returned value is the positive integer
    // representing the number of steps
    int reconfigure(const std::set<bddvar>& start_set,
                    const std::set<bddvar>& goal_set)
    {
        if (!isMemberZ(solution_space_zdd_, start_set)) {
            std::cerr << "The start set is not in the solution space." << std::endl;
            exit(2);
        }
        if (mode_ == ST) {
            if (!isMemberZ(solution_space_zdd_, goal_set)) {
                std::cerr << "The goal set is not in the solution space." << std::endl;
                exit(3);
            }
        }
        ZBDD start_zdd = getSingleSet(start_set);
        F_.push_back(start_zdd);

        if (start_set == goal_set) {
            return 0;
        }

        ZBDD zbdd_null = ZBDD(-1);
        ZBDD zbdd_empty = ZBDD(0);
        int step;
        for (step = 1; ; ++step) {
            if (show_info_) {
                std::cerr << "Step " << step << " ";
            }
            ZBDD previous_zdd = ZBDD(0); // empty zdd
            if (step >= 2) {
                previous_zdd = F_[step - 2];
            }

            ZBDD next_zdd = getNextStep(F_[step - 1], previous_zdd);
            F_.push_back(next_zdd);

            if (is_zdd_store_) {
                if (step >= 1002 && step % 1000 == 2) {
                    for (int j = step - 1000 - 1; j < step - 1; ++j) {
                        std::stringstream ss;
                        ss << zdd_dirname_ << j;
                        FILE* fp = fopen(ss.str().c_str(), "w");
                        if (fp == NULL) {
                            std::cerr << "File cannot be opened" << std::endl;
                            exit(6);
                        }
                        F_[j].Export(fp);
                        fclose(fp);
                        //std::cout << "erased " << j << " " << F_[j].Size() << std::endl;
                        F_[j] = zbdd_null;
                    }
                    BDD_GC();
                }
            }

            if (mode_ == ST) {
                // check whether goal_set is in next_zdd
                if (isMemberZ(next_zdd, goal_set)) {
                    if (show_info_) {
                        std::cerr << "t found" << std::endl;
                    }
                    return step;
                }
            }

            if (next_zdd == zbdd_empty) {
                if (mode_ == ST) {
                    return -1;
                } else { // mode_ == LONGEST
                    // The goal set whose step is the longest is
                    // in F_[step - 1] because F_[step] is empty.
                    return step - 1;
                }
            }
            if (!is_zdd_store_ && is_gc_ && step % 1000 == 0) {
                BDD_GC();
            }
        }
        // never come here
    }

    ZBDD getNextStep(const ZBDD& current_zdd, const ZBDD& previous_zdd)
    {
        double start_time = getTime();
        double total_start_time = start_time;
        ZBDD next_zdd;
        if (model_ == TJ) {
            next_zdd = swapElement(current_zdd, num_elements_);
        } else if (model_ == TAR) {
            next_zdd = removeElement(current_zdd)
                + addElement(current_zdd, num_elements_);
        }
        if (next_zdd == ZBDD(-1)) {
            std::cerr << "Cannot construct the remove/add/swap ZDD due to the memory shortage" << std::endl;
            exit(1);
        }
        double end_time = getTime();
        if (show_info_verbose_) {
            std::cerr << "time = " << (end_time - start_time) << std::endl;
            std::cerr << "size = " << next_zdd.Size() << std::endl;
            std::cerr << "card = " << getCard(next_zdd) << std::endl;
        }

        start_time = getTime();
        next_zdd = solution_space_zdd_ & next_zdd;
        end_time = getTime();
        if (show_info_verbose_) {
            std::cerr << "time = " << (end_time - start_time) << std::endl;
            std::cerr << "size = " << next_zdd.Size() << std::endl;
            std::cerr << "card = " << getCard(next_zdd) << std::endl;
        }

        start_time = getTime();
        next_zdd = next_zdd - current_zdd - previous_zdd;
        end_time = getTime();
        if (show_info_verbose_) {
            std::cerr << "time = " << (end_time - start_time) << std::endl;
            std::cerr << "size = " << next_zdd.Size() << std::endl;
            std::cerr << "card = " << getCard(next_zdd) << std::endl;
        } else if (show_info_) {
            DDNodeIndex index(next_zdd, false);
            std::cerr << "time = " << (getTime() - total_start_time)
                      << ", # ZDD nodes = " << index.size()
                      << ", # elems = " << getCard(next_zdd) << std::endl;
        }
        if (next_zdd == ZBDD(-1)) {
            std::cerr << "Cannot construct a ZDD due to the memory shortage" << std::endl;
            exit(1);
        }
        return next_zdd;
    }

    // return -1 if the reconf seq is not found
    // otherwise, the returned value is the positive integer
    // representing the number of steps
    int reconfigureBoth(const std::set<bddvar>& start_set,
                        const std::set<bddvar>& goal_set)
    {
        assert(mode_ == ST);

        if (!isMemberZ(solution_space_zdd_, start_set)) {
            std::cerr << "The start set is not in the solution space." << std::endl;
            exit(2);
        }
        if (mode_ == ST) {
            if (!isMemberZ(solution_space_zdd_, goal_set)) {
                std::cerr << "The goal set is not in the solution space." << std::endl;
                exit(3);
            }
        }
        ZBDD start_zdd = getSingleSet(start_set);
        F_.push_back(start_zdd);
        ZBDD goal_zdd = getSingleSet(goal_set);
        Fg_.push_back(goal_zdd);

        int step;
        for (step = 1; ; ++step) {
            if (show_info_) {
                std::cerr << "Step " << step << " ";
            }
            std::vector<ZBDD>& Fc = (step % 2 == 1 ? F_ : Fg_);

            ZBDD previous_zdd = ZBDD(0); // empty zdd
            if (Fc.size() >= 2) {
                previous_zdd = Fc[Fc.size() - 2];
            }

            ZBDD next_zdd = getNextStep(Fc.back(), previous_zdd);
            if (next_zdd.Card() == 0) {
                return -1;
            }
            Fc.push_back(next_zdd);

            // check whether goal_set is in next_zdd
            if ((F_.back() & Fg_.back()) != ZBDD(0)) {
                if (show_info_) {
                    std::cerr << "t found" << std::endl;
                }
                return step;
            }

            if (is_gc_ && step % 1000 == 0) {
                BDD_GC();
            }
        }
        // never come here
    }

    // return -1 if the reconf seq is not found
    // otherwise, the returned value is the positive integer
    // representing the number of steps
    int reconfigureForWidth(const std::set<bddvar>& start_set,
                            const std::set<bddvar>& goal_set)
    {
        assert(mode_ == ST);

        int shortest_length = (1 << 28);
        bool found_shortest = false;

        if (!isMemberZ(solution_space_zdd_, start_set)) {
            std::cerr << "The start set is not in the solution space." << std::endl;
            exit(2);
        }
        if (mode_ == ST) {
            if (!isMemberZ(solution_space_zdd_, goal_set)) {
                std::cerr << "The goal set is not in the solution space." << std::endl;
                exit(3);
            }
        }
        ZBDD start_zdd = getSingleSet(start_set);
        F_.push_back(start_zdd);
        ZBDD goal_zdd = getSingleSet(goal_set);
        Fg_.push_back(goal_zdd);

        int step;
        for (step = 1; step <= shortest_length * 2; ++step) {
            if (show_info_) {
                if (step % 2 == 1) {
                    std::cerr << "Zs_";
                } else {
                    std::cerr << "Zt_";
                }
                std::cerr << ((step + 1) / 2) << " construction ";
            }
            std::vector<ZBDD>& Fc = (step % 2 == 1 ? F_ : Fg_);

            ZBDD previous_zdd = ZBDD(0); // empty zdd
            if (Fc.size() >= 2) {
                previous_zdd = Fc[Fc.size() - 2];
            }

            ZBDD next_zdd = getNextStep(Fc.back(), previous_zdd);
            if (next_zdd.Card() == 0) {
                return -1;
            }
            Fc.push_back(next_zdd);

            // check whether goal_set is in next_zdd
            if (!found_shortest) {
                if ((F_.back() & Fg_.back()) != ZBDD(0)) {
                    // shortest sequence found
                    found_shortest = true;
                    shortest_length = step;
                    if (show_info_) {
                        std::cerr << "Reconf seq found. Shortest length = " << shortest_length << std::endl;
                    }
                    //return step;
                }
            }

            if (is_gc_ && step % 1000 == 0) {
                BDD_GC();
            }
        }
        int max_card = 0;
        int max_card_i = -1;
        for (int i = 0; i <= shortest_length; ++i) {
            int rev = shortest_length - i;
            int card = (F_[i] & Fg_[rev]).Card();
            std::cerr << "Card(Zs_" << i << " & Zt_" << rev << ") = " << card << std::endl;
            if (card > max_card) {
                max_card = card;
                max_card_i = i;
            }
        }
        std::cerr << "max card = " << max_card << std::endl;
        std::cerr << "max card index = " << max_card_i << std::endl;
        return (found_shortest ? shortest_length : -1);
    }

    // Assume that zdd_sequence[0] is a ZDD containing only start_set,
    // and zdd_sequence[size - 1] is a ZDD containing goal_set.
    // Note that the content of zdd_sequence is erased.
    void backtrack(const std::set<bddvar>& start_set,
                   const std::set<bddvar>& goal_set,
                   std::vector<ZBDD>& zdd_sequence,
                   std::list<std::set<bddvar> >* output_sequence)
    {
        std::set<bddvar> current_set = goal_set;

        output_sequence->push_back(goal_set);

        ZBDD zbdd_null = ZBDD(-1);

        for (int i = static_cast<int>(zdd_sequence.size()) - 2;
                 i >= 1; --i) {
            bool found_elem = false;
            if (model_ == TJ) {
                for (int v = 1; v <= num_elements_; ++v) {
                    if (current_set.count(v) > 0) { // v found
                        for (int w = 1; w <= num_elements_; ++w) {
                            if (v != w) {
                                if (current_set.count(w) == 0) { // w not found
                                    current_set.insert(w);
                                    current_set.erase(v);
                                    if (isMemberZ(zdd_sequence[i], current_set)) {
                                        found_elem = true;
                                        break;
                                    }
                                    current_set.insert(v);
                                    current_set.erase(w);
                                }
                            }
                        }
                        if (found_elem) {
                            break;
                        }
                    }
                }
            } else if (model_ == TAR) {
                for (int v = 1; v <= num_elements_; ++v) {
                    if (current_set.count(v) > 0) { // v found
                        current_set.erase(v);
                        if (isMemberZ(zdd_sequence[i], current_set)) {
                            found_elem = true;
                            break;
                        }
                        current_set.insert(v);
                    } else { // v not found
                        current_set.insert(v);
                        if (isMemberZ(zdd_sequence[i], current_set)) {
                            found_elem = true;
                            break;
                        }
                        current_set.erase(v);
                    }
                }
            }
            if (found_elem) {
                output_sequence->push_front(current_set);
            } else {
                std::cerr << "backtrack failed" << std::endl;
            }
            if (is_zdd_store_ && i >= 1002 && i % 1000 == 2) {
                for (int j = i - 1000 - 1; j < i - 1; ++j) {
                    std::stringstream ss;
                    ss << zdd_dirname_ << j;
                    FILE* fp = fopen(ss.str().c_str(), "r");
                    if (fp == NULL) {
                        std::cerr << "File cannot be opened" << std::endl;
                        exit(6);
                    }
                    zdd_sequence[j] = ZBDD_Import(fp);
                    fclose(fp);
                    //std::cout << "restored " << j << " " << zdd_sequence[j].Size() << std::endl;
                }
                for (int j = i; j < i + 1000 && j < static_cast<int>(zdd_sequence.size()) - 1; ++j) {
                    zdd_sequence[j] = zbdd_null;
                //    std::cout << "erased " << j << std::endl;
                }
                BDD_GC();
            }
        }
        // zdd_sequence.size() == 1 means that start_set == goal_set (0 step)
        if (zdd_sequence.size() >= 2) {
            output_sequence->push_front(start_set);
        }
    }

    // Output the reconfiguration sequence
    void outputSequence(const std::list<std::set<bddvar> >& output_sequence,
                        std::ostream& ost) {
        for (std::list<std::set<bddvar> >::const_iterator it1 =
                output_sequence.begin(); it1 != output_sequence.end(); ++it1) {
            std::set<bddvar> vec = *it1;
            std::vector<bddvar> elems;
            for (std::set<bddvar>::const_iterator it2 = vec.begin();
                 it2 != vec.end(); ++it2) {
                if (is_edge_variable_) {
                    elems.push_back(num_elements_ + 1 - *it2);
                } else {
                    elems.push_back(getVertexNumber(graph_, *it2));
                }
            }
            std::sort(elems.begin(), elems.end());

            ost << "a ";
            for (size_t i = 0; i < elems.size(); ++i) {
                if (i != 0) {
                    ost << " ";
                }
                ost << elems[i];
            }
            ost << std::endl;
        }
    }
};

#endif // RECONF_HPP

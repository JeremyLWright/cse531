/*******************************************************************************
 * FILENAME:    q.test.cpp
 * DESCRIPTION: unit tests for user-space threads and queue
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Dstributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
#pragma once

#include <gtest/gtest.h>
#include <deque>
#include <functional>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>
#include <iterator>

class Model : public ::testing::Test 
{
    protected:
        Model():
            seed(std::chrono::system_clock::now().time_since_epoch().count()),
            ntests(0, 1000)
        {
            test_size = ntests(rng);
            rng.seed(seed);
            InitQ(&q);
        }

        ~Model()
        {
        }

        size_t const seed;
        std::mt19937 rng;
        std::uniform_int_distribution<int> ntests;
        size_t test_size;
        Q q;
};
#if 0

template <typename RNG, typename D>
bool run_isolated_test(int ninserts, int npops, int nreinserts, D dist, RNG entropy)
{
    Q q;
    InitQ(&q);
    std::deque<int> model;
    auto result(true);

   // std::cout << "Inserting\n";
    for(auto n = 0; n < ninserts; ++n)
    {
        auto d = dist(entropy);
        model.push_back(d);
        AddQ(&q, &d);
    }
    if(size_(&q) != model.size()) return false;

    //std::cout << "Deleting\n";
    for(auto n = 0; n < npops; ++n)
    {
        if(model.size() > 0) model.pop_front(); //Delete from empty deque is undefined behavior! Fucking Compiler!
        DelQ(&q);
    }
    
    //std::cout << "Inserting\n";
    for(auto n = 0; n < nreinserts; ++n)
    {
        auto d = dist(entropy);
        model.push_back(d);
        AddQ(&q, &d);
    }
    if(size_(&q) != model.size()) return false;
    
    //std::cout << "Comparing\n";
    for(auto r : model)
    {
        auto u = DelQ(&q);
        if(u == 0)
        {
            result = false;
#if 0
            std::cout << "Queue Ended Early.\n";
#endif
        }
        else
        {

         //   std::cout << "Actual: " << r << " Expected: " << *u <<'\n';
        if(r != u->data)
        {
            result = false;
#if 0
            std::cout << "Actual: " << r << " Expected: " << *u <<'\n';
            std::cout << "Test:\n"
                << "\tInserts: " << ninserts << '\n'
                << "\tDeletes: " << npops << '\n'
                << "\tReinserts: " << nreinserts << '\n';
#endif
        }
        }
    }
//    FreeQ(&q);
    return result;
}
bool run_isolated_test(int ninserts, int npops, int nreinserts)
{
    std::mt19937 rng(0);
    std::uniform_int_distribution<int> input_data(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    return run_isolated_test(ninserts, npops, nreinserts, input_data, rng);
}
#endif

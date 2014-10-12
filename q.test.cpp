extern "C" {
#ifndef LIST_PARAM
    #define LIST_PARAM
    typedef int list_parameter_t;
#endif
#include "q.h"
}


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
        virtual void SetUp()
        {
            seed = std::chrono::system_clock::now().time_since_epoch().count();
            rng.seed(seed);
        }

        virtual void TearDown()
        {
        }

        std::mt19937 rng;
        size_t seed;
};


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
        if(r != *u)
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
    FreeQ(&q);
    return result;
}
bool run_isolated_test(int ninserts, int npops, int nreinserts)
{
    std::mt19937 rng(0);
    std::uniform_int_distribution<int> input_data(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    return run_isolated_test(ninserts, npops, nreinserts, input_data, rng);
}

TEST_F(Model, RandomizedModel)
{
    std::uniform_int_distribution<int> ntests(0, 1000);
    std::uniform_int_distribution<int> input_data(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

    //Generate a random number of inserts
    auto ninserts = ntests(rng);

    //Generate a random number of reads/pops
    auto npops = ntests(rng);

    //Generate a random number of more inserts to verify circular
    //dynamic functionality.  Verify we can write, read, write and read
    //again in any amount.
    auto nreinserts = ntests(rng);

     std::cout << "Test:\n"
    << "\tSeed: " << seed << '\n'
    << "\tInserts: " << ninserts << '\n'
    << "\tDeletes: " << npops << '\n'
    << "\tReinserts: " << nreinserts << '\n';
    //Verify from the model
    if(!run_isolated_test(ninserts, npops, nreinserts, input_data, rng))
    {
        std::cout << "Test Failed, minimizing.\n";
        while(ninserts > 0 && run_isolated_test(ninserts, npops, nreinserts, input_data, rng) == false)
        {
            --ninserts;
        }
        ++ninserts; //Make it fail again.
        while(npops > 0 && run_isolated_test(ninserts, npops, nreinserts, input_data, rng) == false)
        {
            --npops;
        }
        ++npops; //Make it fail again.
        while(nreinserts > 0 && run_isolated_test(ninserts, npops, nreinserts, input_data, rng) == false)
        {
            --nreinserts;
        }
        ++nreinserts; //Make it fail again.

        ADD_FAILURE();
        std::cout << "Test:\n"
            << "\tInserts: " << ninserts << '\n'
            << "\tDeletes: " << npops << '\n'
            << "\tReinserts: " << nreinserts << '\n';
    }
}

class Directed : public ::testing::Test 
{
    protected:
        virtual void SetUp()
        {
            InitQ(&q);
        }

        virtual void TearDown()
        {
            FreeQ(&q);
        }

        Q q;
};

TEST_F(Directed, RandomMinimizedTest)
{
    ASSERT_TRUE(run_isolated_test(1, 0, 9)); //Random test generator found this test case
}
TEST_F(Directed, Weirdness)
{
    ASSERT_TRUE(run_isolated_test(10240, 0, 0)); 
}
TEST_F(Directed, RandomMinimizedTest2)
{
    ASSERT_TRUE(run_isolated_test(1, 1, 8));
}
TEST_F(Directed, RandomMinimizedTest3)
{
    ASSERT_TRUE(run_isolated_test(1, 1, 9));
}
TEST_F(Directed, RandomMinimizedTest4)
{
    ASSERT_TRUE(run_isolated_test(1, 2, 2));
}

TEST_F(Directed, Add)
{
    for(int i = 1; i < 100; ++i)
    {
        AddQ(&q, &i);
        ASSERT_EQ(i, size_(&q));
    }
    ASSERT_EQ(99, size_(&q));
    for(int i = 1; i < 100; ++i)
    {
        ASSERT_EQ(i, *DelQ(&q));
    }
    ASSERT_EQ(0, size_(&q));
}

TEST_F(Directed, PutGet)
{
    int i = 8;
    AddQ(&q, &i);
    ASSERT_EQ(8, *DelQ(&q));
}

TEST_F(Directed, DeleteEmpty)
{
    InitQ(&q);
    DelQ(&q);
    RotateQ(&q);
    int i = 9;
    AddQ(&q, &i);
    RotateQ(&q);
}

TEST_F(Directed, Compact)
{
    int i;
    i = 0;
    AddQ(&q, &i);
    EXPECT_EQ(0, *PeekQ(&q));
    EXPECT_EQ(q.head, q.curr_read);
    DelQ(&q);
    DelQ(&q);
    EXPECT_EQ(q.head+1, q.curr_read);
    i = 1;
    AddQ(&q, &i);
    i = 2;
    AddQ(&q, &i);
    ASSERT_EQ(1, *q.curr_read);
    ASSERT_EQ(2, *(q.curr_read+1));
    ASSERT_EQ(1, *DelQ(&q));
    ASSERT_EQ(2, *DelQ(&q));

}

TEST_F(Directed, Rotate)
{
    for(int i = 0; i < 4; ++i)
    {
        AddQ(&q, &i);
    }

    for(int i = 0; i < 100; ++i)
    {
    ASSERT_EQ(0, *PeekQ(&q));
    ASSERT_EQ(1, *RotateQ(&q));
    ASSERT_EQ(1, *PeekQ(&q));
    ASSERT_EQ(2, *RotateQ(&q));
    ASSERT_EQ(2, *PeekQ(&q));
    ASSERT_EQ(3, *RotateQ(&q));
    ASSERT_EQ(3, *PeekQ(&q));
    ASSERT_EQ(0, *RotateQ(&q));
    ASSERT_EQ(0, *PeekQ(&q));
    }
}



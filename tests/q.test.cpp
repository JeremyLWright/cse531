/*******************************************************************************
 * FILENAME:    q.test.cpp
 * DESCRIPTION: unit tests for user-space threads and queue
 * AUTHOR:      Jeremy Wright, Matt Welch
 * SCHOOL:      Arizona State University
 * CLASS:       CSE531: Dstributed and Multiprocessor Operating Systems
 * INSTRUCTOR:  Dr. Partha Dasgupta
 * TERM:        Fall 2014
 *******************************************************************************/
extern "C" {
#include "threads/q.h"
}


#include <gtest/gtest.h>
#include <deque>
#include <functional>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>
#include <iterator>
#if 0
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
#endif
class Directed : public ::testing::Test 
{
    protected:
        virtual void SetUp()
        {
            InitQ(&q);
        }

        virtual void TearDown()
        {
            //FreeQ(&q);
        }

        Q q;
};

#if 0
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
#endif

TEST_F(Directed, Add)
{
    for(int i = 1; i < 100; ++i)
    {
        test_item_t t;
        t.data = i;
        AddQ(&q, &t);
        ASSERT_EQ(i, size_(&q));
    }
    ASSERT_EQ(99, size_(&q));
    for(int i = 1; i < 100; ++i)
    {
        DelQ(&q);
    }
    ASSERT_EQ(0, size_(&q));
}

TEST_F(Directed, PutGet)
{
    int i = 8;
    test_item_t t;
    t.data = i;
    AddQ(&q, &t);
    ASSERT_EQ(8, DelQ(&q)->data);
    ASSERT_EQ(0, size_(&q));
}

TEST_F(Directed, DeleteEmpty)
{
    InitQ(&q);
    DelQ(&q);
}
TEST_F(Directed, RotateEmpty)
{
    InitQ(&q);
    RotateQ(&q);
}
TEST_F(Directed, RotateOne)
{
    test_item_t t;
    t.data = 9;

    AddQ(&q, &t);
    for(int i = 0; i < 100; ++i)
        ASSERT_EQ(9, RotateQ(&q)->data);
}
TEST_F(Directed, RotateTwo)
{
    InitQ(&q);

    test_item_t t1;
    t1.data = 0;
    AddQ(&q, &t1);

    test_item_t t2;
    t2.data = 1;
    AddQ(&q, &t2);

    for(int i = 0; i < 100000; ++i)
        ASSERT_EQ(i%2, RotateQ(&q)->data);
}
# if 0
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
#endif

TEST_F(Directed, Rotate)
{
    size_t const test_size = 4;
    test_item_t data[test_size];

    for(int i = 0; i < (int)test_size; ++i)
    {
        data[i].data = i;
        AddQ(&q, &data[i]);
    }

    for(int i = 0; i < 1000; ++i)
    {
        ASSERT_EQ(3, PeekQ(&q)->data);
        ASSERT_EQ(0, RotateQ(&q)->data);
        ASSERT_EQ(0, PeekQ(&q)->data);
        ASSERT_EQ(1, RotateQ(&q)->data);
        ASSERT_EQ(1, PeekQ(&q)->data);
        ASSERT_EQ(2, RotateQ(&q)->data);
        ASSERT_EQ(2, PeekQ(&q)->data);
        ASSERT_EQ(3, RotateQ(&q)->data);
        ASSERT_EQ(3, PeekQ(&q)->data);
    }
}
TEST_F(Directed, AddDel)
{
    size_t const test_size = 4;
    test_item_t data[test_size];

    for(int i = 0; i < (int)test_size; ++i)
    {
        data[i].data = i;
        AddQ(&q, &data[i]);
    }

    for(int i = 0; i < 1000; ++i)
    {
        ASSERT_EQ(3, PeekQ(&q)->data);
        ASSERT_EQ(0, RotateQ(&q)->data);
        ASSERT_EQ(0, PeekQ(&q)->data);
        ASSERT_EQ(1, RotateQ(&q)->data);
        ASSERT_EQ(1, PeekQ(&q)->data);
        ASSERT_EQ(2, RotateQ(&q)->data);
        ASSERT_EQ(2, PeekQ(&q)->data);
        ASSERT_EQ(3, RotateQ(&q)->data);
        ASSERT_EQ(3, PeekQ(&q)->data);
    }
}



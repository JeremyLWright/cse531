extern "C" {
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

    for(auto n = 0; n < ninserts; ++n)
    {
        auto d = dist(entropy);
        model.push_back(d);
        AddQ(&q, &d);
    }
    for(auto n = 0; n < npops; ++n)
    {
        model.pop_front();
        DelQ(&q);
    }
    for(auto n = 0; n < nreinserts; ++n)
    {
        auto d = dist(entropy);
        model.push_back(d);
        AddQ(&q, &d);
    }
    
    for(auto r : model)
    {
        auto u = *DelQ(&q);
        //std::cout << "Actual: " << r << " Expected: " << u <<'\n';

        if(r != u)
        {
            result = false;
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

TEST_F(Model, TheIDontHaveAllFuckingDaySoMakeTheComputerDoMyHomeworkTest)
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
    ASSERT_TRUE(run_isolated_test(10240, 0, 0)); //10 straight inserts don't fail, but 1 and 9 do.
}
TEST_F(Directed, RandomMinimizedTest2)
{
    ASSERT_TRUE(run_isolated_test(1, 1, 8));
}
TEST_F(Directed, RandomMinimizedTest3)
{
    ASSERT_TRUE(run_isolated_test(1, 1, 9));
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
    Q q;
    InitQ(&q);

    DelQ(&q);
}



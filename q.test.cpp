extern "C" {
#include "q.h"
}


#include <gtest/gtest.h>
#include <queue>
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
            InitQ(&q);
            auto seed = std::chrono::system_clock::now().time_since_epoch().count();
            rng.seed(seed);
        }

        virtual void TearDown()
        {
            FreeQ(&q);
        }

        Q q;
        std::queue<int> model;
        std::mt19937 rng;
};

TEST_F(Model, TheIDontHaveAllFuckingDaySoMakeTheComputerDoMyHomeworkTest)
{
    std::uniform_int_distribution<int> datum(
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::max());
    std::uniform_int_distribution<int> ntests(0, 1000);

    //Generate a random number of inserts
    auto const ninserts = ntests(rng);

    for(auto n = 0; n < ninserts; ++n)
    {
        auto r = ntests(rng);
        model.push(r);
        AddQ(&q, &r);
    }

    //Generate a random number of reads/pops
    auto const npops = ntests(rng);
    for(auto n = 0; n < npops; ++n)
    {
        model.pop();
        DelQ(&q);
    }
    
    //Generate a random number of more inserts to verify circular
    //dynamic functionality.  Verify we can write, read, write and read
    //again in any amount.
    auto const nreinserts = ntests(rng);
    for(auto n = 0; n < nreinserts; ++n)
    {
        auto r = datum(rng);
        model.push(r);
        AddQ(&q, &r);
    }

    //Verify from the model
    for(auto r = model.front(); r != model.back(); ++r)
    {
        if(r != *DelQ(&q))
        {
            std::cout << "Test Failed:\n"
                << "\tInserts: " << ninserts << '\n'
                << "\tDeletes: " << npops << '\n'
                << "\tReinserts: " << nreinserts << '\n';
            FAIL();
        }
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



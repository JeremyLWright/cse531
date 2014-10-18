#pragma once
#include <gtest/gtest.h>
#include <deque>
#include <functional>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>
#include <iterator>

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

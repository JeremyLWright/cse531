extern "C" {
#include "q.h"
}


#include <gtest/gtest.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <random>
#include <chrono>
#include <limits>

std::vector<int> model;

template <typename T, typename L>
void minimize(T& api, L& log)
{
    std::cerr << "Failing Test Case: \n";
//    for(auto& f: api)
//    {
//        //std::cerr << "Call: " << f.first << " input=" << log[0]  << '\n';
//    }
    FAIL();
}

template <typename T, typename Fn>
void run_model(T& api, Fn verifier)
{
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(),std::numeric_limits<int>::max());
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);
    std::shuffle (std::begin(api), std::end(api), rng);
    std::vector<int> input_log;
    for(auto& f : api)
    {
        auto i = dist(rng);
        input_log.push_back(i);
        f.first(i);
        f.second(i);
        if(!verifier()) {minimize(api, input_log);}
    }
}

TEST(ModelBased, QueueSize)
{
    Q q;
    InitQ(&q);

    std::pair<std::function<void(int)>, std::function<void(int)>> uut_api_add;

    uut_api_add.first = [&](int i){ model.push_back(i); };
    uut_api_add.second = [&](int i){ Item t; t.t = i;  AddQ(&q, &t); };

    std::pair<std::function<void(int)>, std::function<void(int)>> uut_api_remove;

    uut_api_remove.first = [&](int i){model.push_back(i); };
    uut_api_remove.second = [&](int i){ Item t; t.t = i; AddQ(&q, &t); };

    auto verifier = [&](){ return model.size() == _size(&q);};


    std::vector<std::pair<std::function<void(int)>, std::function<void(int)>>> u{uut_api_add, uut_api_remove};
    run_model(u, verifier);
}

TEST(Directed, Add)
{
    Q q;
    InitQ(&q);
    for(int i = 0; i < 100; ++i)
    {
        Item* t = new Item(); //I hate C interfaces!
        t->t = i;
        AddQ(&q, t);
    }
    ASSERT_EQ(100, _size(&q));
    for(int i = 0; i < 100; ++i)
    {
        ASSERT_EQ(DelQ(&q)->t, i);
    }
    ASSERT_EQ(0, _size(&q));
}

TEST(Directed, DeleteEmpty)
{
    Q q;
    InitQ(&q);

    DelQ(&q);
}



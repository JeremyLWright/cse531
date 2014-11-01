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

typedef struct _test_item_t {
    int data;
    struct _test_item_t* next;
    struct _test_item_t* prev;
} test_item_t;
typedef test_item_t list_value_type;
#define LIST_PARAM

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
#include "Directed.hpp"
#include "utils.hpp"
#include "tests/Model.hpp"
#include <ostream>
#include <istream>
#include <iostream>

std::ostream& operator<<(std::ostream& o, Q q)
{
#if YOU_LOVE_CPP_AND_WANT_CODE_TO_WORK
    if(q.nil->next == q.nil)
        o << "[]";

    for(list_value_type* i = q.nil->next; i != q.nil; i = i->next)
    {
        o << "<[ ";
        o << i->data;
        if (q.curr == i) o << " (c)";
        o << " ]> ";
    }
    o << '\n';
#else //i.e. you don't want things to work
    char* str = new char[4096];
    Print(str, &q);
    o << str;
    delete [] str;
#endif

    return o;
}

TEST_F(Directed, Add)
{
    repeat_n(100, [&](size_t i)
            {
            test_item_t* t = new test_item_t;
            t->data = i;
            AddQ(&q, t);
            ASSERT_EQ(i+1, size_(&q));
            });

    ASSERT_EQ(100, size_(&q));
    repeat_n(100, [&](size_t i){ DelQ(&q); });
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
TEST_F(Directed, CreateBadCondition)
{
    InitQ(&q);
    q.head = (list_value_type*)1;
    RotateQ(&q);
}
TEST_F(Directed, RotateOne)
{
    test_item_t t;
    t.data = 9;

    AddQ(&q, &t);
    repeat_n(100, [&](size_t i){ ASSERT_EQ(9, RotateQ(&q)->data); });
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

    repeat_n(100000, [&](size_t i){ ASSERT_EQ((i+1)%2, RotateQ(&q)->data); });
}

TEST_F(Directed, Rotate)
{
    size_t const test_size = 4;
    test_item_t data[test_size];

    for(size_t i = 0; i < test_size; ++i)
    {
        data[i].data = i;
        AddQ(&q, data + i);
    }

    std::cout << q << '\n';

    for(int i = 0; i < 2; ++i)
    {
        ASSERT_EQ(0, PeekQ(&q)->data);
        ASSERT_EQ(3, RotateQ(&q)->data);
    std::cout << q << '\n';
        ASSERT_EQ(2, RotateQ(&q)->data);
    std::cout << q << '\n';
        ASSERT_EQ(1, RotateQ(&q)->data);
    std::cout << q << '\n';
        ASSERT_EQ(0, RotateQ(&q)->data);
    std::cout << q << '\n';
    }
}

TEST_F(Model, AddDel)
{
    test_item_t data[test_size];

    repeat_n(test_size, [&](size_t i)
    {
        data[i].data = i;
        AddQ(&q, &data[i]);
    });

    for(size_t i = test_size; i > test_size; --i)
    {
        ASSERT_EQ(i, DelQ(&q)->data);
    }
}

TEST_F(Directed, TwoQueues)
{
    Q RunQ;
    InitQ(&RunQ);
    //Q SemQ;
    list_value_type items[4];
    items[0].data = 1;
    items[1].data = 2;
    items[2].data = 3;
    items[3].data = 4;
    AddQ(&RunQ, items);
    AddQ(&RunQ, items+1);
    AddQ(&RunQ, items+2);
    AddQ(&RunQ, items+3);
    std::cout << RunQ << '\n';
    

}

TEST_F(Directed, AddDoesntMoveCurrent)
{
    Q RunQ;
    InitQ(&RunQ);
    list_value_type items[4];
    items[0].data = 1;
    items[1].data = 2;
    items[2].data = 3;
    items[3].data = 4;
    for(int i = 0; i < 4; ++i) AddQ(&RunQ, items + i);

    auto old_current = RunQ.curr;

    AddQ(&RunQ, items+1);
    ASSERT_TRUE(old_current == RunQ.curr);
}

TEST_F(Directed, DelMovesCurrentToNext)
{
    Q RunQ;
    InitQ(&RunQ);
    list_value_type items[4];
    items[0].data = 1;
    items[1].data = 2;
    items[2].data = 3;
    items[3].data = 4;
    for(int i = 0; i < 4; ++i) AddQ(&RunQ, items + i);

    std::cout << RunQ << '\n';
    ASSERT_EQ(1, RunQ.curr->data);
    DelQ(&RunQ);
   
    std::cout << RunQ << '\n';
    ASSERT_EQ(4,  RunQ.curr->data);
    DelQ(&RunQ);

    std::cout << RunQ << '\n';
    ASSERT_EQ(3,  RunQ.curr->data);
    DelQ(&RunQ);

    std::cout << RunQ << '\n';
    ASSERT_EQ(2,  RunQ.curr->data);
    DelQ(&RunQ);
    
    std::cout << RunQ << '\n';
    ASSERT_EQ(2,  RunQ.curr->data);
    DelQ(&RunQ); //Shouldn't crash
}



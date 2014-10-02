extern "C" {
#include "q.h"
}


#include <gtest/gtest.h>
#include <vector>

std::vector<int> model;

TEST(ModelBased, QueueSize)
{
    Q q;
    InitQ(&q);

    model.push_back(0);
    AddQ(&q, 0);

    ASSERT_EQ(model.size(), _size(&q));
}



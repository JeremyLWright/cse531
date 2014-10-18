#pragma once

class Directed : public ::testing::Test 
{
    protected:
        virtual void SetUp()
        {
            InitQ(&q);
        }

        virtual void TearDown()
        {
        }

        Q q;
};

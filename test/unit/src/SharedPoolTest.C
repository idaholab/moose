//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "SharedPool.h"

#include <algorithm> // std::set_symmetric_difference

namespace
{
static int num_constructed = 0;
static int num_resetable_constructed = 0;
}

class PoolDummy
{
public:
  PoolDummy() { num_constructed++; }

  PoolDummy(int dumb) : _dumb(dumb) { num_constructed++; }

  int _dumb;
};

class ResetablePoolDummy
{
public:
  ResetablePoolDummy() { num_resetable_constructed++; }

  ResetablePoolDummy(int dumb) : _dumb(dumb) { num_resetable_constructed++; }

  void reset(int dumb) { _dumb = dumb; }

  int _dumb;
};

TEST(SharedPool, test)
{
  {
    MooseUtils::SharedPool<PoolDummy> pool;

    {
      std::shared_ptr<PoolDummy> dumb = pool.acquire();
      EXPECT_EQ(num_constructed, 1);

      std::shared_ptr<PoolDummy> dumb2 = pool.acquire(2);
      EXPECT_EQ(num_constructed, 2);
      EXPECT_EQ(dumb2->_dumb, 2);
    }

    EXPECT_EQ(pool.size(), 2);
    EXPECT_EQ(pool.num_created(), 2);

    {
      std::shared_ptr<PoolDummy> dumb = pool.acquire();
      EXPECT_EQ(num_constructed, 2);

      std::shared_ptr<PoolDummy> dumb2 = pool.acquire(2);
      EXPECT_EQ(num_constructed, 2);

      EXPECT_EQ(pool.num_created(), 2);

      std::shared_ptr<PoolDummy> dumb3 = pool.acquire(2);
      EXPECT_EQ(num_constructed, 3);

      EXPECT_EQ(pool.num_created(), 3);
    }

    EXPECT_EQ(pool.size(), 3);
    EXPECT_EQ(pool.num_created(), 3);
  }

  {
    MooseUtils::SharedPool<ResetablePoolDummy> pool;

    {
      std::shared_ptr<ResetablePoolDummy> dumb = pool.acquire(1);
      EXPECT_EQ(num_resetable_constructed, 1);
      EXPECT_EQ(dumb->_dumb, 1);

      std::shared_ptr<ResetablePoolDummy> dumb2 = pool.acquire(2);
      EXPECT_EQ(num_resetable_constructed, 2);
      EXPECT_EQ(dumb2->_dumb, 2);
    }

    EXPECT_EQ(pool.size(), 2);

    {
      std::shared_ptr<ResetablePoolDummy> dumb = pool.acquire(3);
      EXPECT_EQ(num_resetable_constructed, 2);
      EXPECT_EQ(dumb->_dumb, 3);

      std::shared_ptr<ResetablePoolDummy> dumb2 = pool.acquire(4);
      EXPECT_EQ(num_resetable_constructed, 2);
      EXPECT_EQ(dumb2->_dumb, 4);

      std::shared_ptr<ResetablePoolDummy> dumb3 = pool.acquire(5);
      EXPECT_EQ(num_resetable_constructed, 3);
      EXPECT_EQ(dumb3->_dumb, 5);
    }

    EXPECT_EQ(pool.size(), 3);
  }
}

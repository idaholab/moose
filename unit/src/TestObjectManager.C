#include <algorithm>
#include <memory>

#include "gtest/gtest.h"
#include "ObjectManager.h"

#include "mfem.hpp"

class CheckObjectManager : public testing::Test
{
protected:
  platypus::ObjectManager<mfem::Coefficient> manager;
  std::shared_ptr<mfem::ConstantCoefficient> c1, c2, c3;

  CheckObjectManager()
  {
    c1 = manager.make<mfem::ConstantCoefficient>(1.);
    c2 = manager.make<mfem::ConstantCoefficient>(2.);
    c3 = manager.make<mfem::ConstantCoefficient>(3.);
  }
};

TEST_F(CheckObjectManager, Iter)
{
  int i = -1;
  for (auto coef : manager)
  {
    auto coef_const = std::dynamic_pointer_cast<mfem::ConstantCoefficient>(coef);
    ASSERT_NE(coef_const.get(), nullptr);
    coef_const->constant = i--;
  }
  EXPECT_EQ(c1->constant, -1);
  EXPECT_EQ(c2->constant, -2);
  EXPECT_EQ(c3->constant, -3);
}

TEST_F(CheckObjectManager, IterMutable)
{
  int i = -1;
  for (auto & coef : manager)
  {
    EXPECT_EQ(coef.use_count(), 2);
    coef.reset();
  }
  EXPECT_EQ(c1.use_count(), 1);
  EXPECT_EQ(c2.use_count(), 1);
  EXPECT_EQ(c3.use_count(), 1);
}

TEST_F(CheckObjectManager, ConstIter)
{
  int i = -1;
  for (auto it = manager.cbegin(); it != manager.cend(); ++it)
  {
    auto coef_const = std::dynamic_pointer_cast<mfem::ConstantCoefficient>(*it);
    ASSERT_NE(coef_const.get(), nullptr);
    coef_const->constant = i--;
  }
  EXPECT_EQ(c1->constant, -1);
  EXPECT_EQ(c2->constant, -2);
  EXPECT_EQ(c3->constant, -3);
}

TEST_F(CheckObjectManager, ReverseIter)
{
  int i = -1;
  for (auto it = manager.rbegin(); it != manager.rend(); ++it)
  {
    auto coef_const = std::dynamic_pointer_cast<mfem::ConstantCoefficient>(*it);
    ASSERT_NE(coef_const.get(), nullptr);
    coef_const->constant = i--;
  }
  EXPECT_EQ(c1->constant, -3);
  EXPECT_EQ(c2->constant, -2);
  EXPECT_EQ(c3->constant, -1);
}

TEST_F(CheckObjectManager, ReverseIterMutable)
{
  int i = -1;
  for (auto it = manager.rbegin(); it != manager.rend(); ++it)
  {
    EXPECT_EQ(it->use_count(), 2);
    it->reset();
  }
  EXPECT_EQ(c1.use_count(), 1);
  EXPECT_EQ(c2.use_count(), 1);
  EXPECT_EQ(c3.use_count(), 1);
}

TEST_F(CheckObjectManager, ConstReverseIter)
{
  int i = -1;
  for (auto it = manager.crbegin(); it != manager.crend(); ++it)
  {
    auto coef_const = std::dynamic_pointer_cast<mfem::ConstantCoefficient>(*it);
    ASSERT_NE(coef_const.get(), nullptr);
    coef_const->constant = i--;
  }
  EXPECT_EQ(c1->constant, -3);
  EXPECT_EQ(c2->constant, -2);
  EXPECT_EQ(c3->constant, -1);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DependencyResolverTest.h"

TEST_F(DependencyResolverTest, operatorParensTest)
{
  std::vector<std::string> unsorted(6);
  unsorted[0] = "c";
  unsorted[1] = "f";
  unsorted[2] = "a";
  unsorted[3] = "d";
  unsorted[4] = "b";
  unsorted[5] = "e";

  // Sort based on the dependency resolver
  // "bead" will come out after the independent items
  std::sort(unsorted.begin(), unsorted.end(), _strict_ordering);

  EXPECT_EQ(unsorted[2], "b");
  EXPECT_EQ(unsorted[3], "e");
  EXPECT_EQ(unsorted[4], "a");
  EXPECT_EQ(unsorted[5], "d");
}

TEST_F(DependencyResolverTest, ptrTest)
{
  DependencyResolver<int *> resolver;

  int mat3;
  int mat1;
  int mat2;

  resolver.insertDependency(&mat2, &mat1);
  resolver.insertDependency(&mat3, &mat1);
  resolver.insertDependency(&mat3, &mat2);

  std::vector<int *> sorted(3);
  sorted[0] = &mat1;
  sorted[1] = &mat2;
  sorted[2] = &mat3;

  /*const std::vector<std::set<int *> > & sets =*/
  resolver.getSortedValuesSets();

  std::sort(sorted.begin(), sorted.end(), resolver);
  EXPECT_EQ(sorted[0], &mat1);
  EXPECT_EQ(sorted[1], &mat2);
  EXPECT_EQ(sorted[2], &mat3);
}

TEST_F(DependencyResolverTest, simpleTest)
{
  DependencyResolver<int> resolver;

  int mat3 = 3;
  int mat1 = 1;
  int mat2 = 2;

  resolver.insertDependency(mat2, mat1);
  resolver.insertDependency(mat3, mat1);
  resolver.insertDependency(mat3, mat2);

  std::vector<int> sorted(3);
  sorted[0] = mat1;
  sorted[1] = mat2;
  sorted[2] = mat3;

  /*const std::vector<std::set<int> > & sets =*/
  resolver.getSortedValuesSets();

  std::sort(sorted.begin(), sorted.end(), resolver);
  EXPECT_EQ(sorted[0], mat1);
  EXPECT_EQ(sorted[1], mat2);
  EXPECT_EQ(sorted[2], mat3);
}

TEST_F(DependencyResolverTest, resolverSets)
{
  // First throw in an extra independent item
  _resolver.addItem("aa");

  const auto & sets = _resolver.getSortedValuesSets();

  EXPECT_EQ(sets.size(), 3);
  EXPECT_EQ(sets[0].size(), 2);
  EXPECT_NE(std::find(sets[0].begin(), sets[0].end(), "a"), sets[0].end());
  EXPECT_NE(std::find(sets[0].begin(), sets[0].end(), "aa"), sets[0].end());

  EXPECT_EQ(sets[1].size(), 2);
  EXPECT_NE(std::find(sets[1].begin(), sets[1].end(), "b"), sets[1].end());
  EXPECT_NE(std::find(sets[1].begin(), sets[1].end(), "c"), sets[1].end());

  EXPECT_EQ(sets[2].size(), 1);
  EXPECT_NE(std::find(sets[2].begin(), sets[2].end(), "d"), sets[2].end());
}

TEST_F(DependencyResolverTest, dependsOnTest)
{
  EXPECT_TRUE(_resolver.dependsOn("b", "a"));
  EXPECT_TRUE(_resolver.dependsOn("c", "a"));
  EXPECT_TRUE(_resolver.dependsOn("d", "c"));
  EXPECT_TRUE(_resolver.dependsOn("d", "a"));
  EXPECT_TRUE(_resolver.dependsOn("b", "b"));
  EXPECT_TRUE(_resolver.dependsOn("a", "a"));
  EXPECT_FALSE(_resolver.dependsOn("b", "c"));
  EXPECT_FALSE(_resolver.dependsOn("b", "d"));
  EXPECT_TRUE(_tree.dependsOn("k0", "m0"));
  EXPECT_TRUE(_tree.dependsOn("k0", "m1"));
  EXPECT_TRUE(_tree.dependsOn("k0", "m2"));
  EXPECT_TRUE(_tree.dependsOn("k0", "mA"));
  EXPECT_TRUE(_tree.dependsOn("k0", "mB"));
  EXPECT_TRUE(_tree.dependsOn("k0", "mC"));
  EXPECT_TRUE(_tree.dependsOn("k0", "mD"));
  EXPECT_FALSE(_tree.dependsOn("k1", "m0"));
  EXPECT_FALSE(_tree.dependsOn("k1", "m1"));
  EXPECT_TRUE(_tree.dependsOn("k1", "m2"));
  EXPECT_FALSE(_tree.dependsOn("k1", "mA"));
  EXPECT_FALSE(_tree.dependsOn("k1", "mB"));
  EXPECT_FALSE(_tree.dependsOn("k1", "mC"));
  EXPECT_TRUE(_tree.dependsOn("k1", "mD"));
  EXPECT_TRUE(_tree.dependsOn("m0", "m0"));
  EXPECT_FALSE(_tree.dependsOn("m0", "m1"));
  EXPECT_FALSE(_tree.dependsOn("m0", "m2"));
  EXPECT_TRUE(_tree.dependsOn("m0", "mA"));
  EXPECT_TRUE(_tree.dependsOn("m0", "mB"));
  EXPECT_TRUE(_tree.dependsOn("m0", "mC"));
  EXPECT_FALSE(_tree.dependsOn("m0", "mD"));
  EXPECT_FALSE(_tree.dependsOn("m0", "k0"));
  EXPECT_FALSE(_tree.dependsOn("m0", "k1"));
  EXPECT_FALSE(_tree.dependsOn("k1", "k0"));
  EXPECT_FALSE(_tree.dependsOn("k0", "k1"));
  EXPECT_FALSE(_tree.dependsOn("m1", "something_else"));
  EXPECT_FALSE(_tree.dependsOn("something_else", "k0"));
}

TEST_F(DependencyResolverTest, cyclicTest)
{
  try
  {
    // Attempt to insert a dependency that results in cyclicity
    _resolver.insertDependency("a", "b");
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("DependencyResolver: attempt to insert dependency will result in cyclic graph") !=
        std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

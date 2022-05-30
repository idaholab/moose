//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DependencyResolverTest.h"

#include <array>

TEST_F(DependencyResolverTest, operatorParensTest)
{
  // Sort based on the dependency resolver
  // "bead" will come out after the independent items
  const auto & unsorted = _strict_ordering.dfs();

  const auto b_loc =
      std::distance(unsorted.begin(), std::find(unsorted.begin(), unsorted.end(), "b"));
  const auto e_loc =
      std::distance(unsorted.begin(), std::find(unsorted.begin(), unsorted.end(), "e"));
  const auto a_loc =
      std::distance(unsorted.begin(), std::find(unsorted.begin(), unsorted.end(), "a"));
  const auto d_loc =
      std::distance(unsorted.begin(), std::find(unsorted.begin(), unsorted.end(), "d"));
  EXPECT_LT(b_loc, e_loc);
  EXPECT_LT(e_loc, a_loc);
  EXPECT_LT(a_loc, d_loc);
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

  const auto & sorted = resolver.dfs();

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

  const auto & sorted = resolver.dfs();

  EXPECT_EQ(sorted[0], mat1);
  EXPECT_EQ(sorted[1], mat2);
  EXPECT_EQ(sorted[2], mat3);
}

TEST_F(DependencyResolverTest, resolverSets)
{
  // First throw in an extra independent item
  _resolver.addItem("aa");

  const auto & sets = _resolver.getSortedValuesSets();
  // Flatten
  std::vector<std::string> flat;
  for (const auto & set : sets)
    flat.insert(flat.end(), set.begin(), set.end());

  const auto a_loc = std::distance(flat.begin(), std::find(flat.begin(), flat.end(), "a"));
  const auto b_loc = std::distance(flat.begin(), std::find(flat.begin(), flat.end(), "b"));
  const auto c_loc = std::distance(flat.begin(), std::find(flat.begin(), flat.end(), "c"));
  const auto d_loc = std::distance(flat.begin(), std::find(flat.begin(), flat.end(), "d"));
  EXPECT_LT(a_loc, b_loc);
  EXPECT_LT(a_loc, c_loc);
  EXPECT_LT(c_loc, d_loc);
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

TEST_F(DependencyResolverTest, deleteDepTest)
{
  DependencyResolver<int> resolver;

  int mat4 = 4;
  int mat3 = 3;
  int mat1 = 1;
  int mat2 = 2;

  resolver.insertDependency(mat2, mat1);
  resolver.insertDependency(mat3, mat2);
  resolver.insertDependency(mat4, mat3);

  const auto & sorted = resolver.dfs();

  EXPECT_EQ(sorted[0], mat1);
  EXPECT_EQ(sorted[1], mat2);
  EXPECT_EQ(sorted[2], mat3);
  EXPECT_EQ(sorted[3], mat4);

  // Now switch around dependencies and check the order again
  resolver.deleteDependency(mat3, mat2);
  resolver.insertDependency(mat1, mat4);

  resolver.dfs();

  EXPECT_EQ(sorted[0], mat3);
  EXPECT_EQ(sorted[1], mat4);
  EXPECT_EQ(sorted[2], mat1);
  EXPECT_EQ(sorted[3], mat2);
}

TEST_F(DependencyResolverTest, deleteDepIndCheckTest)
{
  DependencyResolver<int> resolver;

  resolver.insertDependency(4, 3);
  resolver.insertDependency(3, 2);
  resolver.insertDependency(2, 1);

  resolver.deleteDependency(4, 3);

  const auto & items = resolver.dfs();
  const auto one_loc = std::distance(items.begin(), std::find(items.begin(), items.end(), 1));
  const auto two_loc = std::distance(items.begin(), std::find(items.begin(), items.end(), 2));
  const auto three_loc = std::distance(items.begin(), std::find(items.begin(), items.end(), 3));
  EXPECT_LT(one_loc, two_loc);
  EXPECT_LT(two_loc, three_loc);
}

TEST_F(DependencyResolverTest, deleteDepsCheck)
{
  DependencyResolver<int> resolver;

  resolver.insertDependency(4, 3);
  resolver.insertDependency(3, 2);
  resolver.insertDependency(2, 1);

  // Don't need to know what 4 depends on, but we might just lose 4 now (unless it's part of a
  // subtree
  resolver.deleteDependenciesOfKey(4);

  /** Move 4 to a different place */
  resolver.insertDependency(4, 1);
  resolver.insertDependency(2, 4);

  const auto & items = resolver.getSortedValues();
  EXPECT_EQ(items[0], 1);
  EXPECT_EQ(items[1], 4);
  EXPECT_EQ(items[2], 2);
  EXPECT_EQ(items[3], 3);
}

TEST_F(DependencyResolverTest, cyclicTest)
{
  try
  {
    // Attempt to insert a dependency that results in cyclicity
    _resolver.insertDependency("a", "b");
    _resolver.dfs();
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("cyclic graph") != std::string::npos);
    _resolver.removeEdge("b", "a");
  }
}

TEST_F(DependencyResolverTest, getValuesTest)
{
  auto values = _tree.getAncestors("k0");

  values.sort();

  auto it = values.begin();

  EXPECT_EQ(*it++, "k0");
  EXPECT_EQ(*it++, "m0");
  EXPECT_EQ(*it++, "m1");
  EXPECT_EQ(*it++, "m2");
  EXPECT_EQ(*it++, "mA");
  EXPECT_EQ(*it++, "mB");
  EXPECT_EQ(*it++, "mC");
  EXPECT_EQ(*it++, "mD");
}

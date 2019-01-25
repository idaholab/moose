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

  std::array<int, 4> sorted;
  sorted[0] = mat1;
  sorted[1] = mat2;
  sorted[2] = mat3;
  sorted[3] = mat4;

  /*const std::vector<std::set<int> > & sets =*/
  resolver.getSortedValuesSets();

  std::sort(sorted.begin(), sorted.end(), resolver);
  EXPECT_EQ(sorted[0], mat1);
  EXPECT_EQ(sorted[1], mat2);
  EXPECT_EQ(sorted[2], mat3);
  EXPECT_EQ(sorted[3], mat4);

  // Now switch around dependencies and check the order again
  resolver.deleteDependency(mat3, mat2);
  resolver.insertDependency(mat1, mat4);

  resolver.getSortedValuesSets();

  std::sort(sorted.begin(), sorted.end(), resolver);
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

  // By removing the edge between 4 and 3, we leave 4 as an independent item, meaning it'll
  // come out before 3 which has no dependencies but is part of the remaining graph.
  const auto & items = resolver.getSortedValues();
  EXPECT_EQ(items[0], 4);
  EXPECT_EQ(items[1], 1);
  EXPECT_EQ(items[2], 2);
  EXPECT_EQ(items[3], 3);
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

TEST_F(DependencyResolverTest, getValuesTest)
{
  const std::vector<std::string> & values = _tree.getValues("k0");

  // Make a sorted copy to compare against
  std::vector<std::string> copy;
  for (auto val : values)
    copy.push_back(val);

  std::sort(copy.begin(), copy.end());

  EXPECT_EQ(copy[0], "m0");
  EXPECT_EQ(copy[1], "m1");
  EXPECT_EQ(copy[2], "m2");
}

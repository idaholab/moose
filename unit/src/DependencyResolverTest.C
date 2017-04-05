/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "DependencyResolverTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(DependencyResolverTest);

void
DependencyResolverTest::setUp()
{
  _resolver.insertDependency("b", "a");
  _resolver.insertDependency("c", "a");
  _resolver.insertDependency("d", "c");

  // There are no items in the same "set" so the ordering will be strict
  _strict_ordering.insertDependency("e", "b");
  _strict_ordering.insertDependency("a", "e");
  _strict_ordering.insertDependency("d", "a");
  // Ordering will be "bead"

  /**
   *          k0     k1
   *        / |  \  /
   *      m0  m1  m2
   *     / \   |\  |
   *    mA  mB | \ |
   *          \|  mD
   *           mC
   */
  _tree.insertDependency("k0", "m0");
  _tree.insertDependency("k0", "m1");
  _tree.insertDependency("k0", "m2");
  _tree.insertDependency("k1", "m2");
  _tree.insertDependency("m0", "mA");
  _tree.insertDependency("m0", "mB");
  _tree.insertDependency("m1", "mC");
  _tree.insertDependency("m1", "mD");
  _tree.insertDependency("mB", "mC");
  _tree.insertDependency("m1", "mD");
  _tree.insertDependency("m2", "mD");
}

void
DependencyResolverTest::operatorParensTest()
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

  CPPUNIT_ASSERT(unsorted[2] == "b");
  CPPUNIT_ASSERT(unsorted[3] == "e");
  CPPUNIT_ASSERT(unsorted[4] == "a");
  CPPUNIT_ASSERT(unsorted[5] == "d");
}

void
DependencyResolverTest::ptrTest()
{
  DependencyResolver<int *> resolver;

  int * mat3 = new int;
  int * mat1 = new int;
  int * mat2 = new int;

  resolver.insertDependency(mat2, mat1);
  resolver.insertDependency(mat3, mat1);
  resolver.insertDependency(mat3, mat2);

  std::vector<int *> sorted(3);
  sorted[0] = mat1;
  sorted[1] = mat2;
  sorted[2] = mat3;

  /*const std::vector<std::set<int *> > & sets =*/
  resolver.getSortedValuesSets();

  std::sort(sorted.begin(), sorted.end(), resolver);
  CPPUNIT_ASSERT(sorted[0] == mat1);
  CPPUNIT_ASSERT(sorted[1] == mat2);
  CPPUNIT_ASSERT(sorted[2] == mat3);

  delete mat1;
  delete mat2;
  delete mat3;
}

void
DependencyResolverTest::simpleTest()
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
  CPPUNIT_ASSERT(sorted[0] == mat1);
  CPPUNIT_ASSERT(sorted[1] == mat2);
  CPPUNIT_ASSERT(sorted[2] == mat3);
}

void
DependencyResolverTest::resolverSets()
{
  // First throw in an extra independent item
  _resolver.addItem("aa");

  const auto & sets = _resolver.getSortedValuesSets();

  CPPUNIT_ASSERT(sets.size() == 3);
  CPPUNIT_ASSERT(sets[0].size() == 2);
  CPPUNIT_ASSERT(std::find(sets[0].begin(), sets[0].end(), "a") != sets[0].end());
  CPPUNIT_ASSERT(std::find(sets[0].begin(), sets[0].end(), "aa") != sets[0].end());

  CPPUNIT_ASSERT(sets[1].size() == 2);
  CPPUNIT_ASSERT(std::find(sets[1].begin(), sets[1].end(), "b") != sets[1].end());
  CPPUNIT_ASSERT(std::find(sets[1].begin(), sets[1].end(), "c") != sets[1].end());

  CPPUNIT_ASSERT(sets[2].size() == 1);
  CPPUNIT_ASSERT(std::find(sets[2].begin(), sets[2].end(), "d") != sets[2].end());
}

void
DependencyResolverTest::dependsOnTest()
{
  CPPUNIT_ASSERT(_resolver.dependsOn("b", "a"));
  CPPUNIT_ASSERT(_resolver.dependsOn("c", "a"));
  CPPUNIT_ASSERT(_resolver.dependsOn("d", "c"));
  CPPUNIT_ASSERT(_resolver.dependsOn("d", "a"));
  CPPUNIT_ASSERT(_resolver.dependsOn("b", "b"));
  CPPUNIT_ASSERT(_resolver.dependsOn("a", "a"));
  CPPUNIT_ASSERT(!_resolver.dependsOn("b", "c"));
  CPPUNIT_ASSERT(!_resolver.dependsOn("b", "d"));
  CPPUNIT_ASSERT(_tree.dependsOn("k0", "m0"));
  CPPUNIT_ASSERT(_tree.dependsOn("k0", "m1"));
  CPPUNIT_ASSERT(_tree.dependsOn("k0", "m2"));
  CPPUNIT_ASSERT(_tree.dependsOn("k0", "mA"));
  CPPUNIT_ASSERT(_tree.dependsOn("k0", "mB"));
  CPPUNIT_ASSERT(_tree.dependsOn("k0", "mC"));
  CPPUNIT_ASSERT(_tree.dependsOn("k0", "mD"));
  CPPUNIT_ASSERT(!_tree.dependsOn("k1", "m0"));
  CPPUNIT_ASSERT(!_tree.dependsOn("k1", "m1"));
  CPPUNIT_ASSERT(_tree.dependsOn("k1", "m2"));
  CPPUNIT_ASSERT(!_tree.dependsOn("k1", "mA"));
  CPPUNIT_ASSERT(!_tree.dependsOn("k1", "mB"));
  CPPUNIT_ASSERT(!_tree.dependsOn("k1", "mC"));
  CPPUNIT_ASSERT(_tree.dependsOn("k1", "mD"));
  CPPUNIT_ASSERT(_tree.dependsOn("m0", "m0"));
  CPPUNIT_ASSERT(!_tree.dependsOn("m0", "m1"));
  CPPUNIT_ASSERT(!_tree.dependsOn("m0", "m2"));
  CPPUNIT_ASSERT(_tree.dependsOn("m0", "mA"));
  CPPUNIT_ASSERT(_tree.dependsOn("m0", "mB"));
  CPPUNIT_ASSERT(_tree.dependsOn("m0", "mC"));
  CPPUNIT_ASSERT(!_tree.dependsOn("m0", "mD"));
  CPPUNIT_ASSERT(!_tree.dependsOn("m0", "k0"));
  CPPUNIT_ASSERT(!_tree.dependsOn("m0", "k1"));
  CPPUNIT_ASSERT(!_tree.dependsOn("k1", "k0"));
  CPPUNIT_ASSERT(!_tree.dependsOn("k0", "k1"));
  CPPUNIT_ASSERT(!_tree.dependsOn("m1", "something_else"));
  CPPUNIT_ASSERT(!_tree.dependsOn("something_else", "k0"));
}

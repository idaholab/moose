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

CPPUNIT_TEST_SUITE_REGISTRATION( DependencyResolverTest );

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

  CPPUNIT_ASSERT( unsorted[2] == "b");
  CPPUNIT_ASSERT( unsorted[3] == "e");
  CPPUNIT_ASSERT( unsorted[4] == "a");
  CPPUNIT_ASSERT( unsorted[5] == "d");
}

void
DependencyResolverTest::ptrTest()
{
  DependencyResolver<int *> resolver;

  int *mat3 = new int;
  int *mat1 = new int;
  int *mat2 = new int;

  resolver.insertDependency(mat2, mat1);
  resolver.insertDependency(mat3, mat1);
  resolver.insertDependency(mat3, mat2);

  std::vector<int *> sorted(3);
  sorted[0] = mat1;
  sorted[1] = mat2;
  sorted[2] = mat3;


  const std::vector<std::set<int *> > & sets = resolver.getSortedValuesSets();

  std::sort(sorted.begin(), sorted.end(), resolver);
  CPPUNIT_ASSERT( sorted[0] == mat1);
  CPPUNIT_ASSERT( sorted[1] == mat2);
  CPPUNIT_ASSERT( sorted[2] == mat3);

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


  const std::vector<std::set<int> > & sets = resolver.getSortedValuesSets();

  std::sort(sorted.begin(), sorted.end(), resolver);
  CPPUNIT_ASSERT( sorted[0] == mat1);
  CPPUNIT_ASSERT( sorted[1] == mat2);
  CPPUNIT_ASSERT( sorted[2] == mat3);
}

void
DependencyResolverTest::resolverSets()
{
  // First throw in an extra independent item
  _resolver.addItem("aa");

  const std::vector<std::set<std::string> > & sets = _resolver.getSortedValuesSets();

  /*
  // DEBUG Printout
  for (std::vector<std::set<std::string> >::const_iterator i = sets.begin(); i != sets.end(); ++i)
  {
    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
      std::cout << *j << "\t";
    std::cout << "\n";
  }
  */

  CPPUNIT_ASSERT( sets.size() == 3 );
  CPPUNIT_ASSERT( sets[0].size() == 2);
  CPPUNIT_ASSERT( sets[0].find("a") != sets[0].end() );
  CPPUNIT_ASSERT( sets[0].find("aa") != sets[0].end() );

  CPPUNIT_ASSERT( sets[1].size() == 2);
  CPPUNIT_ASSERT( sets[0].find("b") != sets[1].end() );
  CPPUNIT_ASSERT( sets[0].find("c") != sets[1].end() );

  CPPUNIT_ASSERT( sets[2].size() == 1);
  CPPUNIT_ASSERT( sets[0].find("d") != sets[2].end() );
}

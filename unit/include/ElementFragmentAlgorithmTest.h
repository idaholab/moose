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

#ifndef ELEMENTFRAGMENTALGORITHMTEST_H
#define ELEMENTFRAGMENTALGORITHMTEST_H

// CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

// Moose includes
#include "ElementFragmentAlgorithm.h"

class ElementFragmentAlgorithmTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(ElementFragmentAlgorithmTest);

  CPPUNIT_TEST(ElementFragmentAlgorithmTest1a);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest1b);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest2a);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest2b);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest3);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest4);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest5a);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest5b);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest5c);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest6a);
  CPPUNIT_TEST(ElementFragmentAlgorithmTest6b);

  CPPUNIT_TEST_SUITE_END();

public:
  ElementFragmentAlgorithmTest();
  ~ElementFragmentAlgorithmTest();

  void case1Common(ElementFragmentAlgorithm & MyMesh);

  void ElementFragmentAlgorithmTest1a();
  void ElementFragmentAlgorithmTest1b();

  void case2Intersections(ElementFragmentAlgorithm & MyMesh);
  void case2Mesh(ElementFragmentAlgorithm & MyMesh);
  void ElementFragmentAlgorithmTest2a();
  void ElementFragmentAlgorithmTest2b();

  void ElementFragmentAlgorithmTest3();

  void ElementFragmentAlgorithmTest4();
  void case5Mesh(ElementFragmentAlgorithm & MyMesh);
  void ElementFragmentAlgorithmTest5a();
  void ElementFragmentAlgorithmTest5b();
  void ElementFragmentAlgorithmTest5c();

  void case6Mesh(ElementFragmentAlgorithm & MyMesh);
  void ElementFragmentAlgorithmTest6a();
  void ElementFragmentAlgorithmTest6b();

  void CheckNodes(std::map<unsigned int, EFANode *> & nodes, std::vector<unsigned int> & gold);
  void CheckElements(std::vector<EFAElement *> & elems, std::vector<unsigned int> & gold);
  void CheckElements(std::set<EFAElement *> & elems, std::set<unsigned int> & gold);
};

#endif // ELEMENTFRAGMENTALGORITHMTEST_H

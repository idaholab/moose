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

#ifndef CUTELEMMESHTEST_H
#define CUTELEMMESHTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

// Moose includes
#include "ElementFragmentAlgorithm.h"

class CutElemMeshTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE( CutElemMeshTest );

  CPPUNIT_TEST( CutElemMeshTest1a );
  CPPUNIT_TEST( CutElemMeshTest1b );
  CPPUNIT_TEST( CutElemMeshTest2a );
  CPPUNIT_TEST( CutElemMeshTest2b );
  CPPUNIT_TEST( CutElemMeshTest2c );
  CPPUNIT_TEST( CutElemMeshTest3 );
  CPPUNIT_TEST( CutElemMeshTest4 );
  CPPUNIT_TEST( CutElemMeshTest5a );
  CPPUNIT_TEST( CutElemMeshTest5b );
  CPPUNIT_TEST( CutElemMeshTest5c );
  CPPUNIT_TEST( CutElemMeshTest6a );
  CPPUNIT_TEST( CutElemMeshTest6b );

  CPPUNIT_TEST_SUITE_END();

public:
  CutElemMeshTest();
  ~CutElemMeshTest();

  void case1Common(ElementFragmentAlgorithm &MyMesh);

  void CutElemMeshTest1a();
  void CutElemMeshTest1b();

  void case2Intersections(ElementFragmentAlgorithm &MyMesh);
  void case2Mesh(ElementFragmentAlgorithm &MyMesh);
  void CutElemMeshTest2a();
  void CutElemMeshTest2b();
  void CutElemMeshTest2c();

  void CutElemMeshTest3();

  void CutElemMeshTest4();
  void case5Mesh(ElementFragmentAlgorithm &MyMesh);
  void CutElemMeshTest5a();
  void CutElemMeshTest5b();
  void CutElemMeshTest5c();

  void case6Mesh(ElementFragmentAlgorithm &MyMesh);
  void CutElemMeshTest6a();
  void CutElemMeshTest6b();
};

#endif  // CUTELEMMESHTEST_H

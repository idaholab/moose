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

#ifndef RANKTWOTENSORTEST_H
#define RANKTWOTENSORTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

// Moose includes
#include "RankTwoTensor.h"

class RankTwoTensorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE( RankTwoTensorTest );

  CPPUNIT_TEST( L2normTest );
  CPPUNIT_TEST( addIaTest );
  CPPUNIT_TEST( transposeTest );
  CPPUNIT_TEST( doubleContractionTest );
  CPPUNIT_TEST( traceTest );
  CPPUNIT_TEST( rotateTest );
  CPPUNIT_TEST( secondInvariantTest );
  CPPUNIT_TEST( detTest );
  CPPUNIT_TEST( deviatoricTest );
  CPPUNIT_TEST( inverseTest );
  CPPUNIT_TEST( symmetricEigenvaluesTest );
  CPPUNIT_TEST( dtraceTest );
  CPPUNIT_TEST( dsecondInvariantTest );
  CPPUNIT_TEST( ddetTest );
  CPPUNIT_TEST( dsymmetricEigenvaluesTest );

  CPPUNIT_TEST_SUITE_END();

public:
  RankTwoTensorTest();
  ~RankTwoTensorTest();

  void L2normTest();
  void addIaTest();
  void transposeTest();
  void doubleContractionTest();
  void rotateTest();
  void traceTest();
  void secondInvariantTest();
  void detTest();
  void deviatoricTest();
  void inverseTest();
  void symmetricEigenvaluesTest();
  void dtraceTest();
  void dsecondInvariantTest();
  void ddetTest();
  void dsymmetricEigenvaluesTest();

 private:
  RankTwoTensor _m0;
  RankTwoTensor _m1;
  RankTwoTensor _m2;
  RankTwoTensor _m3;
  RankTwoTensor _unsymmetric0;
  RankTwoTensor _unsymmetric1;

};

#endif  // RANKTWOTENSORTEST_H

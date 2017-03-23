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

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "RankTwoTensor.h"

class RankTwoTensorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(RankTwoTensorTest);

  CPPUNIT_TEST(L2normTest);
  CPPUNIT_TEST(addIaTest);
  CPPUNIT_TEST(transposeTest);
  CPPUNIT_TEST(doubleContractionTest);
  CPPUNIT_TEST(rotateTest);
  CPPUNIT_TEST(deviatoricTest);
  CPPUNIT_TEST(traceTest);
  CPPUNIT_TEST(dtraceTest);
  CPPUNIT_TEST(secondInvariantTest);
  CPPUNIT_TEST(dsecondInvariantTest);
  CPPUNIT_TEST(d2secondInvariantTest);
  CPPUNIT_TEST(thirdInvariantTest);
  CPPUNIT_TEST(dthirdInvariantTest);
  CPPUNIT_TEST(d2thirdInvariantTest);
  CPPUNIT_TEST(sin3LodeTest);
  CPPUNIT_TEST(dsin3LodeTest);
  CPPUNIT_TEST(d2sin3LodeTest);
  CPPUNIT_TEST(detTest);
  CPPUNIT_TEST(ddetTest);
  CPPUNIT_TEST(inverseTest);
  CPPUNIT_TEST(initialContractionTest);

  CPPUNIT_TEST_SUITE_END();

public:
  RankTwoTensorTest();
  ~RankTwoTensorTest();

  void L2normTest();
  void addIaTest();
  void transposeTest();
  void doubleContractionTest();
  void rotateTest();
  void deviatoricTest();
  void traceTest();
  void dtraceTest();
  void secondInvariantTest();
  void dsecondInvariantTest();
  void d2secondInvariantTest();
  void thirdInvariantTest();
  void dthirdInvariantTest();
  void d2thirdInvariantTest();
  void sin3LodeTest();
  void dsin3LodeTest();
  void d2sin3LodeTest();
  void detTest();
  void ddetTest();
  void inverseTest();
  void initialContractionTest();

private:
  RankTwoTensor _m0;
  RankTwoTensor _m1;
  RankTwoTensor _m2;
  RankTwoTensor _m3;
  RankTwoTensor _unsymmetric0;
  RankTwoTensor _unsymmetric1;
};

#endif // RANKTWOTENSORTEST_H

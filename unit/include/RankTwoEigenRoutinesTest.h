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

#ifndef RANKTWOEIGENROUTINESTEST_H
#define RANKTWOEIGENROUTINESTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "RankTwoTensor.h"

class RankTwoEigenRoutinesTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(RankTwoEigenRoutinesTest);

  CPPUNIT_TEST(symmetricEigenvaluesTest);
  CPPUNIT_TEST(dsymmetricEigenvaluesTest);
  CPPUNIT_TEST(d2symmetricEigenvaluesTest1);
  CPPUNIT_TEST(d2symmetricEigenvaluesTest2);

  CPPUNIT_TEST(someIdentitiesTest);

  CPPUNIT_TEST_SUITE_END();

public:
  RankTwoEigenRoutinesTest();
  ~RankTwoEigenRoutinesTest();

  void symmetricEigenvaluesTest();
  void dsymmetricEigenvaluesTest();
  void d2symmetricEigenvaluesTest1();
  void d2symmetricEigenvaluesTest2();

  void someIdentitiesTest();

private:
  RankTwoTensor _m0;
  RankTwoTensor _m1;
  RankTwoTensor _m2;
  RankTwoTensor _m3;
  RankTwoTensor _m4;
  RankTwoTensor _m5;
  RankTwoTensor _m6;
  RankTwoTensor _m7;
  RankTwoTensor _m8;
};

#endif // RANKTWOEIGENROUTINESTEST_H

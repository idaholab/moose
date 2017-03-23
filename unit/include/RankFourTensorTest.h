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

#ifndef RANKFOURTENSORTEST_H
#define RANKFOURTENSORTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "RankFourTensor.h"

class RankFourTensorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(RankFourTensorTest);

  CPPUNIT_TEST(invSymmTest1);
  CPPUNIT_TEST(invSymmTest2);

  CPPUNIT_TEST_SUITE_END();

public:
  RankFourTensorTest();
  ~RankFourTensorTest();

  void invSymmTest1();
  void invSymmTest2();

private:
  RankFourTensor _iSymmetric;
};

#endif // RANKFOURTENSORTEST_H

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

#ifndef RETURNMAPUTILSTEST_H
#define RETURNMAPUTILSTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

// Moose includes
#include "ReturnMapUtils.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

class ReturnMapUtilsTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE( ReturnMapUtilsTest );

  CPPUNIT_TEST( linearSolveTest );
  CPPUNIT_TEST( solutionErrorTest );

  CPPUNIT_TEST_SUITE_END();

public:
  ReturnMapUtilsTest();
  ~ReturnMapUtilsTest();

  void linearSolveTest();
  void solutionErrorTest();

 private:
  int num_f;
  int num_ic;
  RankFourTensor ddirn_dstress;
  std::vector<RankTwoTensor> ddirn_dpm;
  std::vector<RankTwoTensor> ddirn_dintnl;
  std::vector<RankTwoTensor> df_dstress;
  std::vector<std::vector<Real> > df_dintnl;
  std::vector<RankTwoTensor> dic_dstress;
  std::vector<std::vector<Real> > dic_dpm;
  std::vector<std::vector<Real> > dic_dintnl;
  RankTwoTensor dirn;
  std::vector<Real> f;
  std::vector<Real> ic;
};

#endif  // RETURNMAPUTILSTEST_H

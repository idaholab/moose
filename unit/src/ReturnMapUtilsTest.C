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

#include "ReturnMapUtilsTest.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"


CPPUNIT_TEST_SUITE_REGISTRATION( ReturnMapUtilsTest );

ReturnMapUtilsTest::ReturnMapUtilsTest()
{
  num_f = 4;
  num_ic = 2;

  // Create an insanely ugly matrix
  std::vector<Real> r4vals;
  r4vals.resize(21);
  for (int i = 0 ; i < 21 ; ++i)
    r4vals[i] = i+1;
  enum RankFourTensor::FillMethod fm = (RankFourTensor::FillMethod)(int)2;
  ddirn_dstress.fillFromInputVector(r4vals, fm);

  ddirn_dpm.resize(num_f);
  for (int alpha = 0 ; alpha < num_f ; ++alpha)
    ddirn_dpm[alpha] = RankTwoTensor(std::sin(alpha), -2.5*(alpha+1), std::cos(3*alpha), -2.5*(alpha+1), -4*alpha, 5.3*(alpha-5.5), std::cos(3*alpha), 5.3*(alpha-5.5), 6*(alpha+3.3));
    
  ddirn_dintnl.resize(num_ic);
  for (int a = 0 ; a < num_ic ; ++a)
    ddirn_dintnl[a] = RankTwoTensor(a+1, 2*a+2, 3*a+5.5, 2*a+2, -4*a, 8*std::sin(a)+9, 3*a+5.5, 8*std::sin(a)+9, -6*(a-2.2));

  df_dstress.resize(num_f);
  for (int alpha = 0 ; alpha < num_f ; ++alpha)
    df_dstress[alpha] = RankTwoTensor(3*(alpha+1), -2.5*(alpha+1), 3*std::exp((alpha+1.0)/10.0), -2.5*(alpha+1), -4*alpha, 7*alpha, 3*std::exp((alpha+1.0)/10.0), 5*alpha, 7.7*std::sin(alpha+1));

  df_dintnl.resize(num_f);
  for (int alpha = 0 ; alpha < num_f ; ++alpha)
  {
    df_dintnl[alpha].resize(num_ic);
    for (int a = 0 ; a < num_ic ; ++a)
      df_dintnl[alpha][a] = (a-num_ic)*std::cos(alpha+1)*1.1;
  }

  dic_dstress.resize(num_ic);
  for (int a = 0 ; a < num_ic ; ++a)
    dic_dstress[a] = RankTwoTensor(a+1, std::sin(a+4), 3*(a+1), std::sin(a+4), 4*a, 2*std::cos(a)*std::sin(a), 3*(a+1), 2*std::cos(a)*std::sin(a), 3*a);

  dic_dpm.resize(num_ic);
  for (int a = 0 ; a < num_ic ; ++a)
  {
    dic_dpm[a].resize(num_f);
    for (int alpha = 0 ; alpha < num_f ; ++alpha)
      dic_dpm[a][alpha] = (a+1)*(alpha+1)*std::cos(alpha+2);
  }

  dic_dintnl.resize(num_ic);
  for (int a = 0 ; a < num_ic ; ++a)
  {
    dic_dintnl[a].resize(num_ic);
    for (int b = 0 ; b < num_ic ; ++b)
      dic_dintnl[a][b] = a + std::cos(b*a) + 0.1*a*a;
  }


  // Create a RHS
  dirn = RankTwoTensor(1, 2, 4, 2, 3, 5, 4, 5, 6);
  f.resize(num_f);
  f[0] = 7;
  f[1] = 8;
  f[2] = 9;
  f[3] = 10;
  ic.resize(num_ic);
  ic[0] = 11;
  ic[1] = 12;
}

ReturnMapUtilsTest::~ReturnMapUtilsTest()
{}

void
ReturnMapUtilsTest::linearSolveTest()
{

  RankTwoTensor dstress;
  std::vector<Real> dpm;
  std::vector<Real> dintnl;


  ReturnMapUtils::linearSolve(dirn, f, ic, ddirn_dstress, ddirn_dpm, ddirn_dintnl, df_dstress, df_dintnl, dic_dstress, dic_dpm, dic_dintnl, dstress, dpm, dintnl);

  RankTwoTensor answer_dstress(-0.495722363236, -0.390255543628, -1.12597101002, -0.390255543628, 2.45143674228, 0.768244117773, -1.12597101002, 0.768244117773, -0.00734622156907);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (dstress - answer_dstress).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.978428844644, dpm[0], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3.04777740212, dpm[1], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.768566417118, dpm[2], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.955839872825, dpm[3], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.949509242564, dintnl[0], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.81517371472, dintnl[1], 0.0001);
}

void
ReturnMapUtilsTest::solutionErrorTest()
{

  RankTwoTensor dstress;
  std::vector<Real> dpm;
  std::vector<Real> dintnl;

  ReturnMapUtils::linearSolve(dirn, f, ic, ddirn_dstress, ddirn_dpm, ddirn_dintnl, df_dstress, df_dintnl, dic_dstress, dic_dpm, dic_dintnl, dstress, dpm, dintnl);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0,   ReturnMapUtils::solutionError(dirn, f, ic, ddirn_dstress, ddirn_dpm, ddirn_dintnl, df_dstress, df_dintnl, dic_dstress, dic_dpm, dic_dintnl, dstress, dpm, dintnl), 0.0001);

}

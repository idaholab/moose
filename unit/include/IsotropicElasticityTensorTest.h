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

#ifndef ISOTROPICELASTICITYTENSORTEST_H
#define ISOTROPICELASTICITYTENSORTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class IsotropicElasticityTensor;

class IsotropicElasticityTensorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( IsotropicElasticityTensorTest );

  CPPUNIT_TEST( constructor );
  CPPUNIT_TEST( nonConstantConstructor );
  CPPUNIT_TEST( calcLambdaMu );
  CPPUNIT_TEST( calcLambdaNu );
  CPPUNIT_TEST( calcLamdaK );
  CPPUNIT_TEST( calcLamdaE );
  CPPUNIT_TEST( calcMuNu );
  CPPUNIT_TEST( calcMuK );
  CPPUNIT_TEST( calcMuE );
  CPPUNIT_TEST( calcNuK );
  CPPUNIT_TEST( calcENu );
  CPPUNIT_TEST( calcEK );

  CPPUNIT_TEST_SUITE_END();

public:
  void constructor();
  void nonConstantConstructor();
  void calcLambdaMu();
  void calcLambdaNu();
  void calcLamdaK();
  void calcLamdaE();
  void calcMuNu();
  void calcMuK();
  void calcMuE();
  void calcNuK();
  void calcENu();
  void calcEK();

  /**
   * Measures the computed tensor against a known data set. Return a bool
   * so that if a test fails we can use the return value to fail the assert
   * inside the actual test that's failing, instead of in testMatrix and
   * the user doesn't have a clue which function called testMatrix.
   */
  bool testMatrix( double values[9][9], IsotropicElasticityTensor & tensor );

private:
  /** Data to test against. */
  static double _lambdaMu[9][9], _lambdaNu[9][9], _lambdaK[9][9], _lambdaD[9][9],
           _muNu[9][9], _muK[9][9], _muE[9][9], _nuK[9][9], _eNu[9][9], _eK[9][9];
};

#endif  // ISOTROPICELASTICITYTENSORTEST_H

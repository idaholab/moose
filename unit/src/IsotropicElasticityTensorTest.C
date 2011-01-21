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

#ifdef ELK_TEST

#include "math.h"
#include "IsotropicElasticityTensorTest.h"

//Moose includes
#include "IsotropicElasticityTensor.h"

CPPUNIT_TEST_SUITE_REGISTRATION( IsotropicElasticityTensorTest );

bool
IsotropicElasticityTensorTest::testMatrix( double values[9][9], IsotropicElasticityTensor & tensor )
{
  for (int i = 0; i < 9; ++i)
    for (int j = 0; j < 9; ++j)
    {
      if (std::abs(values[i][j] - tensor(i, j)) > 0.0001 ) //sample data goes to 4 digits
      {
        std::cout << i << ',' << j << '\n';
        std::cout << values[i][j] << ' ' << tensor(i, j) << '\n';
        std::cout << values[i][j] - tensor(i, j) << '\n';
        return false;
      }
    }

  return true;
}

void
IsotropicElasticityTensorTest::constructor()
{
  IsotropicElasticityTensor is;
  CPPUNIT_ASSERT( is.numEntries() == 81 );
}

void
IsotropicElasticityTensorTest::nonConstantConstructor()
{
  //TODO: right now I don't test for this because the behaviour of setting new
  //variables and calling calculate() depends on the order calculateLameCoefficients
  //tests the variables, resulting in arbitrary behaviour to the end user.
}

void
IsotropicElasticityTensorTest::calcLambdaMu()
{
  IsotropicElasticityTensor is;
  is.setLambda( 2.57 );
  is.setMu( 1.23 );
  is.calculate(0);
  //std::cout << "LambdaMu\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _lambdaMu, is ) );

  //verify that setShearModulus is the same as setMu
  IsotropicElasticityTensor is2;
  is2.setLambda( 2.57 );
  is2.setShearModulus( 1.23 );
  is2.calculate(0);
  CPPUNIT_ASSERT( testMatrix( _lambdaMu, is2 ) );
}

void
IsotropicElasticityTensorTest::calcLambdaNu()
{
  IsotropicElasticityTensor is;
  is.setLambda( 2.57 );
  is.setPoissonsRatio( 1.23 );
  is.calculate(0);
  //std::cout << "LambdaNu\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _lambdaNu, is ) );
}

void
IsotropicElasticityTensorTest::calcLamdaK()
{
  IsotropicElasticityTensor is;
  is.setLambda( 1.23 );
  is.setBulkModulus( 2.57 );
  is.calculate(0);
  //std::cout << "LambdaK\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _lambdaK, is ) );
}

void
IsotropicElasticityTensorTest::calcLamdaE()
{
  //TODO: do not have data to test against
  IsotropicElasticityTensor is;
  is.setLambda( 2.57 );
  is.setYoungsModulus( 1.23 );
}

void
IsotropicElasticityTensorTest::calcMuNu()
{
  IsotropicElasticityTensor is;
  is.setMu( 2.57 );
  is.setPoissonsRatio( 1.23 );
  is.calculate(0);
  //std::cout << "MuNu\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _muNu, is ) );
}

void
IsotropicElasticityTensorTest::calcMuK()
{
  IsotropicElasticityTensor is;
  is.setMu( 1.23 );
  is.setBulkModulus( 2.57 );
  is.calculate(0);
  //std::cout << "MuK\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _muK, is ) );
}

void
IsotropicElasticityTensorTest::calcMuE()
{
  IsotropicElasticityTensor is;
  is.setMu( 1.23 );
  is.setYoungsModulus( 2.57 );
  is.calculate(0);
  //std::cout << "MuE\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _muE, is ) );
}

void
IsotropicElasticityTensorTest::calcNuK()
{
  IsotropicElasticityTensor is;
  is.setPoissonsRatio( 1.23 );
  is.setBulkModulus( 2.57 );
  is.calculate(0);
  //std::cout << "NuK\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _nuK, is ) );
}

void
IsotropicElasticityTensorTest::calcENu()
{
  IsotropicElasticityTensor is;
  is.setYoungsModulus( 2.57 );
  is.setPoissonsRatio( 1.23 );
  is.calculate(0);
  //std::cout << "ENu\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _eNu, is ) );
}

void
IsotropicElasticityTensorTest::calcEK()
{
  IsotropicElasticityTensor is;
  is.setYoungsModulus( 1.23 );
  is.setBulkModulus( 2.57 );
  is.calculate(0);
  //std::cout << "EK\n";
  //is.print();
  CPPUNIT_ASSERT( testMatrix( _eK, is ) );
}

double IsotropicElasticityTensorTest::_lambdaMu[9][9] = {
  { 5.03, 0, 0, 0, 2.57, 0, 0, 0, 2.57 },
  { 0, 1.23, 0, 1.23, 0, 0, 0, 0, 0 },
  { 0, 0, 1.23, 0, 0, 0, 1.23, 0, 0 },
  { 0, 1.23, 0, 1.23, 0, 0, 0, 0, 0 },
  { 2.57, 0, 0, 0, 5.03, 0, 0, 0, 2.57 },
  { 0, 0, 0, 0, 0, 1.23, 0, 1.23, 0 },
  { 0, 0, 1.23, 0, 0, 0, 1.23, 0, 0 },
  { 0, 0, 0, 0, 0, 1.23, 0, 1.23, 0 },
  { 2.57, 0, 0, 0, 2.57, 0, 0, 0, 5.03 } };
double IsotropicElasticityTensorTest::_lambdaNu[9][9] = {
  { -0.4806, 0, 0, 0, 2.57, 0, 0, 0, 2.57 },
  { 0, -1.5253, 0, -1.5253, 0, 0, 0, 0, 0 },
  { 0, 0, -1.5253, 0, 0, 0, -1.5253, 0, 0 },
  { 0, -1.5253, 0, -1.5253, 0, 0, 0, 0, 0 },
  { 2.57, 0, 0, 0, -0.4806, 0, 0, 0, 2.57 },
  { 0, 0, 0, 0, 0, -1.5253, 0, -1.5253, 0 },
  { 0, 0, -1.5253, 0, 0, 0, -1.5253, 0, 0 },
  { 0, 0, 0, 0, 0, -1.5253, 0, -1.5253, 0 },
  { 2.57, 0, 0, 0, 2.57, 0, 0, 0, -0.4806 } };
double IsotropicElasticityTensorTest::_lambdaK[9][9] = {
  { 5.25, 0, 0, 0, 1.23, 0, 0, 0, 1.23 },
  { 0, 2.01, 0, 2.01, 0, 0, 0, 0, 0 },
  { 0, 0, 2.01, 0, 0, 0, 2.01, 0, 0 },
  { 0, 2.01, 0, 2.01, 0, 0, 0, 0, 0 },
  { 1.23, 0, 0, 0, 5.25, 0, 0, 0, 1.23 },
  { 0, 0, 0, 0, 0, 2.01, 0, 2.01, 0 },
  { 0, 0, 2.01, 0, 0, 0, 2.01, 0, 0 },
  { 0, 0, 0, 0, 0, 2.01, 0, 2.01, 0 },
  { 1.23, 0, 0, 0, 1.23, 0, 0, 0, 5.25 } };
double IsotropicElasticityTensorTest::_muNu[9][9] = {
  { 0.8097, 0, 0, 0, -4.3303, 0, 0, 0, -4.3303 },
  { 0, 2.57, 0, 2.57, 0, 0, 0, 0, 0 },
  { 0, 0, 2.57, 0, 0, 0, 2.57, 0, 0 },
  { 0, 2.57, 0, 2.57, 0, 0, 0, 0, 0 },
  { -4.3303, 0, 0, 0, 0.8097, 0, 0, 0, -4.3303 },
  { 0, 0, 0, 0, 0, 2.57, 0, 2.57, 0 },
  { 0, 0, 2.57, 0, 0, 0, 2.57, 0, 0 },
  { 0, 0, 0, 0, 0, 2.57, 0, 2.57, 0 },
  { -4.3303, 0, 0, 0, -4.3303, 0, 0, 0, 0.8097 } };
double IsotropicElasticityTensorTest::_muK[9][9] = {
  { 4.21, 0, 0, 0, 1.75, 0, 0, 0, 1.75 },
  { 0, 1.23, 0, 1.23, 0, 0, 0, 0, 0 },
  { 0, 0, 1.23, 0, 0, 0, 1.23, 0, 0 },
  { 0, 1.23, 0, 1.23, 0, 0, 0, 0, 0 },
  { 1.75, 0, 0, 0, 4.21, 0, 0, 0, 1.75 },
  { 0, 0, 0, 0, 0, 1.23, 0, 1.23, 0 },
  { 0, 0, 1.23, 0, 0, 0, 1.23, 0, 0 },
  { 0, 0, 0, 0, 0, 1.23, 0, 1.23, 0 },
  { 1.75, 0, 0, 0, 1.75, 0, 0, 0, 4.21 } };
double IsotropicElasticityTensorTest::_muE[9][9] = {
  { 2.5808, 0, 0, 0, 0.1208, 0, 0, 0, 0.1208 },
  { 0, 1.23, 0, 1.23, 0, 0, 0, 0, 0 },
  { 0, 0, 1.23, 0, 0, 0, 1.23, 0, 0 },
  { 0, 1.23, 0, 1.23, 0, 0, 0, 0, 0 },
  { 0.1208, 0, 0, 0, 2.5808, 0, 0, 0, 0.1208 },
  { 0, 0, 0, 0, 0, 1.23, 0, 1.23, 0 },
  { 0, 0, 1.23, 0, 0, 0, 1.23, 0, 0 },
  { 0, 0, 0, 0, 0, 1.23, 0, 1.23, 0 },
  { 0.1208, 0, 0, 0, 0.1208, 0, 0, 0, 2.5808 } };
double IsotropicElasticityTensorTest::_nuK[9][9] = {
  { -0.7952, 0, 0, 0, 4.2526, 0, 0, 0, 4.2526 },
  { 0, -2.5239, 0, -2.5239, 0, 0, 0, 0, 0 },
  { 0, 0, -2.5239, 0, 0, 0, -2.5239, 0, 0 },
  { 0, -2.5239, 0, -2.5239, 0, 0, 0, 0, 0 },
  { 4.2526, 0, 0, 0, -0.7952, 0, 0, 0, 4.2526 },
  { 0, 0, 0, 0, 0, -2.5239, 0, -2.5239, 0 },
  { 0, 0, -2.5239, 0, 0, 0, -2.5239, 0, 0 },
  { 0, 0, 0, 0, 0, -2.5239, 0, -2.5239, 0 },
  { 4.2526, 0, 0, 0, 4.2526, 0, 0, 0, -0.7952 } };
double IsotropicElasticityTensorTest::_eNu[9][9] = {
  { 0.1816, 0, 0, 0, -0.9709, 0, 0, 0, -0.9709 },
  { 0, 0.5762, 0, 0.5762, 0, 0, 0, 0, 0 },
  { 0, 0, 0.5762, 0, 0, 0, 0.5762, 0, 0 },
  { 0, 0.5762, 0, 0.5762, 0, 0, 0, 0, 0 },
  { -0.9709, 0, 0, 0, 0.1816, 0, 0, 0, -0.9709 },
  { 0, 0, 0, 0, 0, 0.5762, 0, 0.5762, 0 },
  { 0, 0, 0.5762, 0, 0, 0, 0.5762, 0, 0 },
  { 0, 0, 0, 0, 0, 0.5762, 0, 0.5762, 0 },
  { -0.9709, 0, 0, 0, -0.9709, 0, 0, 0, 0.1816 } };
double IsotropicElasticityTensorTest::_eK[9][9] = {
  { 3.14737, 0, 0, 0, 2.28132, 0, 0, 0, 2.28132 },
  { 0, 0.433027, 0, 0.433027, 0, 0, 0, 0, 0 },
  { 0, 0, 0.433027, 0, 0, 0, 0.433027, 0, 0 },
  { 0, 0.433027, 0, 0.433027, 0, 0, 0, 0, 0 },
  { 2.28132, 0, 0, 0, 3.14737, 0, 0, 0, 2.28132 },
  { 0, 0, 0, 0, 0, 0.433027, 0, 0.433027, 0 },
  { 0, 0, 0.433027, 0, 0, 0, 0.433027, 0, 0 },
  { 0, 0, 0, 0, 0, 0.433027, 0, 0.433027, 0 },
  { 2.28132, 0, 0, 0, 2.28132, 0, 0, 0, 3.14737 } };

#endif //ELK_TEST: testing if elk library is included

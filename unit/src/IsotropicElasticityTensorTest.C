//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <math.h>
#include "IsotropicElasticityTensorTest.h"

TEST_F(IsotropicElasticityTensorTest, constructor)
{
  IsotropicElasticityTensor is;
  EXPECT_EQ(is.numEntries(), 81);
}

TEST_F(IsotropicElasticityTensorTest, nonConstantConstructor)
{
  // TODO: right now I don't test for this because the behaviour of setting new
  // variables and calling calculate() depends on the order calculateLameCoefficients
  // tests the variables, resulting in arbitrary behaviour to the end user.
}

TEST_F(IsotropicElasticityTensorTest, calcLambdaMu)
{
  IsotropicElasticityTensor is;
  is.setLambda(2.57);
  is.setMu(1.23);
  is.calculate(0);
  // Moose::out << "LambdaMu\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_lambdaMu, is));

  // verify that setShearModulus is the same as setMu
  IsotropicElasticityTensor is2;
  is2.setLambda(2.57);
  is2.setShearModulus(1.23);
  is2.calculate(0);
  ASSERT_TRUE(testMatrix(_lambdaMu, is2));
}

TEST_F(IsotropicElasticityTensorTest, calcLambdaNu)
{
  IsotropicElasticityTensor is;
  is.setLambda(2.57);
  is.setPoissonsRatio(1.23);
  is.calculate(0);
  // Moose::out << "LambdaNu\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_lambdaNu, is));
}

TEST_F(IsotropicElasticityTensorTest, calcLamdaK)
{
  IsotropicElasticityTensor is;
  is.setLambda(1.23);
  is.setBulkModulus(2.57);
  is.calculate(0);
  // Moose::out << "LambdaK\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_lambdaK, is));
}

TEST_F(IsotropicElasticityTensorTest, calcLamdaE)
{
  // TODO: do not have data to test against
  IsotropicElasticityTensor is;
  is.setLambda(2.57);
  is.setYoungsModulus(1.23);
}

TEST_F(IsotropicElasticityTensorTest, calcMuNu)
{
  IsotropicElasticityTensor is;
  is.setMu(2.57);
  is.setPoissonsRatio(1.23);
  is.calculate(0);
  // Moose::out << "MuNu\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_muNu, is));
}

TEST_F(IsotropicElasticityTensorTest, calcMuK)
{
  IsotropicElasticityTensor is;
  is.setMu(1.23);
  is.setBulkModulus(2.57);
  is.calculate(0);
  // Moose::out << "MuK\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_muK, is));
}

TEST_F(IsotropicElasticityTensorTest, calcMuE)
{
  IsotropicElasticityTensor is;
  is.setMu(1.23);
  is.setYoungsModulus(2.57);
  is.calculate(0);
  // Moose::out << "MuE\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_muE, is));
}

TEST_F(IsotropicElasticityTensorTest, calcNuK)
{
  IsotropicElasticityTensor is;
  is.setPoissonsRatio(1.23);
  is.setBulkModulus(2.57);
  is.calculate(0);
  // Moose::out << "NuK\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_nuK, is));
}

TEST_F(IsotropicElasticityTensorTest, calcENu)
{
  IsotropicElasticityTensor is;
  is.setYoungsModulus(2.57);
  is.setPoissonsRatio(1.23);
  is.calculate(0);
  // Moose::out << "ENu\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_eNu, is));
}

TEST_F(IsotropicElasticityTensorTest, calcEK)
{
  IsotropicElasticityTensor is;
  is.setYoungsModulus(1.23);
  is.setBulkModulus(2.57);
  is.calculate(0);
  // Moose::out << "EK\n";
  // is.print();
  ASSERT_TRUE(testMatrix(_eK, is));
}

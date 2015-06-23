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
#include "RankFourTensorTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( RankFourTensorTest );

RankFourTensorTest::RankFourTensorTest()
{
  _iSymmetric = RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
}

RankFourTensorTest::~RankFourTensorTest()
{}

void
RankFourTensorTest::matrixInversionTest1()
{
  // The matrix
  // ( 2  2)
  // (0.5 1)
  // has inverse
  // ( 1  -2)
  // (-0.5 2)
  std::vector<PetscScalar> mat2(2*2);
  mat2[0] = 2;
  mat2[1] = 2;
  mat2[2] = 0.5;
  mat2[3] = 1;
  int error = _iSymmetric.matrixInversion(mat2, 2);
  CPPUNIT_ASSERT(error == 0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1, mat2[0], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-2, mat2[1], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5, mat2[2], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(2, mat2[3], 1E-5);
}

void
RankFourTensorTest::matrixInversionTest2()
{
  // The matrix
  // (1 2 3)
  // (4 5 6)
  // (7 8 1)
  // is singular
  std::vector<PetscScalar> mat3(3*3);
  mat3[0] = 1;
  mat3[1] = 2;
  mat3[2] = 3;
  mat3[3] = 4;
  mat3[4] = 5;
  mat3[5] = 6;
  mat3[6] = 7;
  mat3[7] = 8;
  mat3[8] = 9;
  int error = _iSymmetric.matrixInversion(mat3, 3);
  //CPPUNIT_ASSERT_ASSERTION_FAIL(CPPUNIT_ASSERT(error == 0));
  CPPUNIT_ASSERT(error != 0);
}

void
RankFourTensorTest::matrixInversionTest3()
{
  // The matrix
  // (1 2 3)
  // (0 1 4)
  // (5 6 0)
  // has inverse
  // (-24 18  5)
  // (20 -15 -4)
  // (-5  4   1)
  std::vector<PetscScalar> mat3(3*3);
  mat3[0] = 1;
  mat3[1] = 2;
  mat3[2] = 3;
  mat3[3] = 0;
  mat3[4] = 1;
  mat3[5] = 4;
  mat3[6] = 5;
  mat3[7] = 6;
  mat3[8] = 0;
  int error = _iSymmetric.matrixInversion(mat3, 3);
  CPPUNIT_ASSERT(error == 0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-24, mat3[0], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(18, mat3[1], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(5, mat3[2], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(20, mat3[3], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-15, mat3[4], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-4, mat3[5], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-5, mat3[6], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(4, mat3[7], 1E-5);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1, mat3[8], 1E-5);
}


void
RankFourTensorTest::invSymmTest1()
{
  // inverse check for a standard symmetric-isotropic tensor
  std::vector<Real> input(2);
  input[0] = 1;
  input[1] = 3;
  RankFourTensor a(input, RankFourTensor::symmetric_isotropic);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_iSymmetric - a.invSymm()*a).L2norm(), 1E-5);
}


void
RankFourTensorTest::invSymmTest2()
{
  // following (basically random) "a" tensor has symmetry
  // a_ijkl = a_jikl = a_ijlk
  // BUT it doesn't have a_ijkl = a_klij
  RankFourTensor a;
  a(0, 0, 0, 0) = 1;
  a(0, 0, 0, 1) = a(0, 0, 1, 0) = 2;
  a(0, 0, 0, 2) = a(0, 0, 2, 0) = 1.1;
  a(0, 0, 1, 1) = 0.5;
  a(0, 0, 1, 2) = a(0, 0, 2, 1) = -0.2;
  a(0, 0, 2, 2) = 0.8;
  a(1, 1, 0, 0) = 0.6;
  a(1, 1, 0, 1) = a(1, 1, 1, 0) = 1.3;
  a(1, 1, 0, 2) = a(1, 1, 2, 0) = 0.9;
  a(1, 1, 1, 1) = 0.6;
  a(1, 1, 1, 2) = a(1, 1, 2, 1) = -0.3;
  a(1, 1, 2, 2) = -1.1;
  a(2, 2, 0, 0) = -0.6;
  a(2, 2, 0, 1) = a(2, 2, 1, 0) = -0.1;
  a(2, 2, 0, 2) = a(2, 2, 2, 0) = -0.3;
  a(2, 2, 1, 1) = 0.5;
  a(2, 2, 1, 2) = a(2, 2, 2, 1) = 0.7;
  a(2, 2, 2, 2) = -0.9;
  a(0, 1, 0, 0) = a(1, 0, 0, 0) = 1.2;
  a(0, 1, 0, 1) = a(0, 1, 1, 0) = a(1, 0, 0, 1) = a(1, 0, 1, 0) = 0.1;
  a(0, 1, 0, 2) = a(0, 1, 2, 0) = a(1, 0, 0, 2) = a(1, 0, 2, 0) = 0.15;
  a(0, 1, 1, 1) = a(1, 0, 1, 1) = 0.24;
  a(0, 1, 1, 2) = a(0, 1, 2, 1) = a(1, 0, 1, 2) = a(1, 0, 2, 1) = -0.4;
  a(0, 1, 2, 2) = a(1, 0, 2, 2) = -0.3;
  a(0, 2, 0, 0) = a(2, 0, 0, 0) = -0.3;
  a(0, 2, 0, 1) = a(0, 2, 1, 0) = a(2, 0, 0, 1) = a(2, 0, 1, 0) = -0.4;
  a(0, 2, 0, 2) = a(0, 2, 2, 0) = a(2, 0, 0, 2) = a(2, 0, 2, 0) = 0.22;
  a(0, 2, 1, 1) = a(2, 0, 1, 1) = -0.9;
  a(0, 2, 1, 2) = a(0, 2, 2, 1) = a(2, 0, 1, 2) = a(2, 0, 2, 1) = -0.05;
  a(0, 2, 2, 2) = a(2, 0, 2, 2) = 0.32;
  a(1, 2, 0, 0) = a(2, 1, 0, 0) = -0.35;
  a(1, 2, 0, 1) = a(1, 2, 1, 0) = a(2, 1, 0, 1) = a(2, 1, 1, 0) = -1.4;
  a(1, 2, 0, 2) = a(1, 2, 2, 0) = a(2, 1, 0, 2) = a(2, 1, 2, 0) = 0.2;
  a(1, 2, 1, 1) = a(2, 1, 1, 1) = -0.91;
  a(1, 2, 1, 2) = a(1, 2, 2, 1) = a(2, 1, 1, 2) = a(2, 1, 2, 1) = 1.4;
  a(1, 2, 2, 2) = a(2, 1, 2, 2) = 0.1;

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_iSymmetric - a.invSymm()*a).L2norm(), 1E-5);
}

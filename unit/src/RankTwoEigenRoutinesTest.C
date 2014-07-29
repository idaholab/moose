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
#include "RankTwoEigenRoutinesTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( RankTwoEigenRoutinesTest );

RankTwoEigenRoutinesTest::RankTwoEigenRoutinesTest()
{
  _m0 = RankTwoTensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
  _m1 = RankTwoTensor(1, 0, 0, 0, 1, 0, 0, 0, 1);
  _m2 = RankTwoTensor(1, 0, 0, 0, 2, 0, 0, 0, 3);
  _m3 = RankTwoTensor(1, 2, 3, 2, -5, -6, 3, -6, 9);
  _m4 = RankTwoTensor(1, 0, 0, 0, 3, 0, 0, 0, 2);
  _unsymmetric0 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 9);
  _unsymmetric1 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 10);
}

RankTwoEigenRoutinesTest::~RankTwoEigenRoutinesTest()
{}

void
RankTwoEigenRoutinesTest::symmetricEigenvaluesTest()
{
  std::vector<Real> eigvals;

  RankTwoEigenRoutines::symmetricEigenvalues(_m0, eigvals);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, eigvals[0], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, eigvals[1], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, eigvals[2], 0.0001);

  RankTwoEigenRoutines::symmetricEigenvalues(_m2, eigvals);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1, eigvals[0], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(2, eigvals[1], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3, eigvals[2], 0.0001);

  RankTwoEigenRoutines::symmetricEigenvalues(_m3, eigvals);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-8.17113, eigvals[0], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.51145, eigvals[1], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(11.6597, eigvals[2], 0.0001);

  // note that only the upper-diagonal part of m is used
  // in the symmetric_eigenvalues routine - so the routine
  // only works for symmetric matrices!!
  // That is why the following m appears to have the same
  // eigenvalues as the previous m !
  RankTwoEigenRoutines::symmetricEigenvalues(_unsymmetric0, eigvals);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-8.17113, eigvals[0], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.51145, eigvals[1], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(11.6597, eigvals[2], 0.0001);
}

void
RankTwoEigenRoutinesTest::dsymmetricEigenvaluesTest()
{
  // this derivative is less trivial than dtrace and dsecondInvariant,
  // so let's check with a finite-difference approximation
  Real ep = 1E-5; // small finite-difference parameter

  std::vector<Real> eigvals; // eigenvalues in ascending order provided by RankTwoTensor
  std::vector<RankTwoTensor> deriv; // derivatives of these eigenvalues provided by RankTwoTensor

  RankTwoTensor mep; // the RankTwoTensor with successive entries shifted by ep
  std::vector<Real> eigvalsep; // eigenvalues of mep in ascending order

  RankTwoEigenRoutines::dsymmetricEigenvalues(_m2, eigvals, deriv);
  mep = _m2;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
    {
      // note the explicit symmeterisation here
      mep(i, j) += ep/2;
      mep(j, i) += ep/2;
      RankTwoEigenRoutines::symmetricEigenvalues(mep, eigvalsep);
      for (unsigned k = 0 ; k < 3 ; ++k)
        CPPUNIT_ASSERT_DOUBLES_EQUAL((eigvalsep[k] - eigvals[k])/ep, deriv[k](i, j), ep);
      mep(i, j) -= ep/2;
      mep(j, i) -= ep/2;
    }

  RankTwoEigenRoutines::dsymmetricEigenvalues(_m3, eigvals, deriv);
  mep = _m3;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
    {
      // note the explicit symmeterisation here
      mep(i, j) += ep/2;
      mep(j, i) += ep/2;
      RankTwoEigenRoutines::symmetricEigenvalues(mep, eigvalsep);
      for (unsigned k = 0 ; k < 3 ; ++k)
        CPPUNIT_ASSERT_DOUBLES_EQUAL((eigvalsep[k] - eigvals[k])/ep, deriv[k](i, j), ep);
      mep(i, j) -= ep/2;
      mep(j, i) -= ep/2;
    }
}

/**
* Validity of the second derivatives has been tested by splitting a 3x3 matrix into 2x2 matrices
* Ex: 3x3 symmetric matrix
*       a00 a01 a02
*   A = a10 a11 a12
*       a20 a21 a22
* Upper left four elements and lower right four elements can be given by 2x2 matrices
*       a10 a01   and  a11 a12
*       a10 a11        a21 a22
*
* Now eigen values of the above 2x2 matrixes can be written as:
*    lamda = 0.5*[(a00+a11) + or - sqrt[(a00+a11)^2-4(a00.a11-((a01+a10)/2)^2)]
* By differentiating lamda with respect to a00, a01, a11, a12, a22
* some elements of rank four tensor ie. a0000, a0001, a1100, a0101, a1111, a1112, a2211, a1212
* (second derivatives) can be verified
* Furthermore the validity of all the elements of rank four tensor has been tested in
* "d2symmetricEigenvaluesTest2" method using finite difference method.
*/
void
RankTwoEigenRoutinesTest::d2symmetricEigenvaluesTest1()
{
  std::vector<RankFourTensor> second_deriv;
  RankTwoEigenRoutines::d2symmetricEigenvalues(_m4, second_deriv);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](0,0,0,0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](0,0,0,1), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.25, second_deriv[0](0,1,0,1), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.25, second_deriv[0](0,1,1,0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](1,1,0,0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](2,2,0,0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](1,1,1,0), 0.000001);

  RankTwoEigenRoutines::d2symmetricEigenvalues(_m2, second_deriv);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](0,0,0,0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](0,0,0,1), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5, second_deriv[0](0,1,0,1), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5, second_deriv[0](0,1,1,0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](1,1,0,0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](2,2,0,0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](1,1,1,0), 0.000001);
}

/**
* Second derivative of Eginvalues are compared with finite difference method
* This method checks all the elements in RankFourTensor
**/
void
RankTwoEigenRoutinesTest::d2symmetricEigenvaluesTest2()
{
  Real ep = 1E-5; // small finite-difference parameter
  std::vector<Real> eigvals,eigvalsep; // eigenvalues in ascending order provided by RankTwoTensor
  std::vector<RankTwoTensor> deriv,derivep; // derivatives of these eigenvalues provided by RankTwoTensor
  std::vector<RankFourTensor> second_deriv;

  RankTwoTensor mep; // the RankTwoTensor with successive entries shifted by ep

  RankTwoEigenRoutines::d2symmetricEigenvalues(_m2, second_deriv);
  RankTwoEigenRoutines::dsymmetricEigenvalues(_m2, eigvals, deriv);

  mep = _m2;

  for (unsigned int m = 0; m < 3; m++)
    for (unsigned i = 0; i < 3; i++)
      for (unsigned j = 0; j < 3; j++)
      {
        for (unsigned int k=0; k<3; k++)
          for (unsigned int l=0; l<3; l++)
          {
            mep(k, l) += ep/2;
            mep(l, k) += ep/2;

            RankTwoEigenRoutines::dsymmetricEigenvalues(mep, eigvalsep, derivep);
            CPPUNIT_ASSERT_DOUBLES_EQUAL((derivep[m](i, j) - deriv[m](i, j))/ep, second_deriv[m](i, j, k, l), ep);
            mep(k, l) -= ep/2;
            mep(l, k) -= ep/2;
          }
      }

  RankTwoEigenRoutines::d2symmetricEigenvalues(_m3, second_deriv);
  RankTwoEigenRoutines::dsymmetricEigenvalues(_m3, eigvals, deriv);

  mep = _m3;

  for (unsigned int m = 0; m < 3; m++)
    for (unsigned i = 0; i < 3; i++)
      for (unsigned j = 0; j < 3; j++)
      {
        for (unsigned int k=0; k<3; k++)
          for (unsigned int l=0; l<3; l++)
          {
            mep(k, l) += ep/2;
            mep(l, k) += ep/2;

            RankTwoEigenRoutines::dsymmetricEigenvalues(mep, eigvalsep, derivep);
            CPPUNIT_ASSERT_DOUBLES_EQUAL((derivep[m](i, j) - deriv[m](i, j))/ep, second_deriv[m](i, j, k, l), ep);
            mep(k, l) -= ep/2;
            mep(l, k) -= ep/2;
          }
      }
}

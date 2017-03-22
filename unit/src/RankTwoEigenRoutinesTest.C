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

CPPUNIT_TEST_SUITE_REGISTRATION(RankTwoEigenRoutinesTest);

RankTwoEigenRoutinesTest::RankTwoEigenRoutinesTest()
{
  _m0 = RankTwoTensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
  _m1 = RankTwoTensor(1, 0, 0, 0, 1, 0, 0, 0, 1);
  _m2 = RankTwoTensor(1, 0, 0, 0, 2, 0, 0, 0, 3);
  _m3 = RankTwoTensor(1, 2, 3, 2, -5, -6, 3, -6, 9);
  _m4 = RankTwoTensor(1, 0, 0, 0, 3, 0, 0, 0, 2);
  _m5 = RankTwoTensor(1, 0, 0, 0, 1, 0, 0, 0, 2);
  _m6 = RankTwoTensor(1, 0, 0, 0, 2, 0, 0, 0, 1);
  _m7 = RankTwoTensor(1, 0, 0, 0, 2, 0, 0, 0, 2);
  _m8 = RankTwoTensor(1, 1, 0, 1, 1, 0, 0, 0, 2); // has eigenvalues 0, 2 and 2
}

RankTwoEigenRoutinesTest::~RankTwoEigenRoutinesTest() {}

void
RankTwoEigenRoutinesTest::symmetricEigenvaluesTest()
{
  std::vector<Real> eigvals;

  _m0.symmetricEigenvalues(eigvals);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, eigvals[0], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, eigvals[1], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, eigvals[2], 0.0001);

  _m2.symmetricEigenvalues(eigvals);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1, eigvals[0], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(2, eigvals[1], 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3, eigvals[2], 0.0001);

  _m3.symmetricEigenvalues(eigvals);
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

  std::vector<Real> eigvals;        // eigenvalues in ascending order provided by RankTwoTensor
  std::vector<RankTwoTensor> deriv; // derivatives of these eigenvalues provided by RankTwoTensor

  RankTwoTensor mep;                 // the RankTwoTensor with successive entries shifted by ep
  std::vector<Real> eigvalsep;       // eigenvalues of mep in ascending order
  std::vector<Real> eigvalsep_minus; // for equal-eigenvalue cases, i take a central difference

  _m2.dsymmetricEigenvalues(eigvals, deriv);
  mep = _m2;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      mep(i, j) += ep;
      mep.symmetricEigenvalues(eigvalsep);
      for (unsigned k = 0; k < 3; ++k)
        CPPUNIT_ASSERT_DOUBLES_EQUAL((eigvalsep[k] - eigvals[k]) / ep, deriv[k](i, j), ep);
      mep(i, j) -= ep;
    }

  _m3.dsymmetricEigenvalues(eigvals, deriv);
  mep = _m3;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      mep(i, j) += ep;
      mep.symmetricEigenvalues(eigvalsep);
      for (unsigned k = 0; k < 3; ++k)
        CPPUNIT_ASSERT_DOUBLES_EQUAL((eigvalsep[k] - eigvals[k]) / ep, deriv[k](i, j), ep);
      mep(i, j) -= ep;
    }

  // the equal-eigenvalue cases follow:

  _m5.dsymmetricEigenvalues(eigvals, deriv);
  mep = _m5;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      // here i use a central difference to define the
      // discontinuous derivative
      mep(i, j) += ep / 2.0;
      mep.symmetricEigenvalues(eigvalsep);
      mep(i, j) -= ep;
      mep.symmetricEigenvalues(eigvalsep_minus);
      for (unsigned k = 0; k < 3; ++k)
        CPPUNIT_ASSERT_DOUBLES_EQUAL((eigvalsep[k] - eigvalsep_minus[k]) / ep, deriv[k](i, j), ep);
      mep(i, j) += ep / 2.0;
    }

  _m6.dsymmetricEigenvalues(eigvals, deriv);
  mep = _m6;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      // here i use a central difference to define the
      // discontinuous derivative
      mep(i, j) += ep / 2.0;
      mep.symmetricEigenvalues(eigvalsep);
      mep(i, j) -= ep;
      mep.symmetricEigenvalues(eigvalsep_minus);
      for (unsigned k = 0; k < 3; ++k)
        CPPUNIT_ASSERT_DOUBLES_EQUAL((eigvalsep[k] - eigvalsep_minus[k]) / ep, deriv[k](i, j), ep);
      mep(i, j) += ep / 2.0;
    }

  _m7.dsymmetricEigenvalues(eigvals, deriv);
  mep = _m7;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      // here i use a central difference to define the
      // discontinuous derivative
      mep(i, j) += ep / 2.0;
      mep.symmetricEigenvalues(eigvalsep);
      mep(i, j) -= ep;
      mep.symmetricEigenvalues(eigvalsep_minus);
      for (unsigned k = 0; k < 3; ++k)
        CPPUNIT_ASSERT_DOUBLES_EQUAL((eigvalsep[k] - eigvalsep_minus[k]) / ep, deriv[k](i, j), ep);
      mep(i, j) += ep / 2.0;
    }

  _m8.dsymmetricEigenvalues(eigvals, deriv);
  mep = _m8;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      // here i use a central difference to define the
      // discontinuous derivative
      mep(i, j) += ep / 2.0;
      mep.symmetricEigenvalues(eigvalsep);
      mep(i, j) -= ep;
      mep.symmetricEigenvalues(eigvalsep_minus);
      for (unsigned k = 0; k < 3; ++k)
        CPPUNIT_ASSERT_DOUBLES_EQUAL((eigvalsep[k] - eigvalsep_minus[k]) / ep, deriv[k](i, j), ep);
      mep(i, j) += ep / 2.0;
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
  _m4.d2symmetricEigenvalues(second_deriv);

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](0, 0, 0, 0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](0, 0, 0, 1), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.25, second_deriv[0](0, 1, 0, 1), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.25, second_deriv[0](0, 1, 1, 0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](1, 1, 0, 0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](2, 2, 0, 0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](1, 1, 1, 0), 0.000001);

  _m2.d2symmetricEigenvalues(second_deriv);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](0, 0, 0, 0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](0, 0, 0, 1), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5, second_deriv[0](0, 1, 0, 1), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.5, second_deriv[0](0, 1, 1, 0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](1, 1, 0, 0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](2, 2, 0, 0), 0.000001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, second_deriv[0](1, 1, 1, 0), 0.000001);
}

/**
* Second derivative of Eginvalues are compared with finite difference method
* This method checks all the elements in RankFourTensor
**/
void
RankTwoEigenRoutinesTest::d2symmetricEigenvaluesTest2()
{
  Real ep = 1E-5; // small finite-difference parameter
  std::vector<Real> eigvals, eigvalsep,
      eigvalsep_minus; // eigenvalues in ascending order provided by RankTwoTensor
  std::vector<RankTwoTensor> deriv, derivep,
      derivep_minus; // derivatives of these eigenvalues provided by RankTwoTensor
  std::vector<RankFourTensor> second_deriv;

  RankTwoTensor mep; // the RankTwoTensor with successive entries shifted by ep

  _m2.d2symmetricEigenvalues(second_deriv);
  _m2.dsymmetricEigenvalues(eigvals, deriv);
  mep = _m2;
  for (unsigned int m = 0; m < 3; m++)
    for (unsigned i = 0; i < 3; i++)
      for (unsigned j = 0; j < 3; j++)
      {
        for (unsigned int k = 0; k < 3; k++)
          for (unsigned int l = 0; l < 3; l++)
          {
            mep(k, l) += ep;
            mep.dsymmetricEigenvalues(eigvalsep, derivep);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(
                (derivep[m](i, j) - deriv[m](i, j)) / ep, second_deriv[m](i, j, k, l), ep);
            mep(k, l) -= ep;
          }
      }

  _m3.d2symmetricEigenvalues(second_deriv);
  _m3.dsymmetricEigenvalues(eigvals, deriv);
  mep = _m3;
  for (unsigned int m = 0; m < 3; m++)
    for (unsigned i = 0; i < 3; i++)
      for (unsigned j = 0; j < 3; j++)
      {
        for (unsigned int k = 0; k < 3; k++)
          for (unsigned int l = 0; l < 3; l++)
          {
            mep(k, l) += ep;
            mep.dsymmetricEigenvalues(eigvalsep, derivep);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(
                (derivep[m](i, j) - deriv[m](i, j)) / ep, second_deriv[m](i, j, k, l), ep);
            mep(k, l) -= ep;
          }
      }
}

void
RankTwoEigenRoutinesTest::someIdentitiesTest()
{
  // checks identities that should hold if eigenvalues
  // and invariants are correctly calculated
  std::vector<Real> eigvals;
  _m3.symmetricEigenvalues(eigvals);

  Real mean = _m3.trace() / 3.0;
  Real secondInvariant = _m3.secondInvariant();
  Real shear = std::sqrt(secondInvariant);
  _m3.thirdInvariant();

  Real lode = std::asin(_m3.sin3Lode(0, 0) / 3.0);

  Real two_pi_over_3 = 2.09439510239;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      eigvals[0], 2 * shear * std::sin(lode - two_pi_over_3) / std::sqrt(3.0) + mean, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      eigvals[1], 2 * shear * std::sin(lode) / std::sqrt(3.0) + mean, 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(
      eigvals[2], 2 * shear * std::sin(lode + two_pi_over_3) / std::sqrt(3.0) + mean, 0.0001);
}

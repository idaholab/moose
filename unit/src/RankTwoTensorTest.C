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
#include "RankTwoTensorTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( RankTwoTensorTest );

RankTwoTensorTest::RankTwoTensorTest()
{
  _m0 = RankTwoTensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
  _m1 = RankTwoTensor(1, 0, 0, 0, 1, 0, 0, 0, 1);
  _m2 = RankTwoTensor(1, 0, 0, 0, 2, 0, 0, 0, 3);
  _m3 = RankTwoTensor(1, 2, 3, 2, -5, -6, 3, -6, 9);
  _unsymmetric0 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 9);
  _unsymmetric1 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 10);
}

RankTwoTensorTest::~RankTwoTensorTest()
{}

void
RankTwoTensorTest::L2normTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _m0.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.732051, _m1.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3.741657, _m2.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(14.31782, _m3.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(16.88194, _unsymmetric0.L2norm(), 0.0001);
}

void
RankTwoTensorTest::addIaTest()
{
  RankTwoTensor m(1, 2, 3, 4, 5, 6, 7, 8, 9);
  m.addIa(1.23);
  RankTwoTensor n(2.23, 2, 3, 4, 6.23, 6, 7, 8, 10.23);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (m - n).L2norm(), 0.0001);
}

void
RankTwoTensorTest::transposeTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m0.transpose() - _m0).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m1.transpose() - _m1).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m2.transpose() - _m2).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m3.transpose() - _m3).L2norm(), 0.0001);
  RankTwoTensor t(1, -4, 7, 2, -5, 8, 3, -6, 9);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_unsymmetric0.transpose() - t).L2norm(), 0.0001);
}

void
RankTwoTensorTest::doubleContractionTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(121, _m3.doubleContraction(_unsymmetric0), 0.0001);
}

void
RankTwoTensorTest::rotateTest()
{
  Real sqrt2 = 0.707106781187;
  RealTensorValue rtv0(sqrt2, -sqrt2, 0, sqrt2, sqrt2, 0, 0, 0, 1); // rotation about "0" axis
  RealTensorValue rtv1(sqrt2, 0, -sqrt2, 0, 1, 0, sqrt2, 0, sqrt2); // rotation about "1" axis
  RealTensorValue rtv2(1, 0, 0, 0, sqrt2, -sqrt2, 0, sqrt2, sqrt2); // rotation about "2" axis

  RankTwoTensor rot0(rtv0);
  RankTwoTensor rot0T = rot0.transpose();
  RankTwoTensor rot1(rtv1);
  RankTwoTensor rot1T = rot1.transpose();
  RankTwoTensor rot2(rtv2);
  RankTwoTensor rot2T = rot2.transpose();
  RankTwoTensor rot = rot0*rot1*rot2;

  RankTwoTensor answer;
  RankTwoTensor m3;

  // the following "answer"s come from mathematica of course!

  // rotate about "0" axis with RealTensorValue, then back again with RankTwoTensor
  m3 = _m3;
  answer = RankTwoTensor(-4, 3, 6.363961, 3, 0, -2.1213403, 6.363961, -2.1213403, 9);
  m3.rotate(rtv0);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (m3 - answer).L2norm(), 0.0001);
  m3.rotate(rot0T);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (m3 - _m3).L2norm(), 0.0001);

  // rotate about "1" axis with RealTensorValue, then back again with RankTwoTensor
  m3 = _m3;
  answer = RankTwoTensor(2, 5.656854, -4, 5.656854, -5, -2.828427, -4, -2.828427, 8);
  m3.rotate(rtv1);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (m3 - answer).L2norm(), 0.0001);
  m3.rotate(rot1T);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (m3 - _m3).L2norm(), 0.0001);

  // rotate about "2" axis with RealTensorValue, then back again with RankTwoTensor
  m3 = _m3;
  answer = RankTwoTensor(1, -sqrt2, 3.5355339, -sqrt2, 8, -7, 3.5355339, -7, -4);
  m3.rotate(rtv2);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (m3 - answer).L2norm(), 0.0001);
  m3.rotate(rot2T);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (m3 - _m3).L2norm(), 0.0001);

  // rotate with "rot"
  m3 = _m3;
  answer = RankTwoTensor(-2.9675144, -6.51776695, 5.6213203, -6.51776695, 5.9319805, -2.0857864, 5.6213203, -2.0857864, 2.0355339);
  m3.rotate(rot);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (m3 - answer).L2norm(), 0.0001);
}

void
RankTwoTensorTest::traceTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _m0.trace(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3, _m1.trace(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(6, _m2.trace(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(5, _m3.trace(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(5, _unsymmetric0.trace(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(6, _unsymmetric1.trace(), 0.0001);
}

void
RankTwoTensorTest::secondInvariantTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _m0.secondInvariant(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _m1.secondInvariant(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1, _m2.secondInvariant(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(146, _unsymmetric1.secondInvariant(), 0.0001);
}

void
RankTwoTensorTest::detTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _m0.det(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1, _m1.det(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(6, _m2.det(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(-144, _m3.det(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _unsymmetric0.det(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3, _unsymmetric1.det(), 0.0001);
}

void
RankTwoTensorTest::deviatoricTest()
{
  RankTwoTensor dev(-1, 2, 3, -4, -7, -6, 7, 8, 8);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (dev - _unsymmetric1.deviatoric()).L2norm(), 0.0001);
}

void
RankTwoTensorTest::inverseTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m3*_m3.inverse() - _m1).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_unsymmetric1*_unsymmetric1.inverse() - _m1).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_unsymmetric1.inverse()*_unsymmetric1 - _m1).L2norm(), 0.0001);
}

void
RankTwoTensorTest::dtraceTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m0.dtrace() - _m1).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m3.dtrace() - _m1).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_unsymmetric0.dtrace() - _m1).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_unsymmetric1.dtrace() - _m1).L2norm(), 0.0001);
}

void
RankTwoTensorTest::dsecondInvariantTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m0.dsecondInvariant() - _m0.deviatoric()).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m3.dsecondInvariant() - _m3.deviatoric()).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_unsymmetric0.dsecondInvariant() - _unsymmetric0.deviatoric()).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_unsymmetric1.dsecondInvariant() - _unsymmetric1.deviatoric()).L2norm(), 0.0001);
}

void
RankTwoTensorTest::ddetTest()
{
  // this derivative is less trivial than dtrace and dsecondInvariant,
  // so let's check with a finite-difference approximation
  Real ep = 1E-5; // small finite-difference parameter
  Real det; // determinant provided by RankTwoTensor
  RankTwoTensor deriv; // derivative of det provided by RankTwoTensor
  RankTwoTensor mep; // the RankTwoTensor with successive entries shifted by ep

  det = _m3.det();
  deriv = _m3.ddet();
  mep = _m3;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
    {
      mep(i, j) += ep;
      CPPUNIT_ASSERT_DOUBLES_EQUAL((mep.det() - det)/ep, deriv(i, j), ep);
      mep(i, j) -= ep;
    }

  det = _unsymmetric1.det();
  deriv = _unsymmetric1.ddet();
  mep = _unsymmetric1;
  for (unsigned i = 0 ; i < 3 ; ++i)
    for (unsigned j = 0 ; j < 3 ; ++j)
    {
      mep(i, j) += ep;
      CPPUNIT_ASSERT_DOUBLES_EQUAL((mep.det() - det)/ep, deriv(i, j), ep);
      mep(i, j) -= ep;
    }
}


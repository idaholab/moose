//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoTensorTest.h"
#include "RankTwoScalarTools.h"
#include "RankFourTensor.h"
#include "ADReal.h"

#include "libmesh/point.h"

#include "metaphysicl/raw_type.h"

TEST_F(RankTwoTensorTest, L2norm)
{
  EXPECT_NEAR(0, _m0.L2norm(), 0.0001);
  EXPECT_NEAR(1.732051, _m1.L2norm(), 0.0001);
  EXPECT_NEAR(3.741657, _m2.L2norm(), 0.0001);
  EXPECT_NEAR(14.31782, _m3.L2norm(), 0.0001);
  EXPECT_NEAR(16.88194, _unsymmetric0.L2norm(), 0.0001);
}

TEST_F(RankTwoTensorTest, addIa)
{
  RankTwoTensor m(1, 2, 3, 4, 5, 6, 7, 8, 9);
  m.addIa(1.23);
  RankTwoTensor n(2.23, 2, 3, 4, 6.23, 6, 7, 8, 10.23);
  EXPECT_NEAR(0, (m - n).L2norm(), 0.0001);
}

TEST_F(RankTwoTensorTest, transpose)
{
  EXPECT_NEAR(0, (_m0.transpose() - _m0).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_m1.transpose() - _m1).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_m2.transpose() - _m2).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_m3.transpose() - _m3).L2norm(), 0.0001);
  RankTwoTensor t(1, -4, 7, 2, -5, 8, 3, -6, 9);
  EXPECT_NEAR(0, (_unsymmetric0.transpose() - t).L2norm(), 0.0001);
}

TEST_F(RankTwoTensorTest, doubleContraction)
{
  EXPECT_NEAR(121, _m3.doubleContraction(_unsymmetric0), 0.0001);
}

TEST_F(RankTwoTensorTest, rotate)
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
  RankTwoTensor rot = rot0 * rot1 * rot2;

  RankTwoTensor answer;
  RankTwoTensor m3;

  // the following "answer"s come from mathematica of course!

  // rotate about "0" axis with RealTensorValue, then back again with RankTwoTensor
  m3 = _m3;
  answer = RankTwoTensor(-4, 3, 6.363961, 3, 0, -2.1213403, 6.363961, -2.1213403, 9);
  m3.rotate(rtv0);
  EXPECT_NEAR(0, (m3 - answer).L2norm(), 0.0001);
  m3.rotate(rot0T);
  EXPECT_NEAR(0, (m3 - _m3).L2norm(), 0.0001);

  // rotate about "1" axis with RealTensorValue, then back again with RankTwoTensor
  m3 = _m3;
  answer = RankTwoTensor(2, 5.656854, -4, 5.656854, -5, -2.828427, -4, -2.828427, 8);
  m3.rotate(rtv1);
  EXPECT_NEAR(0, (m3 - answer).L2norm(), 0.0001);
  m3.rotate(rot1T);
  EXPECT_NEAR(0, (m3 - _m3).L2norm(), 0.0001);

  // rotate about "2" axis with RealTensorValue, then back again with RankTwoTensor
  m3 = _m3;
  answer = RankTwoTensor(1, -sqrt2, 3.5355339, -sqrt2, 8, -7, 3.5355339, -7, -4);
  m3.rotate(rtv2);
  EXPECT_NEAR(0, (m3 - answer).L2norm(), 0.0001);
  m3.rotate(rot2T);
  EXPECT_NEAR(0, (m3 - _m3).L2norm(), 0.0001);

  // rotate with "rot"
  m3 = _m3;
  answer = RankTwoTensor(-2.9675144,
                         -6.51776695,
                         5.6213203,
                         -6.51776695,
                         5.9319805,
                         -2.0857864,
                         5.6213203,
                         -2.0857864,
                         2.0355339);
  m3.rotate(rot);
  EXPECT_NEAR(0, (m3 - answer).L2norm(), 0.0001);
}

TEST_F(RankTwoTensorTest, trace)
{
  EXPECT_NEAR(0, _m0.tr(), 0.0001);
  EXPECT_NEAR(3, _m1.tr(), 0.0001);
  EXPECT_NEAR(6, _m2.tr(), 0.0001);
  EXPECT_NEAR(5, _m3.tr(), 0.0001);
  EXPECT_NEAR(5, _unsymmetric0.tr(), 0.0001);
  EXPECT_NEAR(6, _unsymmetric1.tr(), 0.0001);
}

TEST_F(RankTwoTensorTest, secondInvariant)
{
  EXPECT_NEAR(0, _m0.secondInvariant(), 0.0001);
  EXPECT_NEAR(0, _m1.secondInvariant(), 0.0001);
  EXPECT_NEAR(1, _m2.secondInvariant(), 0.0001);
  EXPECT_NEAR(84, _unsymmetric1.secondInvariant(), 0.0001);
}

TEST_F(RankTwoTensorTest, det)
{
  EXPECT_NEAR(0, _m0.det(), 0.0001);
  EXPECT_NEAR(1, _m1.det(), 0.0001);
  EXPECT_NEAR(6, _m2.det(), 0.0001);
  EXPECT_NEAR(-144, _m3.det(), 0.0001);
  EXPECT_NEAR(0, _unsymmetric0.det(), 0.0001);
  EXPECT_NEAR(3, _unsymmetric1.det(), 0.0001);
}

TEST_F(RankTwoTensorTest, deviatoric)
{
  RankTwoTensor dev(-1, 2, 3, -4, -7, -6, 7, 8, 8);
  EXPECT_NEAR(0, (dev - _unsymmetric1.deviatoric()).L2norm(), 0.0001);
}

TEST_F(RankTwoTensorTest, inverse)
{
  EXPECT_NEAR(0, (_m3 * _m3.inverse() - _m1).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_unsymmetric1 * _unsymmetric1.inverse() - _m1).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_unsymmetric1.inverse() * _unsymmetric1 - _m1).L2norm(), 0.0001);
}

TEST_F(RankTwoTensorTest, dtrace)
{
  EXPECT_NEAR(0, (_m0.dtrace() - _m1).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_m3.dtrace() - _m1).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_unsymmetric0.dtrace() - _m1).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_unsymmetric1.dtrace() - _m1).L2norm(), 0.0001);
}

TEST_F(RankTwoTensorTest, dsecondInvariant)
{
  EXPECT_NEAR(0, (_m0.dsecondInvariant() - _m0.deviatoric()).L2norm(), 0.0001);
  EXPECT_NEAR(0, (_m3.dsecondInvariant() - _m3.deviatoric()).L2norm(), 0.0001);
  EXPECT_NEAR(0,
              (_unsymmetric0.dsecondInvariant() -
               0.5 * (_unsymmetric0.deviatoric() + _unsymmetric0.deviatoric().transpose()))
                  .L2norm(),
              0.0001);
  EXPECT_NEAR(0,
              (_unsymmetric1.dsecondInvariant() -
               0.5 * (_unsymmetric1.deviatoric() + _unsymmetric1.deviatoric().transpose()))
                  .L2norm(),
              0.0001);
}

TEST_F(RankTwoTensorTest, d2secondInvariant)
{
  // Here i do a finite-difference calculation of the second
  // derivative and compare with the closed-solution form
  Real ep = 1E-5; // small finite-difference parameter

  RankTwoTensor d1;   // first derivative of secondInvariant - from RankTwoTensor - do a
                      // finite-difference of this
  RankFourTensor d2;  // second derivative of second Invariant - from RankTwoTensor
  RankTwoTensor mep;  // matrix with shifted entries
  RankTwoTensor d1ep; // first derivative of secondInvariant of mep

  mep = _m3;
  d1 = _m3.dsecondInvariant();
  d2 = _m3.d2secondInvariant();
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 3; j++)
    {
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
        {
          mep(k, l) += ep;
          d1ep = mep.dsecondInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;
        }
    }

  mep = _unsymmetric1;
  d1 = _unsymmetric1.dsecondInvariant();
  d2 = _unsymmetric1.d2secondInvariant();
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 3; j++)
    {
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
        {
          mep(k, l) += ep;
          d1ep = mep.dsecondInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;

          // note that because d1 and d2 explicitly symmeterise the matrix
          // the derivative may or may not explicitly symmeterise
          mep(k, l) += 0.5 * ep;
          mep(l, k) += 0.5 * ep;
          d1ep = mep.dsecondInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= 0.5 * ep;
          mep(l, k) -= 0.5 * ep;
        }
    }
}

TEST_F(RankTwoTensorTest, thirdInvariant)
{
  EXPECT_NEAR(214, _unsymmetric1.thirdInvariant(), 0.0001);
}

TEST_F(RankTwoTensorTest, dthirdInvariant)
{
  // this derivative is less trivial
  // so let's check with a finite-difference approximation
  Real ep = 1E-5;      // small finite-difference parameter
  Real thirdInvariant; // thirdInvariant provided by RankTwoTensor
  RankTwoTensor deriv; // derivative of thirdInvariant provided by RankTwoTensor
  RankTwoTensor mep;   // the RankTwoTensor with successive entries shifted by ep

  thirdInvariant = _m3.thirdInvariant();
  deriv = _m3.dthirdInvariant();
  mep = _m3;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.thirdInvariant() - thirdInvariant) / ep, deriv(i, j), 10 * ep);
      mep(i, j) -= ep;
    }

  thirdInvariant = _unsymmetric1.thirdInvariant();
  deriv = _unsymmetric1.dthirdInvariant();
  mep = _unsymmetric1;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.thirdInvariant() - thirdInvariant) / ep, deriv(i, j), 10 * ep);
      mep(i, j) -= ep;

      // since thirdInvariant is explicitly symmeterised, we can also do
      mep(i, j) += 0.5 * ep;
      mep(j, i) += 0.5 * ep;
      EXPECT_NEAR((mep.thirdInvariant() - thirdInvariant) / ep, deriv(i, j), 10 * ep);
      mep(i, j) -= 0.5 * ep;
      mep(j, i) -= 0.5 * ep;
    }
}

TEST_F(RankTwoTensorTest, d2thirdInvariant)
{
  // Here i do a finite-difference calculation of the third
  // derivative and compare with the closed-solution form
  Real ep = 1E-5; // small finite-difference parameter

  RankTwoTensor d1;   // first derivative of thirdInvariant - from RankTwoTensor - do a
                      // finite-difference of this
  RankFourTensor d2;  // third derivative of third Invariant - from RankTwoTensor
  RankTwoTensor mep;  // matrix with shifted entries
  RankTwoTensor d1ep; // first derivative of thirdInvariant of mep

  mep = _m3;
  d1 = _m3.dthirdInvariant();
  d2 = _m3.d2thirdInvariant();
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 3; j++)
    {
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
        {
          mep(k, l) += ep;
          d1ep = mep.dthirdInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;
        }
    }

  mep = _unsymmetric1;
  d1 = _unsymmetric1.dthirdInvariant();
  d2 = _unsymmetric1.d2thirdInvariant();
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 3; j++)
    {
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
        {
          mep(k, l) += ep;
          d1ep = mep.dthirdInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;

          // note that because d1 and d2 explicitly symmeterise the matrix
          // the derivative may or may not explicitly symmeterise
          mep(k, l) += 0.5 * ep;
          mep(l, k) += 0.5 * ep;
          d1ep = mep.dthirdInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= 0.5 * ep;
          mep(l, k) -= 0.5 * ep;
        }
    }
}

TEST_F(RankTwoTensorTest, sin3Lode)
{
  EXPECT_NEAR(-0.72218212, _unsymmetric1.sin3Lode(0, 0), 0.0001);
}

TEST_F(RankTwoTensorTest, dsin3Lode)
{
  // this derivative is less trivial
  // so let's check with a finite-difference approximation
  Real ep = 1E-5;      // small finite-difference parameter
  Real sin3Lode;       // sin3Lode provided by RankTwoTensor
  RankTwoTensor deriv; // derivative of sin3Lode provided by RankTwoTensor
  RankTwoTensor mep;   // the RankTwoTensor with successive entries shifted by ep

  sin3Lode = _m3.sin3Lode(0, 0);
  deriv = _m3.dsin3Lode(0);
  mep = _m3;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.sin3Lode(0, 0) - sin3Lode) / ep, deriv(i, j), 10 * ep);
      mep(i, j) -= ep;
    }

  sin3Lode = _unsymmetric1.sin3Lode(0, 0);
  deriv = _unsymmetric1.dsin3Lode(0);
  mep = _unsymmetric1;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.sin3Lode(0, 0) - sin3Lode) / ep, deriv(i, j), 10 * ep);
      mep(i, j) -= ep;

      // since sin3Lode is explicitly symmeterised, we can also do
      mep(i, j) += 0.5 * ep;
      mep(j, i) += 0.5 * ep;
      EXPECT_NEAR((mep.sin3Lode(0, 0) - sin3Lode) / ep, deriv(i, j), 10 * ep);
      mep(i, j) -= 0.5 * ep;
      mep(j, i) -= 0.5 * ep;
    }
}

TEST_F(RankTwoTensorTest, d2sin3Lode)
{
  // Here i do a finite-difference calculation of the third
  // derivative and compare with the closed-solution form
  Real ep = 1E-5; // small finite-difference parameter

  RankTwoTensor
      d1; // first derivative of sin3Lode - from RankTwoTensor - do a finite-difference of this
  RankFourTensor d2;  // third derivative of third Invariant - from RankTwoTensor
  RankTwoTensor mep;  // matrix with shifted entries
  RankTwoTensor d1ep; // first derivative of sin3Lode of mep

  mep = _m3;
  d1 = _m3.dsin3Lode(0);
  d2 = _m3.d2sin3Lode(0);
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 3; j++)
    {
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
        {
          mep(k, l) += ep;
          d1ep = mep.dsin3Lode(0);
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;
        }
    }

  mep = _unsymmetric1;
  d1 = _unsymmetric1.dsin3Lode(0);
  d2 = _unsymmetric1.d2sin3Lode(0);
  for (unsigned i = 0; i < 3; i++)
    for (unsigned j = 0; j < 3; j++)
    {
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
        {
          mep(k, l) += ep;
          d1ep = mep.dsin3Lode(0);
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;

          // note that because d1 and d2 explicitly symmeterise the matrix
          // the derivative may or may not explicitly symmeterise
          mep(k, l) += 0.5 * ep;
          mep(l, k) += 0.5 * ep;
          d1ep = mep.dsin3Lode(0);
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= 0.5 * ep;
          mep(l, k) -= 0.5 * ep;
        }
    }
}

TEST_F(RankTwoTensorTest, ddet)
{
  // this derivative is less trivial than dtrace and dsecondInvariant,
  // so let's check with a finite-difference approximation
  Real ep = 1E-5;      // small finite-difference parameter
  Real det;            // determinant provided by RankTwoTensor
  RankTwoTensor deriv; // derivative of det provided by RankTwoTensor
  RankTwoTensor mep;   // the RankTwoTensor with successive entries shifted by ep

  det = _m3.det();
  deriv = _m3.ddet();
  mep = _m3;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.det() - det) / ep, deriv(i, j), ep);
      mep(i, j) -= ep;
    }

  det = _unsymmetric1.det();
  deriv = _unsymmetric1.ddet();
  mep = _unsymmetric1;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.det() - det) / ep, deriv(i, j), ep);
      mep(i, j) -= ep;
    }
}

TEST_F(RankTwoTensorTest, initialContraction)
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

  const RankTwoTensor ic = _unsymmetric1.initialContraction(a);
  const RankTwoTensor ic1 = a.transposeMajor() * _unsymmetric1;
  EXPECT_NEAR(0.0, (ic - ic1).L2norm(), 0.0001);
}

TEST_F(RankTwoTensorTest, ErrorComputingEV)
{
  // generate tensor that does not allow computation of eigenvectors
  // and fails because of nan entries

  RankTwoTensor a;
  a(0, 0) = a(0, 1) = a(0, 2) = std::numeric_limits<double>::quiet_NaN();
  a(1, 0) = a(1, 1) = a(1, 2) = a(0, 0);
  a(2, 0) = a(2, 1) = a(2, 2) = a(0, 0);

  try
  {
    Point p(1, 0, 0);
    RankTwoScalarTools::calcEigenValuesEigenVectors(a, 0, p);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("In computing the eigenvalues and eigenvectors");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST_F(RankTwoTensorTest, HessenbergTransformation)
{
  RankTwoTensor a(1, 3, 4, 2, 5, 12, 3, 6, 1);
  RankTwoTensor H, U;
  a.hessenberg(H, U);

  EXPECT_NEAR(1.0, H(0, 0), 0.0001);
  EXPECT_NEAR(5.0, H(1, 0), 0.0001);
  EXPECT_NEAR(0.0, H(2, 0), 0.0001);
  EXPECT_NEAR(3.6, H(0, 1), 0.0001);
  EXPECT_NEAR(11.08, H(1, 1), 0.0001);
  EXPECT_NEAR(-1.44, H(2, 1), 0.0001);
  EXPECT_NEAR(0.2, H(0, 2), 0.0001);
  EXPECT_NEAR(-7.44, H(1, 2), 0.0001);
  EXPECT_NEAR(-5.08, H(2, 2), 0.0001);

  EXPECT_NEAR(1.0, U(0, 0), 0.0001);
  EXPECT_NEAR(0.0, U(1, 0), 0.0001);
  EXPECT_NEAR(0.0, U(2, 0), 0.0001);
  EXPECT_NEAR(0.0, U(0, 1), 0.0001);
  EXPECT_NEAR(0.6, U(1, 1), 0.0001);
  EXPECT_NEAR(0.8, U(2, 1), 0.0001);
  EXPECT_NEAR(0.0, U(0, 2), 0.0001);
  EXPECT_NEAR(-0.8, U(1, 2), 0.0001);
  EXPECT_NEAR(0.6, U(2, 2), 0.0001);
}

TEST_F(RankTwoTensorTest, QRFactorization)
{
  RankTwoTensor a(1, 3, 4, 2, 5, 12, 3, 6, 1);
  RankTwoTensor Q, R;
  a.QR(Q, R);

  EXPECT_NEAR(0.1961, std::abs(Q(0, 0)), 0.0001);
  EXPECT_NEAR(0.5883, std::abs(Q(1, 0)), 0.0001);
  EXPECT_NEAR(0.7845, std::abs(Q(2, 0)), 0.0001);
  EXPECT_NEAR(0.1543, std::abs(Q(0, 1)), 0.0001);
  EXPECT_NEAR(0.7715, std::abs(Q(1, 1)), 0.0001);
  EXPECT_NEAR(0.6172, std::abs(Q(2, 1)), 0.0001);
  EXPECT_NEAR(0.9684, std::abs(Q(0, 2)), 0.0001);
  EXPECT_NEAR(0.2421, std::abs(Q(1, 2)), 0.0001);
  EXPECT_NEAR(0.0605, std::abs(Q(2, 2)), 0.0001);

  EXPECT_NEAR(5.099, std::abs(R(0, 0)), 0.0001);
  EXPECT_NEAR(0.0, std::abs(R(1, 0)), 0.0001);
  EXPECT_NEAR(0.0, std::abs(R(2, 0)), 0.0001);
  EXPECT_NEAR(12.7475, std::abs(R(0, 1)), 0.0001);
  EXPECT_NEAR(3.2404, std::abs(R(1, 1)), 0.0001);
  EXPECT_NEAR(0.0, std::abs(R(2, 1)), 0.0001);
  EXPECT_NEAR(4.9029, std::abs(R(0, 2)), 0.0001);
  EXPECT_NEAR(4.4748, std::abs(R(1, 2)), 0.0001);
  EXPECT_NEAR(1.392, std::abs(R(2, 2)), 0.0001);
}

TEST_F(RankTwoTensorTest, ADConversion)
{
  RankTwoTensor reg;
  ADRankTwoTensor ad;

  ad = reg;
  reg = MetaPhysicL::raw_value(ad);

  GenericRankTwoTensor<false> generic_reg;
  GenericRankTwoTensor<true> generic_ad;

  generic_ad = generic_reg;
  generic_reg = MetaPhysicL::raw_value(generic_ad);
}

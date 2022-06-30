//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoTensorTest.h"
#include "RankFourTensor.h"
#include "MooseTypes.h"
#include "ADReal.h"

#include "libmesh/point.h"
#include "libmesh/int_range.h"

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

  // the following "answer"s come from Mathematica of course!

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
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          mep(k, l) += ep;
          d1ep = mep.dsecondInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;
        }

  mep = _unsymmetric1;
  d1 = _unsymmetric1.dsecondInvariant();
  d2 = _unsymmetric1.d2secondInvariant();
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          mep(k, l) += ep;
          d1ep = mep.dsecondInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;

          // note that because d1 and d2 explicitly symmetrise the matrix
          // the derivative may or may not explicitly symmetrise
          mep(k, l) += 0.5 * ep;
          mep(l, k) += 0.5 * ep;
          d1ep = mep.dsecondInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= 0.5 * ep;
          mep(l, k) -= 0.5 * ep;
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
  for (auto i : make_range(N))
    for (auto j : make_range(N))
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.thirdInvariant() - thirdInvariant) / ep, deriv(i, j), 10 * ep);
      mep(i, j) -= ep;
    }

  thirdInvariant = _unsymmetric1.thirdInvariant();
  deriv = _unsymmetric1.dthirdInvariant();
  mep = _unsymmetric1;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
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
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
        {
          mep(k, l) += ep;
          d1ep = mep.dthirdInvariant();
          EXPECT_NEAR((d1ep(i, j) - d1(i, j)) / ep, d2(i, j, k, l), ep);
          mep(k, l) -= ep;
        }

  mep = _unsymmetric1;
  d1 = _unsymmetric1.dthirdInvariant();
  d2 = _unsymmetric1.d2thirdInvariant();
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
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
  for (auto i : make_range(N))
    for (auto j : make_range(N))
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.sin3Lode(0, 0) - sin3Lode) / ep, deriv(i, j), 10 * ep);
      mep(i, j) -= ep;
    }

  sin3Lode = _unsymmetric1.sin3Lode(0, 0);
  deriv = _unsymmetric1.dsin3Lode(0);
  mep = _unsymmetric1;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
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
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
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
  for (auto i : make_range(N))
    for (auto j : make_range(N))
    {
      mep(i, j) += ep;
      EXPECT_NEAR((mep.det() - det) / ep, deriv(i, j), ep);
      mep(i, j) -= ep;
    }

  det = _unsymmetric1.det();
  deriv = _unsymmetric1.ddet();
  mep = _unsymmetric1;
  for (auto i : make_range(N))
    for (auto j : make_range(N))
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

TEST_F(RankTwoTensorTest, timesIMMJKL)
{
  usingTensorIndices(i_, j_, k_, l_, m_);
  const RankFourTensor computed_val = _unsymmetric1.times<i_, m_, m_, j_, k_, l_>(_r4);
  RankFourTensor expected_val;
  expected_val.fillFromInputVector(
      {274, 278, 282, 286, 290, 294, 298, 302, 306, 310, 314, 318, 322, 326, 330, 334, 338,
       342, 346, 350, 354, 358, 362, 366, 370, 374, 378, 302, 307, 312, 317, 322, 327, 332,
       337, 342, 347, 352, 357, 362, 367, 372, 377, 382, 387, 392, 397, 402, 407, 412, 417,
       422, 427, 432, 385, 392, 399, 406, 413, 420, 427, 434, 441, 448, 455, 462, 469, 476,
       483, 490, 497, 504, 511, 518, 525, 532, 539, 546, 553, 560, 567},
      RankFourTensor::general);

  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
          EXPECT_NEAR(expected_val(i, j, k, l), computed_val(i, j, k, l), 1e-5);
}

TEST_F(RankTwoTensorTest, timesMLIMJK)
{
  usingTensorIndices(i_, j_, k_, l_, m_);
  const RankFourTensor computed_val = _unsymmetric1.times<m_, l_, i_, m_, j_, k_>(_r4);
  RankFourTensor expected_val;
  expected_val.fillFromInputVector(
      {78,    -168, 277,  84,    -183, 302,   90,    -198, 327,   96,    -213, 352,   102,  -228,
       377,   108,  -243, 402,   114,  -258,  427,   120,  -273,  452,   126,  -288,  477,  240,
       -573,  952,  246,  -588,  977,  252,   -603,  1002, 258,   -618,  1027, 264,   -633, 1052,
       270,   -648, 1077, 276,   -663, 1102,  282,   -678, 1127,  288,   -693, 1152,  402,  -978,
       1627,  408,  -993, 1652,  414,  -1008, 1677,  420,  -1023, 1702,  426,  -1038, 1727, 432,
       -1053, 1752, 438,  -1068, 1777, 444,   -1083, 1802, 450,   -1098, 1827},
      RankFourTensor::general);

  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
          EXPECT_EQ(expected_val(i, j, k, l), computed_val(i, j, k, l));
}

TEST_F(RankTwoTensorTest, timesMJIMKL)
{
  usingTensorIndices(i_, j_, k_, l_, m_);
  const RankFourTensor computed_val = _unsymmetric1.times<m_, j_, i_, m_, k_, l_>(_r4);
  RankFourTensor expected_val;
  expected_val.fillFromInputVector(
      {78,    84,    90,   96,   102,  108,  114,  120,  126,  -168,  -183,  -198,  -213,  -228,
       -243,  -258,  -273, -288, 277,  302,  327,  352,  377,  402,   427,   452,   477,   240,
       246,   252,   258,  264,  270,  276,  282,  288,  -573, -588,  -603,  -618,  -633,  -648,
       -663,  -678,  -693, 952,  977,  1002, 1027, 1052, 1077, 1102,  1127,  1152,  402,   408,
       414,   420,   426,  432,  438,  444,  450,  -978, -993, -1008, -1023, -1038, -1053, -1068,
       -1083, -1098, 1627, 1652, 1677, 1702, 1727, 1752, 1777, 1802,  1827},
      RankFourTensor::general);

  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        for (auto l : make_range(N))
          EXPECT_EQ(expected_val(i, j, k, l), computed_val(i, j, k, l));
}

TEST_F(RankTwoTensorTest, contraction)
{
  const RankThreeTensor computed_val = _unsymmetric1.contraction(_r3);
  RankThreeTensor expected_val;
  expected_val.fillFromInputVector({94,  98,  102, 106, 110, 114, 118, 122, 126,
                                    104, 109, 114, 119, 124, 129, 134, 139, 144,
                                    133, 140, 147, 154, 161, 168, 175, 182, 189},
                                   RankThreeTensor::general);
  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        EXPECT_EQ(expected_val(i, j, k), computed_val(i, j, k));
}

TEST_F(RankTwoTensorTest, mixedProductJkI)
{
  const RankThreeTensor computed_val = _unsymmetric1.mixedProductJkI(_v);
  RankThreeTensor expected_val;
  expected_val.fillFromInputVector({1,  -4, 7,   2,  -5, 8,   3,  -6, 10,  2,  -8, 14,  4, -10,
                                    16, 6,  -12, 20, 3,  -12, 21, 6,  -15, 24, 9,  -18, 30},
                                   RankThreeTensor::general);

  for (auto i : make_range(N))
    for (auto j : make_range(N))
      for (auto k : make_range(N))
        EXPECT_EQ(expected_val(i, j, k), computed_val(i, j, k));
}

TEST_F(RankTwoTensorTest, initializeSymmetric)
{
  auto sym1 = RankTwoTensor::initializeSymmetric(
      RealVectorValue(1, 2, 3), RealVectorValue(4, 5, 6), RealVectorValue(7, 8, 9));

  // 1 2 3   1 4 7       1 3 5
  // 4 5 6 + 2 5 8 = 2 * 3 5 7
  // 7 8 9   3 6 9       5 7 9

  auto sym2 = RankTwoTensor::initializeFromRows(
      RealVectorValue(1, 3, 5), RealVectorValue(3, 5, 7), RealVectorValue(5, 7, 9));

  for (auto i : make_range(N))
    for (auto j : make_range(N))
      EXPECT_EQ(sym1(i, j), sym2(i, j));
}

TEST_F(RankTwoTensorTest, row)
{
  EXPECT_EQ(_unsymmetric0.row(0) * _v, 14);
  EXPECT_EQ(_unsymmetric0.row(1) * _v, 16);
  EXPECT_EQ(_unsymmetric0.row(2) * _v, 18);
}

TEST_F(RankTwoTensorTest, timesTranspose)
{
  auto A = RankTwoTensor::timesTranspose(_unsymmetric0);
  EXPECT_NEAR(0, (A - (_unsymmetric0 * _unsymmetric0.transpose())).L2norm(), 1e-9);
}

TEST_F(RankTwoTensorTest, plusTranspose)
{
  auto A = RankTwoTensor::plusTranspose(_unsymmetric0);
  EXPECT_NEAR(0, (A - (_unsymmetric0 + _unsymmetric0.transpose())).L2norm(), 1e-9);
}

TEST_F(RankTwoTensorTest, sqr)
{
  auto B = RankTwoTensor(14, 16, 18, -26, -31, -36, 38, 46, 54);
  EXPECT_NEAR(0, (_unsymmetric0.square() - B).L2norm(), 1e-9);
}

TEST_F(RankTwoTensorTest, vectorOuterProduct)
{
  RealVectorValue v2(5, -4, 7);
  auto A = RankTwoTensor::outerProduct(v2, _v);
  auto B = RankTwoTensor(5, -4, 7, 10, -8, 14, 15, -12, 21);
  EXPECT_EQ((A - B).L2norm(), 0);
}

TEST_F(RankTwoTensorTest, outerProduct)
{
  RealVectorValue v2(5, -4, 7);
  auto A = RankTwoTensor::outerProduct(v2, _v);
  auto B = RankTwoTensor(5, -4, 7, 10, -8, 14, 15, -12, 21);
  EXPECT_EQ((A - B).L2norm(), 0);
}

TEST_F(RankTwoTensorTest, selfOuterProduct)
{
  auto A = RankTwoTensor::selfOuterProduct(_v);
  auto B = RankTwoTensor(1, 2, 3, 2, 4, 6, 3, 6, 9);
  EXPECT_EQ((A - B).L2norm(), 0);
}

TEST_F(RankTwoTensorTest, fillRealTensor)
{
  RankTwoTensor A(1, 2, 3, 4, 5, 6, 7, 8, 9);
  RealTensorValue B;
  A.fillRealTensor(B);
  RankTwoTensor C = B;
  EXPECT_EQ((A - C).L2norm(), 0);
}

TEST_F(RankTwoTensorTest, fillColumn)
{
  RankTwoTensor A;
  A.fillColumn(1, _v);
  auto B = RankTwoTensor(0, 0, 0, 1, 2, 3, 0, 0, 0);
  EXPECT_EQ((A - B).L2norm(), 0);
}

TEST_F(RankTwoTensorTest, fillRow)
{
  RankTwoTensor A;
  A.fillRow(1, _v);
  auto B = RankTwoTensor(0, 1, 0, 0, 2, 0, 0, 3, 0);
  EXPECT_EQ((A - B).L2norm(), 0);
}

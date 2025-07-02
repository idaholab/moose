//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "BSpline.h"
#include "libmesh/point.h"

// using namespace libMesh;

//* The following tests check that the BSpline utility and its submethods work as intended.
//* Degrees 2, 3, and 4 interpolating polynomials will be evaluated at prescribed values of
//* t, the parameter.
//*
//* All expected points were independently calculated and verified.

TEST(BSplineTest, CP_MakerDegree2Test)
{
  const unsigned int degree = 2;

  const libMesh::Point start(1, 2, 1);
  const libMesh::Point end(2, 1, 0);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const double sharpness = 0.5;
  const unsigned int cps_per_half = 2;

  const libMesh::Real t0 = 0;
  const libMesh::Real t1 = 0.4;
  const libMesh::Real t2 = 0.8;
  const libMesh::Real t3 = 1.0;

  /// for reference:
  // const std::vector<libMesh::Point> control_points = {
  //     {1, 2, 1}, {1.5, 2, 1}, {2, 2, 0.5}, {2, 1.5, 0}, {2, 1, 0}};

  Moose::BSpline b_spline(degree, start, end, direction1, direction2, cps_per_half, sharpness);

  const libMesh::Point point_0 = b_spline.getPoint(t0);
  const libMesh::Point point_1 = b_spline.getPoint(t1);
  const libMesh::Point point_2 = b_spline.getPoint(t2);
  const libMesh::Point point_3 = b_spline.getPoint(t3);

  const libMesh::Point expected_point_0(1, 2, 1);
  const libMesh::Point expected_point_1(1.84, 1.99, 0.65);
  const libMesh::Point expected_point_2(2., 1.51, 0.09);
  const libMesh::Point expected_point_3(2, 1, 0);

  EXPECT_NEAR((point_0 - expected_point_0).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_1 - expected_point_1).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_2 - expected_point_2).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_3 - expected_point_3).norm(), 0, libMesh::TOLERANCE);
}

TEST(BSplineTest, CP_MakerDegree3Test)
{
  const unsigned int degree = 3;

  const libMesh::Point start(1, 2, 1);
  const libMesh::Point end(2, 1, 0);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const double sharpness = 0.5;
  const unsigned int cps_per_half = 2;

  const libMesh::Real t0 = 0;
  const libMesh::Real t1 = 0.4;
  const libMesh::Real t2 = 0.8;
  const libMesh::Real t3 = 1.0;

  /// for reference:
  // const std::vector<libMesh::Point> control_points = {
  //     {1, 2, 1}, {1.5, 2, 1}, {2, 2, 0.5}, {2, 1.5, 0}, {2, 1, 0}};

  Moose::BSpline b_spline(degree, start, end, direction1, direction2, cps_per_half, sharpness);
  const libMesh::Point point_0 = b_spline.getPoint(t0);
  const libMesh::Point point_1 = b_spline.getPoint(t1);
  const libMesh::Point point_2 = b_spline.getPoint(t2);
  const libMesh::Point point_3 = b_spline.getPoint(t3);

  const libMesh::Point expected_point_0(1, 2, 1);
  const libMesh::Point expected_point_1(1.784, 1.936, 0.648);
  const libMesh::Point expected_point_2(1.992, 1.488, 0.104);
  const libMesh::Point expected_point_3(2, 1, 0);

  EXPECT_NEAR((point_0 - expected_point_0).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_1 - expected_point_1).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_2 - expected_point_2).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_3 - expected_point_3).norm(), 0, libMesh::TOLERANCE);
}

TEST(BSplineTest, CP_MakerDegree4Test)
{
  const unsigned int degree = 4;

  const libMesh::Point start(1, 2, 1);
  const libMesh::Point end(2, 1, 0);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const double sharpness = 0.5;
  const unsigned int cps_per_half = 2;

  const libMesh::Real t0 = 0;
  const libMesh::Real t1 = 0.4;
  const libMesh::Real t2 = 0.8;
  const libMesh::Real t3 = 1.0;

  /// for reference:
  // const std::vector<libMesh::Point> control_points = {
  //     {1, 2, 1}, {1.5, 2, 1}, {2, 2, 0.5}, {2, 1.5, 0}, {2, 1, 0}};

  Moose::BSpline b_spline(degree, start, end, direction1, direction2, cps_per_half, sharpness);

  const libMesh::Point point_0 = b_spline.getPoint(t0);
  const libMesh::Point point_1 = b_spline.getPoint(t1);
  const libMesh::Point point_2 = b_spline.getPoint(t2);
  const libMesh::Point point_3 = b_spline.getPoint(t3);

  const libMesh::Point expected_point_0(1, 2, 1);
  const libMesh::Point expected_point_1(1.6976, 1.8976, 0.648);
  const libMesh::Point expected_point_2(1.9856, 1.3856, 0.104);
  const libMesh::Point expected_point_3(2, 1, 0);

  EXPECT_NEAR((point_0 - expected_point_0).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_1 - expected_point_1).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_2 - expected_point_2).norm(), 0, libMesh::TOLERANCE);
  EXPECT_NEAR((point_3 - expected_point_3).norm(), 0, libMesh::TOLERANCE);
}

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

//* The following tests check that the BSpline utility and its submethods work as intended.
//* Degrees 2, 3, and 4 interpolating polynomials will be evaluated at prescribed values of
//* t, the parameter. In addition, a vector of control points is supplied.
//*
//* All expected points were independently calculated and verified.

const Real t0 = 0;
const Real t1 = 0.4;
const Real t2 = 0.8;
const Real t3 = 1.0;
const std::vector<libMesh::Point> control_points = {
    {1, 2, 0}, {1, 2, 0}, {1.5, 2.5, 0}, {3, 1, 0}, {3, 1, 0}};

TEST(BSplineTest, Degree2Test)
{
  const unsigned int degree = 2;

  Moose::BSpline b_spline(degree, control_points);

  const libMesh::Point point_0 = b_spline.getPoint(t0);
  const libMesh::Point point_1 = b_spline.getPoint(t1);
  const libMesh::Point point_2 = b_spline.getPoint(t2);
  const libMesh::Point point_3 = b_spline.getPoint(t3);

  const libMesh::Point expected_point_0(1, 2, 0);
  const libMesh::Point expected_point_1(1.37, 2.31, 0);
  const libMesh::Point expected_point_2(2.73, 1.2699999999999996, 0);
  const libMesh::Point expected_point_3(3, 1, 0);

  EXPECT_NEAR((point_0 - expected_point_0).norm(), 0, 1e-9);
  EXPECT_NEAR((point_1 - expected_point_1).norm(), 0, 1e-9);
  EXPECT_NEAR((point_2 - expected_point_2).norm(), 0, 1e-9);
  EXPECT_NEAR((point_3 - expected_point_3).norm(), 0, 1e-9);
}

TEST(BSplineTest, Degree3Test)
{
  const unsigned int degree = 3;

  Moose::BSpline b_spline(degree, control_points);

  const libMesh::Point point_0 = b_spline.getPoint(t0);
  const libMesh::Point point_1 = b_spline.getPoint(t1);
  const libMesh::Point point_2 = b_spline.getPoint(t2);
  const libMesh::Point point_3 = b_spline.getPoint(t3);

  const libMesh::Point expected_point_0(1, 2, 0);
  const libMesh::Point expected_point_1(1.48, 2.0959999999999996, 0);
  const libMesh::Point expected_point_2(2.7039999999999997, 1.2799999999999998, 0);
  const libMesh::Point expected_point_3(3, 1, 0);

  EXPECT_NEAR((point_0 - expected_point_0).norm(), 0, 1e-9);
  EXPECT_NEAR((point_1 - expected_point_1).norm(), 0, 1e-9);
  EXPECT_NEAR((point_2 - expected_point_2).norm(), 0, 1e-9);
  EXPECT_NEAR((point_3 - expected_point_3).norm(), 0, 1e-9);
}

TEST(BSplineTest, Degree4Test)
{
  const unsigned int degree = 4;

  Moose::BSpline b_spline(degree, control_points);

  const libMesh::Point point_0 = b_spline.getPoint(t0);
  const libMesh::Point point_1 = b_spline.getPoint(t1);
  const libMesh::Point point_2 = b_spline.getPoint(t2);
  const libMesh::Point point_3 = b_spline.getPoint(t3);

  const libMesh::Point expected_point_0(1, 2, 0);
  const libMesh::Point expected_point_1(1.5312, 1.9936, 0);
  const libMesh::Point expected_point_2(2.7152000000000003, 1.2576, 0);
  const libMesh::Point expected_point_3(3, 1, 0);

  EXPECT_NEAR((point_0 - expected_point_0).norm(), 0, 1e-9);
  EXPECT_NEAR((point_1 - expected_point_1).norm(), 0, 1e-9);
  EXPECT_NEAR((point_2 - expected_point_2).norm(), 0, 1e-9);
  EXPECT_NEAR((point_3 - expected_point_3).norm(), 0, 1e-9);
}

/**
 * Test for mooseError when the number of control points supplied is
 * insufficient.
 */
TEST(BSplineTest, ControlPointErrorTest)
{
  const unsigned int degree = 5;

  EXPECT_THROW(
      {
        try
        {
          Moose::BSpline b_spline(degree, control_points);
        }
        catch (const std::exception & e)
        {
          EXPECT_STREQ(e.what(),
                       "Number of control points must be greater than or equal to degree + 1!");
          throw;
        }
      },
      std::exception);
}

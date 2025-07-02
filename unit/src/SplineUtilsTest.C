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
#include "libMeshReducedNamespace.h"
#include "MooseError.h"
#include "SplineUtils.h"

TEST(SplineUtilsTest, SimpleCircularCPTest)
{
  const libMesh::Point start(0, 0, 0);
  const libMesh::Point end(1, 0, 0);
  const libMesh::RealVectorValue direction(0, 1, 0);
  const unsigned int cps = 5;
  const std::vector<Point> expected_cps = {libMesh::Point(0, 0, 0),
                                           libMesh::Point(1.46446609e-01, 3.53553391e-01, 0),
                                           libMesh::Point(0.5, 0.5, 0),
                                           libMesh::Point(8.53553391e-01, 3.53553391e-01, 0),
                                           libMesh::Point(1.0, 0, 0)};
  const std::vector<Point> evaluated_points =
      SplineUtils::circularControlPoints(start, end, direction, cps);
  for (const auto i : index_range(expected_cps))
  {
    EXPECT_NEAR((expected_cps[i] - evaluated_points[i]).norm(), 0, libMesh::TOLERANCE);
  }
}

TEST(SplineUtilsTest, CircularError)
{
  const libMesh::Point start(0, 1, 0);
  const libMesh::Point end(1, 0, 0);
  const libMesh::RealVectorValue direction(0, 1, 0);
  const unsigned int cps = 5;
  EXPECT_THROW(
      {
        try
        {
          SplineUtils::circularControlPoints(start, end, direction, cps);
        }
        catch (const std::exception & e)
        {
          EXPECT_STREQ(
              e.what(),
              "Parallel lines cannot be interpolated using points along a circle! Direction "
              "vectors are incompatible with point locations.");
          throw;
        }
      },
      std::exception);
}

TEST(SplineUtilsTest, 3DCircularCPTest)
{
  const libMesh::Point start(0, 0, 0);
  const libMesh::Point end(1, 0, 1);
  const libMesh::RealVectorValue direction(0, 1, 0);
  const unsigned int cps_per_half = 2;
  const libMesh::Real sharpness = 1.0; // not used in calculation but needed for function
  const std::vector<Point> expected_cps = {
      libMesh::Point(0, 0, 0),
      libMesh::Point(1.46446609e-01, 5.00000000e-01, 1.46446609e-01),
      libMesh::Point(5.00000000e-01, 7.07106781e-01, 5.00000000e-01),
      libMesh::Point(8.53553391e-01, 5.00000000e-01, 8.53553391e-01),
      libMesh::Point(1.00000000e+00, 8.65956056e-17, 1.00000000e+00)};
  const std::vector<Point> evaluated_points =
      SplineUtils::bSplineControlPoints(start, end, direction, direction, cps_per_half, sharpness);
  for (const auto i : index_range(expected_cps))
  {
    EXPECT_NEAR((expected_cps[i] - evaluated_points[i]).norm(), 0, libMesh::TOLERANCE);
  }
}

TEST(SplineUtilsTest, ClosestPoint2DTest)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 1, 0);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const std::vector<Point> evaluated_points =
      SplineUtils::closestPoints(start, end, direction1, direction2);
  const std::vector<Point> expected_points = {libMesh::Point(2, 2, 0), libMesh::Point(2, 2, 0)};
  for (const auto i : index_range(expected_points))
    EXPECT_NEAR((expected_points[i] - evaluated_points[i]).norm(), 0, libMesh::TOLERANCE);
}

TEST(SplineUtilsTest, ClosestPoint3DTest1)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 1, 1);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const std::vector<Point> evaluated_points =
      SplineUtils::closestPoints(start, end, direction1, direction2);
  const std::vector<Point> expected_points = {libMesh::Point(2, 2, 0), libMesh::Point(2, 2, 1)};
  for (const auto i : index_range(expected_points))
    EXPECT_NEAR((expected_points[i] - evaluated_points[i]).norm(), 0, libMesh::TOLERANCE);
}

TEST(SplineUtilsTest, ClosestPoint3DTest2)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 1, 1);
  const libMesh::RealVectorValue direction1(1, 1, 1);
  const libMesh::RealVectorValue direction2(1, 2, -1);
  const std::vector<Point> evaluated_points =
      SplineUtils::closestPoints(start, end, direction1, direction2);
  const std::vector<Point> expected_points = {libMesh::Point(1.71428571, 2.71428571, 0.71428571),
                                              libMesh::Point(2.57142857, 2.14285714, 0.42857143)};
  for (const auto i : index_range(expected_points))
    EXPECT_NEAR((expected_points[i] - evaluated_points[i]).norm(), 0, libMesh::TOLERANCE);
}

TEST(SplineUtilsTest, CPsAlongLineTest)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 2, 0);
  const libMesh::RealVectorValue direction(1, 0, 0);
  const libMesh::Real sharpness = 1.0;
  const unsigned int num_cps = 3;
  const std::vector<Point> evaluated_points =
      SplineUtils::controlPointsAlongLine(start, end, direction, sharpness, num_cps);
  const std::vector<Point> expected_points = {
      libMesh::Point(1, 2, 0), libMesh::Point(1.5, 2, 0), libMesh::Point(2, 2, 0)};
  for (const auto i : index_range(expected_points))
  {
    EXPECT_NEAR((expected_points[i] - evaluated_points[i]).norm(), 0, libMesh::TOLERANCE);
  }
}

TEST(SplineUtilsTest, CPsAlongLineOneTest)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 2, 0);
  const libMesh::RealVectorValue direction(1, 0, 0);
  const libMesh::Real sharpness = 1.0;
  const unsigned int num_cps = 1;
  const std::vector<Point> evaluated_points =
      SplineUtils::controlPointsAlongLine(start, end, direction, sharpness, num_cps);
  const std::vector<Point> expected_points = {libMesh::Point(1, 2, 0)};
  for (const auto i : index_range(expected_points))
  {
    EXPECT_NEAR((expected_points[i] - evaluated_points[i]).norm(), 0, libMesh::TOLERANCE);
  }
}

TEST(SplineUtilsTest, MakeControlPointFailTest1)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 2, 0);
  const libMesh::RealVectorValue direction(1, 0, 0);
  const libMesh::Real sharpness = -1.0;
  EXPECT_THROW(
      {
        try
        {
          SplineUtils::makeControlPoint(start, end, direction, sharpness);
        }
        catch (const std::exception & e)
        {
          EXPECT_STREQ(e.what(), "Sharpness must be in [0,1]!");
          throw;
        }
      },
      std::exception);
}

TEST(SplineUtilsTest, MakeControlPointFailTest2)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 2, 0);
  const libMesh::RealVectorValue direction(1, 0, 0);
  const libMesh::Real sharpness = 1.5;
  EXPECT_THROW(
      {
        try
        {
          SplineUtils::makeControlPoint(start, end, direction, sharpness);
        }
        catch (const std::exception & e)
        {
          EXPECT_STREQ(e.what(), "Sharpness must be in [0,1]!");
          throw;
        }
      },
      std::exception);
}

TEST(SplineUtilsTest, TrivialCPsTest)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 1, 0);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const double sharpness = 0;
  const unsigned int cps_per_half = 2;
  const std::vector<Point> evaluated_cps = SplineUtils::bSplineControlPoints(
      start, end, direction1, direction2, cps_per_half, sharpness);

  const std::vector<Point> expected_cps = {libMesh::Point(1, 2, 0),
                                           libMesh::Point(1, 2, 0),
                                           libMesh::Point(2, 2, 0),
                                           libMesh::Point(2, 1, 0),
                                           libMesh::Point(2, 1, 0)};
  for (const auto i : index_range(expected_cps))
    EXPECT_NEAR((expected_cps[i] - evaluated_cps[i]).norm(), 0, libMesh::TOLERANCE);
}

TEST(SplineUtilsTest, HalfSharpCPsTest)
{
  const libMesh::Point start(1, 2, 0);
  const libMesh::Point end(2, 1, 0);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const double sharpness = 0.5;
  const unsigned int cps_per_half = 2;
  const std::vector<Point> evaluated_cps = SplineUtils::bSplineControlPoints(
      start, end, direction1, direction2, cps_per_half, sharpness);

  const std::vector<Point> expected_cps = {libMesh::Point(1, 2, 0),
                                           libMesh::Point(1.5, 2, 0),
                                           libMesh::Point(2, 2, 0),
                                           libMesh::Point(2, 1.5, 0),
                                           libMesh::Point(2, 1, 0)};
  for (const auto i : index_range(expected_cps))
    EXPECT_NEAR((expected_cps[i] - evaluated_cps[i]).norm(), 0, libMesh::TOLERANCE);
}

TEST(SplineUtilsTest, HalfSharpCPs3DTest)
{
  const libMesh::Point start(1, 2, 1);
  const libMesh::Point end(2, 1, 0);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const double sharpness = 0.5;
  const unsigned int cps_per_half = 2;
  const std::vector<Point> evaluated_cps = SplineUtils::bSplineControlPoints(
      start, end, direction1, direction2, cps_per_half, sharpness);

  const std::vector<Point> expected_cps = {libMesh::Point(1, 2, 1),
                                           libMesh::Point(1.5, 2, 1),
                                           libMesh::Point(2, 2, 0.5),
                                           libMesh::Point(2, 1.5, 0),
                                           libMesh::Point(2, 1, 0)};
  for (const auto i : index_range(expected_cps))
    EXPECT_NEAR((expected_cps[i] - evaluated_cps[i]).norm(), 0, libMesh::TOLERANCE);
}

TEST(SplineUtilsTest, FullSharpCPs3DTest)
{
  const libMesh::Point start(1, 2, 1);
  const libMesh::Point end(2, 1, 0);
  const libMesh::RealVectorValue direction1(1, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const double sharpness = 1.0;
  const unsigned int cps_per_half = 3;
  const std::vector<Point> evaluated_cps = SplineUtils::bSplineControlPoints(
      start, end, direction1, direction2, cps_per_half, sharpness);

  const std::vector<Point> expected_cps = {libMesh::Point(1, 2, 1),
                                           libMesh::Point(1.5, 2, 1),
                                           libMesh::Point(2, 2, 1),
                                           libMesh::Point(2, 2, 0.5),
                                           libMesh::Point(2, 2, 0),
                                           libMesh::Point(2, 1.5, 0),
                                           libMesh::Point(2, 1, 0)};
  for (const auto i : index_range(expected_cps))
    EXPECT_NEAR((expected_cps[i] - evaluated_cps[i]).norm(), 0, libMesh::TOLERANCE);
}

TEST(SplineUtilsTest, NonuniquePointError)
{
  const libMesh::Point start(1, 0, 0);
  const libMesh::Point end(1, 0, 0);
  const libMesh::RealVectorValue direction1(0, 1, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const unsigned int cps_per_half = 2;
  const double sharpness = 1.0;
  EXPECT_THROW(
      {
        try
        {
          SplineUtils::bSplineControlPoints(
              start, end, direction1, direction2, cps_per_half, sharpness);
        }
        catch (const std::exception & e)
        {
          EXPECT_STREQ(e.what(), "Start and end points must be unique.");
          throw;
        }
      },
      std::exception);
}

TEST(SplineUtilsTest, ZeroDirection1Error)
{
  const libMesh::Point start(1, 0, 0);
  const libMesh::Point end(1, 1, 0);
  const libMesh::RealVectorValue direction1(0, 0, 0);
  const libMesh::RealVectorValue direction2(0, 1, 0);
  const unsigned int cps_per_half = 2;
  const double sharpness = 1.0;
  EXPECT_THROW(
      {
        try
        {
          SplineUtils::bSplineControlPoints(
              start, end, direction1, direction2, cps_per_half, sharpness);
        }
        catch (const std::exception & e)
        {
          EXPECT_STREQ(e.what(), "Direction vectors must have a nonzero magnitude.");
          throw;
        }
      },
      std::exception);
}
TEST(SplineUtilsTest, ZeroDirection2Error)
{
  const libMesh::Point start(1, 0, 0);
  const libMesh::Point end(1, 1, 0);
  const libMesh::RealVectorValue direction1(0, 1, 0);
  const libMesh::RealVectorValue direction2(0, 0, 0);
  const unsigned int cps_per_half = 2;
  const double sharpness = 1.0;
  EXPECT_THROW(
      {
        try
        {
          SplineUtils::bSplineControlPoints(
              start, end, direction1, direction2, cps_per_half, sharpness);
        }
        catch (const std::exception & e)
        {
          EXPECT_STREQ(e.what(), "Direction vectors must have a nonzero magnitude.");
          throw;
        }
      },
      std::exception);
}
TEST(SplineUtilsTest, ZeroDirectionBothError)
{
  const libMesh::Point start(1, 0, 0);
  const libMesh::Point end(1, 1, 0);
  const libMesh::RealVectorValue direction1(0, 0, 0);
  const libMesh::RealVectorValue direction2(0, 0, 0);
  const unsigned int cps_per_half = 2;
  const double sharpness = 1.0;
  EXPECT_THROW(
      {
        try
        {
          SplineUtils::bSplineControlPoints(
              start, end, direction1, direction2, cps_per_half, sharpness);
        }
        catch (const std::exception & e)
        {
          EXPECT_STREQ(e.what(), "Direction vectors must have a nonzero magnitude.");
          throw;
        }
      },
      std::exception);
}
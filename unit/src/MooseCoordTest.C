//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseCoordTransform.h"
#include "libmesh/point.h"

TEST(MooseCoordTest, test)
{
  MooseCoordTransform transform{};
  const auto x = MooseCoordTransform::X;
  const auto y = MooseCoordTransform::Y;
  const auto z = MooseCoordTransform::Z;
  const auto invalid = MooseCoordTransform::INVALID;
  const Point xpt(1, 0, 0);
  const Point ypt(0, 1, 0);
  const Point zpt(0, 0, 1);
  const Point minus_xpt(-1, 0, 0);
  const Point minus_ypt(0, -1, 0);
  const Point minus_zpt(0, 0, -1);

  auto error_checking =
      [&transform](const auto up_direction, const auto rz_z_axis, const auto & error_string)
  {
    try
    {
      transform.setRotation(up_direction, rz_z_axis);
      EXPECT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      std::string error_message(e.what());
      EXPECT_TRUE(error_message.find(error_string) != std::string::npos);
    }
  };

  auto compare_points = [](const Point & pt1, const Point & pt2)
  {
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      EXPECT_NEAR(pt1(i), pt2(i), TOLERANCE * TOLERANCE);
  };

  error_checking(x, z, "Invalid rz_coord_axis option");
  error_checking(x, invalid, "Called MooseCoordTransform::setRotation with INVALID");

  // xx
  error_checking(x, x, "Rotation yields negative radial values");
  compare_points(transform(ypt), minus_xpt);

  // zx
  error_checking(z, x, "Rotation yields negative radial values");
  compare_points(transform(ypt), minus_zpt);

  // xy
  transform.setRotation(x, y);
  compare_points(transform(xpt), ypt);

  // yx
  transform.setRotation(y, x);
  compare_points(transform(xpt), xpt);

  // yy
  transform.setRotation(y, y);
  compare_points(transform(xpt), xpt);

  // zy
  transform.setRotation(z, y);
  compare_points(transform(xpt), xpt);
  compare_points(transform(ypt), minus_zpt);

  auto error_angles_checking = [&transform](const auto alpha,
                                            const auto beta,
                                            const auto gamma,
                                            const auto rz_z_axis,
                                            const auto & error_string)
  {
    try
    {
      transform.setRotation(alpha, beta, gamma, rz_z_axis);
      EXPECT_TRUE(false);
    }
    catch (std::runtime_error & e)
    {
      std::string error_message(e.what());
      EXPECT_TRUE(error_message.find(error_string) != std::string::npos);
    }
  };

  error_angles_checking(0, 0, 0, z, "Invalid rz_coord_axis option");
  error_angles_checking(0, 0, 0, invalid, "Called MooseCoordTransform::setRotation with INVALID");
  error_angles_checking(0, 0, 0, x, "Unsupported manual angle prescription");

  transform.setRotation(0, 90, 0, x);
  compare_points(transform(ypt), zpt);
  compare_points(transform(xpt), xpt);
}

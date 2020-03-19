//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GriddedData.h"
#include "MooseException.h"

TEST(GriddedData, 2D)
{
  GriddedData gridded_data("data/gridded/twod.txt");

  // Test dimension
  EXPECT_EQ(gridded_data.getDim(), 2);

  // Test axes
  std::vector<int> axes;
  gridded_data.getAxes(axes);

  std::vector<int> axes_gold{{0, 1}};
  EXPECT_EQ(axes, axes_gold);

  // Test grid (AXES points in file)
  std::vector<std::vector<Real>> grid;
  gridded_data.getGrid(grid);

  std::vector<std::vector<Real>> grid_gold{{{-1.0, 0.0, 2.0}, {-1.0, 2.0, 3.0}}};
  EXPECT_EQ(grid, grid_gold);

  // Test function values (DATA values in file)
  std::vector<Real> fcn;
  gridded_data.getFcn(fcn);

  std::vector<Real> fcn_gold{{-4.0, -2.0, 2.0, 5.0, 7.0, 11.0, 8.0, 10.0, 14.0}};
  EXPECT_EQ(fcn, fcn_gold);
}

TEST(GriddedData, 4D)
{
  GriddedData gridded_data("data/gridded/fourd.txt");

  // Test dimension
  EXPECT_EQ(gridded_data.getDim(), 4);

  // Test axes
  std::vector<int> axes;
  gridded_data.getAxes(axes);

  std::vector<int> axes_gold{{0, 1, 2, 3}};
  EXPECT_EQ(axes, axes_gold);

  // Test grid (AXES points in file)
  std::vector<std::vector<Real>> grid;
  gridded_data.getGrid(grid);

  std::vector<std::vector<Real>> grid_gold{{{0.0, 1.0}, {0.0, 1.0}, {0.0, 1.0}, {3.0, 7.0}}};
  EXPECT_EQ(grid, grid_gold);

  // Test function values (DATA values in file)
  std::vector<Real> fcn;
  gridded_data.getFcn(fcn);

  std::vector<Real> fcn_gold{
      {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0}};
  EXPECT_EQ(fcn, fcn_gold);
}

TEST(GriddedData, noAxes)
{
  try
  {
    GriddedData gridded_data("data/gridded/no_axes.txt");
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("No valid AXIS lines found by GriddedData");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(GriddedData, emptyAxis)
{
  try
  {
    GriddedData gridded_data("data/gridded/empty_axis.txt");
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("Axis 1 in your GriddedData has zero size");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(GriddedData, missingData)
{
  try
  {
    GriddedData gridded_data("data/gridded/missing_data.txt");
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("According to AXIS statements in GriddedData, number of data "
                                "points is 9 but 6 function values were read from file");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

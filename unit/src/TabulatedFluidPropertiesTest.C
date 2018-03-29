//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedFluidPropertiesTest.h"
#include "Utils.h"

// Test data for unordered data
TEST_F(TabulatedFluidPropertiesTest, unorderedData)
{
  try
  {
    // Must cast away const to call initialSetup(), where the file is
    // checked for consistency
    const_cast<TabulatedFluidProperties *>(_unordered_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find(
                "unordered_fp: the column data for temperature is not monotonically increasing in "
                "data/csv/unordered_fluid_props.csv");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test data for different temperatues ranges in pressure data
TEST_F(TabulatedFluidPropertiesTest, unequalTemperatures)
{
  try
  {
    const_cast<TabulatedFluidProperties *>(_unequal_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("unequal_fp: temperature values for pressure 2e+06 are not "
                                "identical to values for 1e+06");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test data for missing column
TEST_F(TabulatedFluidPropertiesTest, missingColumn)
{
  try
  {
    const_cast<TabulatedFluidProperties *>(_missing_col_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("missing_col_fp: no temperature data read in "
                                "data/csv/missing_col_fluid_props.csv. A "
                                "column named temperature must be present");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test data for unknown property column
TEST_F(TabulatedFluidPropertiesTest, unknownColumn)
{
  try
  {
    const_cast<TabulatedFluidProperties *>(_unknown_col_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("unknown read in data/csv/unknown_fluid_props.csv is not one of "
                                "the properties that TabulatedFluidProperties understands");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test data for missing data
TEST_F(TabulatedFluidPropertiesTest, missingData)
{
  try
  {
    const_cast<TabulatedFluidProperties *>(_missing_data_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos =
        std::string(err.what())
            .find("missing_data_fp: the number of rows in data/csv/missing_data_fluid_props.csv "
                  "is not equal to the number of unique pressure values 3 multiplied "
                  "by the number of unique temperature values 3");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test tabulated fluid properties read from file including comments
TEST_F(TabulatedFluidPropertiesTest, fromFile)
{
  Real p = 1.5e6;
  Real T = 450.0;

  // Read the data file
  const_cast<TabulatedFluidProperties *>(_tab_fp)->initialSetup();

  REL_TEST("density", _tab_fp->rho(p, T), _co2_fp->rho(p, T), 1.0e-4);
  REL_TEST("enthalpy", _tab_fp->h(p, T), _co2_fp->h(p, T), 1.0e-4);
  REL_TEST("internal_energy", _tab_fp->e(p, T), _co2_fp->e(p, T), 1.0e-4);
  REL_TEST("viscosity", _tab_fp->mu(p, T), _co2_fp->mu(p, T), 1.0e-4);
  REL_TEST("k", _tab_fp->k(p, T), _co2_fp->k(p, T), 1.0e-4);
  REL_TEST("cp", _tab_fp->cp(p, T), _co2_fp->cp(p, T), 1.0e-4);
  REL_TEST("cv", _tab_fp->cv(p, T), _co2_fp->cv(p, T), 1.0e-4);
  REL_TEST("entropy", _tab_fp->s(p, T), _co2_fp->s(p, T), 1.0e-4);
}

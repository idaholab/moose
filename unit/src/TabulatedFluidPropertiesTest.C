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
            .find("The column data for temperature is not monotonically increasing in "
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
    std::size_t pos =
        std::string(err.what())
            .find("Temperature values for pressure 2e+06 are not identical to values for 1e+06");
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
                          .find("No enthalpy data read in data/csv/missing_col_fluid_props.csv. A "
                                "column named enthalpy must be present");
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
    std::size_t pos = std::string(err.what())
                          .find("The number of rows in data/csv/missing_data_fluid_props.csv "
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
}

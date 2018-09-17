//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesPTTestUtils.h"

#include <fstream>

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

  // Fluid properties
  REL_TEST(_tab_fp->rho_from_p_T(p, T), _co2_fp->rho_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_fp->h_from_p_T(p, T), _co2_fp->h_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_fp->e_from_p_T(p, T), _co2_fp->e_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_fp->mu_from_p_T(p, T), _co2_fp->mu_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_fp->k_from_p_T(p, T), _co2_fp->k_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_fp->cp_from_p_T(p, T), _co2_fp->cp_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_fp->cv_from_p_T(p, T), _co2_fp->cv_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_fp->s_from_p_T(p, T), _co2_fp->s_from_p_T(p, T), 1.0e-4);

  // Fluid properties and derivatives
  Real rho, drho_dp, drho_dT, rhoc, drhoc_dp, drhoc_dT;
  _tab_fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  _co2_fp->rho_from_p_T(p, T, rhoc, drhoc_dp, drhoc_dT);
  REL_TEST(rho, rhoc, 1.0e-4);
  REL_TEST(drho_dp, drhoc_dp, 1.0e-3);
  REL_TEST(drho_dT, drhoc_dT, 1.0e-3);

  Real h, dh_dp, dh_dT, hc, dhc_dp, dhc_dT;
  _tab_fp->h_from_p_T(p, T, h, dh_dp, dh_dT);
  _co2_fp->h_from_p_T(p, T, hc, dhc_dp, dhc_dT);
  REL_TEST(h, hc, 1.0e-4);
  REL_TEST(dh_dp, dhc_dp, 1.0e-3);
  REL_TEST(dh_dT, dhc_dT, 1.0e-3);

  Real mu, dmu_dp, dmu_dT, muc, dmuc_dp, dmuc_dT;
  _tab_fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  _co2_fp->mu_from_p_T(p, T, muc, dmuc_dp, dmuc_dT);
  REL_TEST(mu, muc, 1.0e-4);
  REL_TEST(dmu_dp, dmuc_dp, 1.0e-3);
  REL_TEST(dmu_dT, dmuc_dT, 1.0e-3);

  Real e, de_dp, de_dT, ec, dec_dp, dec_dT;
  _tab_fp->e_from_p_T(p, T, e, de_dp, de_dT);
  _co2_fp->e_from_p_T(p, T, ec, dec_dp, dec_dT);
  REL_TEST(e, ec, 1.0e-4);
  REL_TEST(de_dp, dec_dp, 1.0e-3);
  REL_TEST(de_dT, dec_dT, 1.0e-3);
}

// Test generation of tabulated fluid properties
TEST_F(TabulatedFluidPropertiesTest, generateTabulatedData)
{
  Real p = 1.5e6;
  Real T = 450.0;

  // Generate the tabulated data
  const_cast<TabulatedFluidProperties *>(_tab_gen_fp)->initialSetup();

  REL_TEST(_tab_gen_fp->rho_from_p_T(p, T), _co2_fp->rho_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_gen_fp->h_from_p_T(p, T), _co2_fp->h_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_gen_fp->e_from_p_T(p, T), _co2_fp->e_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_gen_fp->mu_from_p_T(p, T), _co2_fp->mu_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_gen_fp->k_from_p_T(p, T), _co2_fp->k_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_gen_fp->cp_from_p_T(p, T), _co2_fp->cp_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_gen_fp->cv_from_p_T(p, T), _co2_fp->cv_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_gen_fp->s_from_p_T(p, T), _co2_fp->s_from_p_T(p, T), 1.0e-4);
}

// Test that all fluid properties are properly passed back to the given user object
// if they are not tabulated
TEST_F(TabulatedFluidPropertiesTest, passthrough)
{
  Real p = 1.5e6;
  Real T = 450.0;
  const Real tol = REL_TOL_SAVED_VALUE;

  // As the flags for interpolation in TabulatedFluidProperties default to false,
  // properties will be passed through to the given userobject
  ABS_TEST(_tab_fp->rho_from_p_T(p, T), _co2_fp->rho_from_p_T(p, T), tol);
  ABS_TEST(_tab_fp->h_from_p_T(p, T), _co2_fp->h_from_p_T(p, T), tol);
  ABS_TEST(_tab_fp->e_from_p_T(p, T), _co2_fp->e_from_p_T(p, T), tol);
  ABS_TEST(_tab_fp->mu_from_p_T(p, T), _co2_fp->mu_from_p_T(p, T), tol);
  ABS_TEST(_tab_fp->k_from_p_T(p, T), _co2_fp->k_from_p_T(p, T), tol);
  ABS_TEST(_tab_fp->cp_from_p_T(p, T), _co2_fp->cp_from_p_T(p, T), tol);
  ABS_TEST(_tab_fp->cv_from_p_T(p, T), _co2_fp->cv_from_p_T(p, T), tol);
  ABS_TEST(_tab_fp->s_from_p_T(p, T), _co2_fp->s_from_p_T(p, T), tol);
  ABS_TEST(_tab_fp->henryConstant(T), _co2_fp->henryConstant(T), tol);
}

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(TabulatedFluidPropertiesTest, fluidName) { EXPECT_EQ(_tab_fp->fluidName(), "co2"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(TabulatedFluidPropertiesTest, molarMass)
{
  ABS_TEST(_tab_fp->molarMass(), 44.0098e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(TabulatedFluidPropertiesTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;

  combinedProperties(_tab_fp, p, T, REL_TOL_SAVED_VALUE);
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedBicubicFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

#include <fstream>

// Test data for unordered data
TEST_F(TabulatedBicubicFluidPropertiesTest, unorderedData)
{
  try
  {
    // Must cast away const to call initialSetup(), where the file is
    // checked for consistency
    const_cast<TabulatedBicubicFluidProperties *>(_unordered_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("The column data for temperature is not monotonically increasing");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test data for different temperatures ranges in pressure data
TEST_F(TabulatedBicubicFluidPropertiesTest, unequalTemperatures)
{
  try
  {
    const_cast<TabulatedBicubicFluidProperties *>(_unequal_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("temperature values for pressure 2e+06 are not "
                                "identical to values for 1e+06");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test data for missing column
TEST_F(TabulatedBicubicFluidPropertiesTest, missingColumn)
{
  try
  {
    const auto tow = Moose::_throw_on_warning;
    Moose::_throw_on_warning = true;
    const_cast<TabulatedBicubicFluidProperties *>(_missing_col_fp)->initialSetup();
    Moose::_throw_on_warning = tow;
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("data/csv/missing_col_fluid_props.csv. A "
                                "column named temperature must be present");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test data for unknown property column
TEST_F(TabulatedBicubicFluidPropertiesTest, unknownColumn)
{
  try
  {
    const_cast<TabulatedBicubicFluidProperties *>(_unknown_col_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("data/csv/unknown_fluid_props.csv tabulation file is not one of "
                                "the properties that TabulatedFluidProperties understands");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test data for missing data
TEST_F(TabulatedBicubicFluidPropertiesTest, missingData)
{
  try
  {
    const_cast<TabulatedBicubicFluidProperties *>(_missing_data_fp)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("data/csv/missing_data_fluid_props.csv "
                                "is not equal to the number of unique pressure values 3 multiplied "
                                "by the number of unique temperature values 3");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

// Test tabulated fluid properties read from file including comments
TEST_F(TabulatedBicubicFluidPropertiesTest, fromPTFile)
{
  Real p = 1.5e6;
  Real T = 450.0;

  // Read the data file
  const_cast<TabulatedBicubicFluidProperties *>(_tab_pT_from_fp)->initialSetup();

  // Fluid properties
  REL_TEST(_tab_pT_from_fp->rho_from_p_T(p, T), _co2_fp->rho_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_pT_from_fp->h_from_p_T(p, T), _co2_fp->h_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_pT_from_fp->e_from_p_T(p, T), _co2_fp->e_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_pT_from_fp->mu_from_p_T(p, T), _co2_fp->mu_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_pT_from_fp->k_from_p_T(p, T), _co2_fp->k_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_pT_from_fp->cp_from_p_T(p, T), _co2_fp->cp_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_pT_from_fp->cv_from_p_T(p, T), _co2_fp->cv_from_p_T(p, T), 1.0e-4);
  REL_TEST(_tab_pT_from_fp->s_from_p_T(p, T), _co2_fp->s_from_p_T(p, T), 1.0e-4);

  // Fluid properties and derivatives
  Real rho, drho_dp, drho_dT, rhoc, drhoc_dp, drhoc_dT;
  _tab_pT_from_fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  _co2_fp->rho_from_p_T(p, T, rhoc, drhoc_dp, drhoc_dT);
  REL_TEST(rho, rhoc, 1.0e-4);
  REL_TEST(drho_dp, drhoc_dp, 1.0e-3);
  REL_TEST(drho_dT, drhoc_dT, 1.0e-3);

  Real h, dh_dp, dh_dT, hc, dhc_dp, dhc_dT;
  _tab_pT_from_fp->h_from_p_T(p, T, h, dh_dp, dh_dT);
  _co2_fp->h_from_p_T(p, T, hc, dhc_dp, dhc_dT);
  REL_TEST(h, hc, 1.0e-4);
  REL_TEST(dh_dp, dhc_dp, 1.0e-3);
  REL_TEST(dh_dT, dhc_dT, 1.0e-3);

  Real mu, dmu_dp, dmu_dT, muc, dmuc_dp, dmuc_dT;
  _tab_pT_from_fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  _co2_fp->mu_from_p_T(p, T, muc, dmuc_dp, dmuc_dT);
  REL_TEST(mu, muc, 1.0e-4);
  REL_TEST(dmu_dp, dmuc_dp, 1.0e-3);
  REL_TEST(dmu_dT, dmuc_dT, 1.0e-3);

  Real e, de_dp, de_dT, ec, dec_dp, dec_dT;
  _tab_pT_from_fp->e_from_p_T(p, T, e, de_dp, de_dT);
  _co2_fp->e_from_p_T(p, T, ec, dec_dp, dec_dT);
  REL_TEST(e, ec, 1.0e-4);
  REL_TEST(de_dp, dec_dp, 1.0e-3);
  REL_TEST(de_dT, dec_dT, 1.0e-3);
}

// Test tabulated fluid properties read from file including comments
TEST_F(TabulatedBicubicFluidPropertiesTest, fromPTFileToVE)
{
  Real p = 1.5e6;
  Real T = 450.0;
  Real pert = 1.0e-7;

  // Read the data file
  Moose::_throw_on_warning = false;
  const_cast<TabulatedBicubicFluidProperties *>(_tab_ve_from_pT)->initialSetup();
  Moose::_throw_on_warning = true;

  // round trip p,T -> v,e -> p,T
  {
    Real e = _tab_ve_from_pT->e_from_p_T(p, T);
    Real v = _tab_ve_from_pT->v_from_p_T(p, T);
    Real pp = _tab_ve_from_pT->p_from_v_e(v, e);
    Real TT = _tab_ve_from_pT->T_from_v_e(v, e);
    ABS_TEST(T, TT, 1.0);
    REL_TEST(p, pp, 0.001);
  }

  // check computation of fluid props from p, T & v, e
  {
    Real e = _tab_ve_from_pT->e_from_p_T(p, T);
    Real v = _tab_ve_from_pT->v_from_p_T(p, T);

    // heat capacity at constant pressure
    Real cp1 = _tab_ve_from_pT->cp_from_p_T(p, T);
    Real cp2 = _tab_ve_from_pT->cp_from_v_e(v, e);
    REL_TEST(cp1, cp2, 0.001);

    // heat capacity at constant volume
    Real cv1 = _tab_ve_from_pT->cv_from_p_T(p, T);
    Real cv2 = _tab_ve_from_pT->cv_from_v_e(v, e);
    REL_TEST(cv1, cv2, 0.001);

    // viscosity
    Real mu1 = _tab_ve_from_pT->mu_from_p_T(p, T);
    Real mu2 = _tab_ve_from_pT->mu_from_v_e(v, e);
    REL_TEST(mu1, mu2, 0.001);

    // thermal conductivity
    Real k1 = _tab_ve_from_pT->k_from_p_T(p, T);
    Real k2 = _tab_ve_from_pT->k_from_v_e(v, e);
    REL_TEST(k1, k2, 0.001);
  }

  // check computation of fluid props from p, s
  {
    Real s = _tab_ve_from_pT->s_from_p_T(p, T);

    // density
    Real rho1 = _tab_ve_from_pT->rho_from_p_T(p, T);
    Real rho2 = _tab_ve_from_pT->rho_from_p_s(p, s);
    REL_TEST(rho1, rho2, 0.001);

    // temperature
    Real Ts = _tab_ve_from_pT->T_from_p_s(p, s);
    REL_TEST(T, Ts, 0.001);
  }

  // check computation of fluids props from p, h
  {
    Real T = 450;
    Real p = 1.5e6;
    Real h = _tab_ve_from_pT->h_from_p_T(p, T);
    Real Ts = _tab_ve_from_pT->T_from_p_h(p, h);
    REL_TEST(T, Ts, 1e-4);

    // to keep coverage on the default definition in SinglePhaseFP
    Ts = dynamic_cast<const SinglePhaseFluidProperties *>(_tab_ve_from_pT)->T_from_p_h(p, h);
    REL_TEST(T, Ts, 1e-4);
  }

  // are the two version of functions equivalent
  {
    Real e = _tab_ve_from_pT->e_from_p_T(p, T);
    Real v = _tab_ve_from_pT->v_from_p_T(p, T);

    Real d1, d2;

    // speed of sound errors bc co2 props don't
    // implement enough

    // heat capacity at constant pressure
    Real cp1;
    _tab_ve_from_pT->cp_from_v_e(v, e, cp1, d1, d2);
    Real cp2 = _tab_ve_from_pT->cp_from_v_e(v, e);
    REL_TEST(cp1, cp2, 0.001);

    // heat capacity at constant volume
    Real cv1;
    _tab_ve_from_pT->cv_from_v_e(v, e, cv1, d1, d2);
    Real cv2 = _tab_ve_from_pT->cv_from_v_e(v, e);
    REL_TEST(cv1, cv2, 0.001);

    // viscosity
    Real mu1;
    _tab_ve_from_pT->mu_from_v_e(v, e, mu1, d1, d2);
    Real mu2 = _tab_ve_from_pT->mu_from_v_e(v, e);
    REL_TEST(mu1, mu2, 0.001);

    // thermal conductivity
    Real k1;
    _tab_ve_from_pT->k_from_v_e(v, e, k1, d1, d2);
    Real k2 = _tab_ve_from_pT->k_from_v_e(v, e);
    REL_TEST(k1, k2, 0.001);
  }

  // check derivatives
  {
    Real e = _tab_ve_from_pT->e_from_p_T(p, T);
    Real v = _tab_ve_from_pT->v_from_p_T(p, T);

    Real deriv1, deriv2;

    // pressure
    Real p1;
    _tab_ve_from_pT->p_from_v_e(v, e, p1, deriv1, deriv2);
    Real p2 = _tab_ve_from_pT->p_from_v_e(v, e);
    REL_TEST(p1, p2, 0.001);

    // temperature
    Real T1;
    _tab_ve_from_pT->T_from_v_e(v, e, T1, deriv1, deriv2);
    Real T2 = _tab_ve_from_pT->T_from_v_e(v, e);
    REL_TEST(T1, T2, 0.001);

    // speed of sound errors bc co2 props don't
    // implement enough

    // heat capacity at constant pressure
    Real cp1;
    _tab_ve_from_pT->cp_from_v_e(v, e, cp1, deriv1, deriv2);
    Real cp_0 = _tab_ve_from_pT->cp_from_v_e(v, e);
    Real cp_1 = _tab_ve_from_pT->cp_from_v_e(v * (1 + pert), e);
    Real cp_2 = _tab_ve_from_pT->cp_from_v_e(v, e * (1 + pert));
    REL_TEST(deriv1, (cp_1 - cp_0) / (v * pert), 0.001);
    REL_TEST(deriv2, (cp_2 - cp_0) / (e * pert), 0.001);

    // heat capacity at constant volume
    Real cv1;
    _tab_ve_from_pT->cv_from_v_e(v, e, cv1, deriv1, deriv2);
    Real cv_0 = _tab_ve_from_pT->cv_from_v_e(v, e);
    Real cv_1 = _tab_ve_from_pT->cv_from_v_e(v * (1 + pert), e);
    Real cv_2 = _tab_ve_from_pT->cv_from_v_e(v, e * (1 + pert));
    REL_TEST(deriv1, (cv_1 - cv_0) / (v * pert), 0.001);
    REL_TEST(deriv2, (cv_2 - cv_0) / (e * pert), 0.001);

    // viscosity
    Real mu1;
    _tab_ve_from_pT->mu_from_v_e(v, e, mu1, deriv1, deriv2);
    Real mu_0 = _tab_ve_from_pT->mu_from_v_e(v, e);
    Real mu_1 = _tab_ve_from_pT->mu_from_v_e(v * (1 + pert), e);
    Real mu_2 = _tab_ve_from_pT->mu_from_v_e(v, e * (1 + pert));
    REL_TEST(deriv1, (mu_1 - mu_0) / (v * pert), 0.001);
    REL_TEST(deriv2, (mu_2 - mu_0) / (e * pert), 0.001);

    // thermal conductivity
    Real k1;
    _tab_ve_from_pT->k_from_v_e(v, e, k1, deriv1, deriv2);
    Real k_0 = _tab_ve_from_pT->k_from_v_e(v, e);
    Real k_1 = _tab_ve_from_pT->k_from_v_e(v * (1 + pert), e);
    Real k_2 = _tab_ve_from_pT->k_from_v_e(v, e * (1 + pert));
    REL_TEST(deriv1, (k_1 - k_0) / (v * pert), 0.001);
    REL_TEST(deriv2, (k_2 - k_0) / (e * pert), 0.001);
  }

  // test enthalpy relationships
  {
    Real h = _tab_ve_from_pT->h_from_p_T(p, T);
    Real v = _tab_ve_from_pT->v_from_p_T(p, T);
    Real e_gold = _tab_ve_from_pT->e_from_p_T(p, T);
    Real e = _tab_ve_from_pT->e_from_v_h(v, h);
    REL_TEST(e_gold, e, 0.001);

    Real e2, de_dv, de_dh;
    _tab_ve_from_pT->e_from_v_h(v, h, e2, de_dv, de_dh);
    REL_TEST(e_gold, e2, 0.001);
    Real e_0 = _tab_ve_from_pT->e_from_v_h(v, h);
    Real e_1 = _tab_ve_from_pT->e_from_v_h(v * (1 + pert), h);
    Real e_2 = _tab_ve_from_pT->e_from_v_h(v, h * (1 + pert));
    REL_TEST(de_dv, (e_1 - e_0) / (v * pert), 0.001);
    REL_TEST(de_dh, (e_2 - e_0) / (h * pert), 0.001);
  }

  // AD p_from_v_e
  {
    Real e = _tab_ve_from_pT->e_from_p_T(p, T);
    Real v = _tab_ve_from_pT->v_from_p_T(p, T);
    DNDerivativeType dvdx;
    DNDerivativeType dedx;
    // set it up so these are the derivatives
    // w.r.t. to themselves
    Moose::derivInsert(dvdx, 0, 1);
    Moose::derivInsert(dvdx, 1, 0);
    Moose::derivInsert(dedx, 0, 0);
    Moose::derivInsert(dedx, 1, 1);

    ADReal v_ad(v, dvdx);
    ADReal e_ad(e, dedx);
    ADReal p_ad = _tab_ve_from_pT->p_from_v_e(v_ad, e_ad);

    Real pp, dp_dv, dp_de;
    _tab_ve_from_pT->p_from_v_e(v, e, pp, dp_dv, dp_de);
    REL_TEST(p_ad.derivatives()[0], dp_dv, 0.0001);
    REL_TEST(p_ad.derivatives()[1], dp_de, 0.0001);
  }

  // AD T_from_v_e
  {
    Real e = _tab_ve_from_pT->e_from_p_T(p, T);
    Real v = _tab_ve_from_pT->v_from_p_T(p, T);
    DNDerivativeType dvdx;
    DNDerivativeType dedx;
    // set it up so these are the derivatives
    // w.r.t. to themselves
    Moose::derivInsert(dvdx, 0, 1);
    Moose::derivInsert(dvdx, 1, 0);
    Moose::derivInsert(dedx, 0, 0);
    Moose::derivInsert(dedx, 1, 1);

    ADReal v_ad(v, dvdx);
    ADReal e_ad(e, dedx);
    ADReal T_ad = _tab_ve_from_pT->T_from_v_e(v_ad, e_ad);

    Real TT, dT_dv, dT_de;
    _tab_ve_from_pT->T_from_v_e(v, e, TT, dT_dv, dT_de);
    REL_TEST(T_ad.derivatives()[0], dT_dv, 0.0001);
    REL_TEST(T_ad.derivatives()[1], dT_de, 0.0001);
  }

  // cannot test AD c_from_v_e because co2 props do not
  // implement enough

  // AD T_from_p_h
  {
    Real h = _tab_ve_from_pT->h_from_p_T(p, T);
    DNDerivativeType dpdx;
    DNDerivativeType dhdx;
    // set it up so these are the derivatives
    // w.r.t. to themselves
    Moose::derivInsert(dpdx, 0, 1);
    Moose::derivInsert(dpdx, 1, 0);
    Moose::derivInsert(dhdx, 0, 0);
    Moose::derivInsert(dhdx, 1, 1);

    ADReal p_ad(p, dpdx);
    ADReal h_ad(h, dhdx);
    ADReal T_ad = _tab_ve_from_pT->T_from_p_h(p_ad, h_ad);

    Real TT_1 = _tab_ve_from_pT->T_from_p_h(p * (1 + pert), h);
    Real TT_2 = _tab_ve_from_pT->T_from_p_h(p, h * (1 + pert));
    Real dT_dp = (TT_1 - T) / p / pert;
    Real dT_dh = (TT_2 - T) / h / pert;
    REL_TEST(T_ad.derivatives()[0], dT_dp, 0.0001);
    REL_TEST(T_ad.derivatives()[1], dT_dh, 0.0001);
  }
}

// Test tabulated fluid properties read from file including comments
TEST_F(TabulatedBicubicFluidPropertiesTest, fromVEFile)
{
  // These values must be within the bounds specified in the header
  Real p = 1.223e6;
  Real T = 420.1;
  Real pert = 1.0e-7;

  // Read the data file
  Moose::_throw_on_warning = false;
  const_cast<TabulatedBicubicFluidProperties *>(_tab_ve_from_fp)->initialSetup();
  Moose::_throw_on_warning = true;

  // Use as a reference
  Real e = _idg_fp->e_from_p_T(p, T);
  Real v = _idg_fp->v_from_p_T(p, T);

  // NOTE: direct from (v,e) currently does not support
  //       calls with (p, T), (p, rho), (p, s).

  // check computation of fluid props from v, e
  {
    // heat capacity at constant pressure
    Real cp1 = _idg_fp->cp_from_v_e(v, e);
    Real cp2 = _tab_ve_from_fp->cp_from_v_e(v, e);
    REL_TEST(cp1, cp2, 0.001);

    // heat capacity at constant volume
    Real cv1 = _idg_fp->cv_from_v_e(v, e);
    Real cv2 = _tab_ve_from_fp->cv_from_v_e(v, e);
    REL_TEST(cv1, cv2, 0.001);

    // speed of sound
    Real c1 = _idg_fp->c_from_v_e(v, e);
    Real c2 = _tab_ve_from_fp->c_from_v_e(v, e);
    REL_TEST(c1, c2, 0.001);

    // viscosity
    Real mu1 = _idg_fp->mu_from_v_e(v, e);
    Real mu2 = _tab_ve_from_fp->mu_from_v_e(v, e);
    REL_TEST(mu1, mu2, 0.001);

    // thermal conductivity
    Real k1 = _idg_fp->k_from_v_e(v, e);
    Real k2 = _tab_ve_from_fp->k_from_v_e(v, e);
    REL_TEST(k1, k2, 0.001);
  }

  // are the two version of functions equivalent
  {
    Real d1, d2;

    // pressure
    Real p1;
    _tab_ve_from_fp->p_from_v_e(v, e, p1, d1, d2);
    Real p2 = _tab_ve_from_fp->p_from_v_e(v, e);
    REL_TEST(p1, p2, 0.001);

    // temperature
    Real T1;
    _tab_ve_from_fp->T_from_v_e(v, e, T1, d1, d2);
    Real T2 = _tab_ve_from_fp->T_from_v_e(v, e);
    REL_TEST(T1, T2, 0.001);

    // heat capacity at constant pressure
    Real cp1;
    _tab_ve_from_fp->cp_from_v_e(v, e, cp1, d1, d2);
    Real cp2 = _tab_ve_from_fp->cp_from_v_e(v, e);
    REL_TEST(cp1, cp2, 0.001);

    // heat capacity at constant volume
    Real cv1;
    _tab_ve_from_fp->cv_from_v_e(v, e, cv1, d1, d2);
    Real cv2 = _tab_ve_from_fp->cv_from_v_e(v, e);
    REL_TEST(cv1, cv2, 0.001);

    // speed of sound
    Real c1;
    _tab_ve_from_fp->c_from_v_e(v, e, c1, d1, d2);
    Real c2 = _tab_ve_from_fp->c_from_v_e(v, e);
    REL_TEST(c1, c2, 0.001);

    // viscosity
    Real mu1;
    _tab_ve_from_fp->mu_from_v_e(v, e, mu1, d1, d2);
    Real mu2 = _tab_ve_from_fp->mu_from_v_e(v, e);
    REL_TEST(mu1, mu2, 0.001);

    // thermal conductivity
    Real k1;
    _tab_ve_from_fp->k_from_v_e(v, e, k1, d1, d2);
    Real k2 = _tab_ve_from_fp->k_from_v_e(v, e);
    REL_TEST(k1, k2, 0.001);
  }

  // check derivatives
  {
    Real deriv1, deriv2;

    // speed of sound errors bc co2 props don't
    // implement enough

    // heat capacity at constant pressure
    Real cp1;
    _tab_ve_from_fp->cp_from_v_e(v, e, cp1, deriv1, deriv2);
    Real cp_0 = _tab_ve_from_fp->cp_from_v_e(v, e);
    Real cp_1 = _tab_ve_from_fp->cp_from_v_e(v * (1 + pert), e);
    Real cp_2 = _tab_ve_from_fp->cp_from_v_e(v, e * (1 + pert));
    REL_TEST(deriv1, (cp_1 - cp_0) / (v * pert), 0.001);
    REL_TEST(deriv2, (cp_2 - cp_0) / (e * pert), 0.001);

    // heat capacity at constant volume
    Real cv1;
    _tab_ve_from_fp->cv_from_v_e(v, e, cv1, deriv1, deriv2);
    Real cv_0 = _tab_ve_from_fp->cv_from_v_e(v, e);
    Real cv_1 = _tab_ve_from_fp->cv_from_v_e(v * (1 + pert), e);
    Real cv_2 = _tab_ve_from_fp->cv_from_v_e(v, e * (1 + pert));
    REL_TEST(deriv1, (cv_1 - cv_0) / (v * pert), 0.001);
    REL_TEST(deriv2, (cv_2 - cv_0) / (e * pert), 0.001);

    // viscosity
    Real mu1;
    _tab_ve_from_fp->mu_from_v_e(v, e, mu1, deriv1, deriv2);
    Real mu_0 = _tab_ve_from_fp->mu_from_v_e(v, e);
    Real mu_1 = _tab_ve_from_fp->mu_from_v_e(v * (1 + pert), e);
    Real mu_2 = _tab_ve_from_fp->mu_from_v_e(v, e * (1 + pert));
    REL_TEST(deriv1, (mu_1 - mu_0) / (v * pert), 0.001);
    REL_TEST(deriv2, (mu_2 - mu_0) / (e * pert), 0.001);

    // thermal conductivity
    Real k1;
    _tab_ve_from_fp->k_from_v_e(v, e, k1, deriv1, deriv2);
    Real k_0 = _tab_ve_from_fp->k_from_v_e(v, e);
    Real k_1 = _tab_ve_from_fp->k_from_v_e(v * (1 + pert), e);
    Real k_2 = _tab_ve_from_fp->k_from_v_e(v, e * (1 + pert));
    REL_TEST(deriv1, (k_1 - k_0) / (v * pert), 0.001);
    REL_TEST(deriv2, (k_2 - k_0) / (e * pert), 0.001);
  }

  // test enthalpy relationships (v,h)
  {
    Moose::_throw_on_warning = false;
    Real h = _idg_fp->h_from_p_T(p, T);
    Real v = _idg_fp->v_from_p_T(p, T);
    Real e_gold = _idg_fp->e_from_p_T(p, T);
    Real e0 = _tab_ve_from_fp->e_from_v_h(v, h);
    REL_TEST(e_gold, e0, 0.001);

    Real e2, de_dv, de_dh;
    _tab_ve_from_fp->e_from_v_h(v, h, e2, de_dv, de_dh);
    REL_TEST(e_gold, e2, 0.001);
    Real e_0 = _tab_ve_from_fp->e_from_v_h(v, h);
    Real e_1 = _tab_ve_from_fp->e_from_v_h(v * (1 + pert), h);
    Real e_2 = _tab_ve_from_fp->e_from_v_h(v, h * (1 + pert));
    REL_TEST(de_dv, (e_1 - e_0) / (v * pert), 0.001);
    REL_TEST(de_dh, (e_2 - e_0) / (h * pert), 0.001);
    Moose::_throw_on_warning = true;
  }

  // AD p_from_v_e
  {
    DNDerivativeType dvdx;
    DNDerivativeType dedx;
    // set it up so these are the derivatives
    // w.r.t. to themselves
    Moose::derivInsert(dvdx, 0, 1);
    Moose::derivInsert(dvdx, 1, 0);
    Moose::derivInsert(dedx, 0, 0);
    Moose::derivInsert(dedx, 1, 1);

    ADReal v_ad(v, dvdx);
    ADReal e_ad(e, dedx);
    ADReal p_ad = _tab_ve_from_fp->p_from_v_e(v_ad, e_ad);

    Real pp, dp_dv, dp_de;
    _tab_ve_from_fp->p_from_v_e(v, e, pp, dp_dv, dp_de);
    REL_TEST(p_ad.derivatives()[0], dp_dv, 0.0001);
    REL_TEST(p_ad.derivatives()[1], dp_de, 0.0001);
  }

  // AD T_from_v_e
  {
    DNDerivativeType dvdx;
    DNDerivativeType dedx;
    // set it up so these are the derivatives
    // w.r.t. to themselves
    Moose::derivInsert(dvdx, 0, 1);
    Moose::derivInsert(dvdx, 1, 0);
    Moose::derivInsert(dedx, 0, 0);
    Moose::derivInsert(dedx, 1, 1);

    ADReal v_ad(v, dvdx);
    ADReal e_ad(e, dedx);
    ADReal T_ad = _tab_ve_from_fp->T_from_v_e(v_ad, e_ad);

    Real TT, dT_dv, dT_de;
    _tab_ve_from_fp->T_from_v_e(v, e, TT, dT_dv, dT_de);
    REL_TEST(T_ad.derivatives()[0], dT_dv, 0.0001);
    REL_TEST(T_ad.derivatives()[1], dT_de, 0.0001);
  }

  // cannot test AD c_from_v_e because co2 props do not
  // implement enough
}

/**
 * Verify calculation of the derivatives of tabulated properties by comparing with finite
 * differences
 */
TEST_F(TabulatedBicubicFluidPropertiesTest, derivatives)
{

  const Real p = 1.5e6;
  const Real T = 452.0;

  {
    // Read the data file and generate (v,e) to (p,T) interpolations
    Moose::_throw_on_warning = false;
    const_cast<TabulatedBicubicFluidProperties *>(_tab_ve_from_pT)->initialSetup();
    Moose::_throw_on_warning = true;
    const Real tol = REL_TOL_DERIVATIVE;

    const Real rho = _tab_ve_from_pT->rho_from_p_T(p, T);
    const Real v = 1. / rho;
    const Real e = _tab_ve_from_pT->e_from_p_T(p, T);
    const Real s = _tab_ve_from_pT->s_from_p_T(p, T);

    DERIV_TEST(_tab_ve_from_pT->rho_from_p_T, p, T, tol);
    DERIV_TEST(_tab_ve_from_pT->e_from_p_T, p, T, tol);
    DERIV_TEST(_tab_ve_from_pT->v_from_p_T, p, T, tol);
    DERIV_TEST(_tab_ve_from_pT->h_from_p_T, p, T, tol);
    DERIV_TEST(_tab_ve_from_pT->k_from_p_T, p, T, tol);
    DERIV_TEST(_tab_ve_from_pT->cp_from_p_T, p, T, tol);
    DERIV_TEST(_tab_ve_from_pT->cv_from_p_T, p, T, tol);
    DERIV_TEST(_tab_ve_from_pT->mu_from_p_T, p, T, tol);

    DERIV_TEST(_tab_ve_from_pT->p_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_pT->mu_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_pT->k_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_pT->T_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_pT->cp_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_pT->cv_from_v_e, v, e, tol);

    DERIV_TEST(_tab_ve_from_pT->T_from_p_rho, p, rho, tol);
    DERIV_TEST(_tab_ve_from_pT->e_from_p_rho, p, rho, tol);

    DERIV_TEST(_tab_ve_from_pT->T_from_p_s, p, s, tol);
    DERIV_TEST(_tab_ve_from_pT->rho_from_p_s, p, s, tol);
  }

  {
    // Read the data file with direct to (v,e) interpolations
    Moose::_throw_on_warning = false;
    const_cast<TabulatedBicubicFluidProperties *>(_tab_ve_from_fp)->initialSetup();
    Moose::_throw_on_warning = true;
    const Real tol = REL_TOL_DERIVATIVE;

    const Real rho = _tab_ve_from_fp->rho_from_p_T(p, T);
    const Real v = 1. / rho;
    const Real e = _tab_ve_from_fp->e_from_p_T(p, T);
    const Real h = e + p * v;

    DERIV_TEST(_tab_ve_from_fp->p_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_fp->mu_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_fp->k_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_fp->T_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_fp->cp_from_v_e, v, e, tol);
    DERIV_TEST(_tab_ve_from_fp->cv_from_v_e, v, e, tol);

    // Jacobian warning and finite differencing in use
    Moose::_throw_on_warning = false;
    DERIV_TEST(_tab_ve_from_fp->e_from_v_h, v, h, 100 * tol);
    Moose::_throw_on_warning = true;
  }
}

// Test generation of tabulated fluid properties
TEST_F(TabulatedBicubicFluidPropertiesTest, generateTabulatedData)
{
  Real p = 1.5e6;
  Real T = 450.0;

  // Generate the tabulated data
  const_cast<TabulatedBicubicFluidProperties *>(_tab_gen_fp)->initialSetup();

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
TEST_F(TabulatedBicubicFluidPropertiesTest, passthrough)
{
  Real p = 1.5e6;
  Real T = 450.0;
  const Real tol = REL_TOL_SAVED_VALUE;

  // As the flags for interpolation in TabulatedBicubicFluidProperties default to false,
  // properties will be passed through to the given fluid properties object
  ABS_TEST(_tab_pT_from_fp->rho_from_p_T(p, T), _co2_fp->rho_from_p_T(p, T), tol);
  ABS_TEST(_tab_pT_from_fp->h_from_p_T(p, T), _co2_fp->h_from_p_T(p, T), tol);
  ABS_TEST(_tab_pT_from_fp->e_from_p_T(p, T), _co2_fp->e_from_p_T(p, T), tol);
  ABS_TEST(_tab_pT_from_fp->mu_from_p_T(p, T), _co2_fp->mu_from_p_T(p, T), tol);
  ABS_TEST(_tab_pT_from_fp->k_from_p_T(p, T), _co2_fp->k_from_p_T(p, T), tol);
  ABS_TEST(_tab_pT_from_fp->cp_from_p_T(p, T), _co2_fp->cp_from_p_T(p, T), tol);
  ABS_TEST(_tab_pT_from_fp->cv_from_p_T(p, T), _co2_fp->cv_from_p_T(p, T), tol);
  ABS_TEST(_tab_pT_from_fp->s_from_p_T(p, T), _co2_fp->s_from_p_T(p, T), tol);

  // These calls are always forwarded to the 'fp' parameter fluid properties because the
  // tabulations are not implemented
  ABS_TEST(_tab_pT_from_fp->henryCoefficients()[0], _co2_fp->henryCoefficients()[0], tol);
  ABS_TEST(_tab_pT_from_fp->henryCoefficients()[1], _co2_fp->henryCoefficients()[1], tol);
  ABS_TEST(_tab_pT_from_fp->henryCoefficients()[2], _co2_fp->henryCoefficients()[2], tol);
  ABS_TEST(_tab_pT_from_fp->triplePointPressure(), _co2_fp->triplePointPressure(), tol);
  ABS_TEST(_tab_pT_from_fp->triplePointTemperature(), _co2_fp->triplePointTemperature(), tol);
  ABS_TEST(_tab_pT_from_fp->criticalPressure(), _co2_fp->criticalPressure(), tol);
  ABS_TEST(_tab_pT_from_fp->criticalTemperature(), _co2_fp->criticalTemperature(), tol);
  ABS_TEST(_tab_pT_from_fp->criticalDensity(), _co2_fp->criticalDensity(), tol);

  // Use a temperature less than the critical point
  T = 300.0;
  ABS_TEST(_tab_pT_from_fp->vaporPressure(T), _co2_fp->vaporPressure(T), tol);

  // TODO: these properties are not implemented in CO2 fp so we cannot test the pass through
  // T_from_p_h
  // vaporPressure with saturation pressure
  // vaporTemperature
}

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(TabulatedBicubicFluidPropertiesTest, fluidName)
{
  EXPECT_EQ(_tab_pT_from_fp->fluidName(), "co2");
}

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(TabulatedBicubicFluidPropertiesTest, molarMass)
{
  ABS_TEST(_tab_pT_from_fp->molarMass(), 44.0098e-3, REL_TOL_SAVED_VALUE);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(TabulatedBicubicFluidPropertiesTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;
  const Real tol = REL_TOL_CONSISTENCY;

  // Single property methods
  Real rho, drho_dp, drho_dT;
  _tab_pT_from_fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real mu, dmu_dp, dmu_dT;
  _tab_pT_from_fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  Real e, de_dp, de_dT;
  _tab_pT_from_fp->e_from_p_T(p, T, e, de_dp, de_dT);

  // Combined property methods
  Real rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT, e2, de2_dp, de2_dT;
  _tab_pT_from_fp->rho_mu_from_p_T(p, T, rho2, mu2);

  ABS_TEST(rho, rho2, tol);
  ABS_TEST(mu, mu2, tol);

  _tab_pT_from_fp->rho_mu_from_p_T(p, T, rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(mu, mu2, tol);
  ABS_TEST(dmu_dp, dmu2_dp, tol);
  ABS_TEST(dmu_dT, dmu2_dT, tol);

  _tab_pT_from_fp->rho_e_from_p_T(p, T, rho2, drho2_dp, drho2_dT, e2, de2_dp, de2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(e, e2, tol);
  ABS_TEST(de_dp, de2_dp, tol);
  ABS_TEST(de_dT, de2_dT, tol);
}

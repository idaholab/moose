//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADReal.h"
#include "Moose.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "SodiumSaturationFluidPropertiesTest.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(SodiumSaturationFluidPropertiesTest, fluidName)
{
  EXPECT_EQ(_fp->fluidName(), "sodium_sat");
}

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(SodiumSaturationFluidPropertiesTest, molarMass)
{
  ABS_TEST(_fp->molarMass(), 22.989769E-3, REL_TOL_SAVED_VALUE);
}

/**
 * Test that the thermal conductivity and its derivatives are
 * correctly computed.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, thermalConductivity)
{
  const Real T = 800.0;
  const Real p = 101325;

  // Fink and Leibowitz (1979) list conductivity as 66.8 at 800 K, so the fit agrees well
  REL_TEST(_fp->k_from_p_T(p, T), 66.9752096, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->k_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the viscosity and its derivatives are correctly computed.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, viscosity)
{
  const Real T = 800.0;
  const Real p = 101325;

  // Fink and Leibowitz (1979) list viscosity as 0.000229 at 800 K, so the fit agrees well
  REL_TEST(_fp->mu_from_p_T(p, T), 0.00022907910937500003, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->mu_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isobaric specific heat and its derivatives are correctly computed.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, isobaricSpecificHeat)
{
  const Real T1 = 400.0;
  const Real T2 = 800.0;
  const Real p = 1e5;

  // Fink and Leibowitz (1979) show Cp = 1373.6 at 400 K, so the fit agrees well
  REL_TEST(_fp->cp_from_p_T(p, T1), 1383.985792, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_p_T, 5e5, T1, REL_TOL_DERIVATIVE);

  // Fink and Leibowitz (1979) show Cp = 1263.6 at 800 K, so the fit agrees well
  REL_TEST(_fp->cp_from_p_T(p, T2), 1260.719872, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cp_from_p_T, 5e5, T2, REL_TOL_DERIVATIVE);
}

/**
 * Test that the isochoric specific heat and its derivatives are correctly computed.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, isochoricSpecificHeat)
{
  const Real T1 = 400.0;
  const Real T2 = 800.0;
  const Real p = 101325;

  // Fink and Leibowitz (1979) show Cv = 1230.1 at 400 K, so the fit agrees well
  REL_TEST(_fp->cv_from_p_T(p, T1), 1222.9660159999999, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cv_from_p_T, p, T1, REL_TOL_DERIVATIVE);

  // Fink and Leibowitz (1979) show Cv = 982.611 at 800 K, so the fit agrees well
  REL_TEST(_fp->cv_from_p_T(p, T2), 986.2385279999999, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->cv_from_p_T, p, T2, REL_TOL_DERIVATIVE);
}

/**
 * Test that the density and its derivatives are correctly computed.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, density)
{
  const Real Tm = 370.98;
  const Real T = 800.0;
  const Real Tb = 1156.5;
  const Real p = 101325;

  // Fink and Leibowitz (1979) list density at 370.98 K is 927.3, so the fit agrees well
  ABS_TEST(_fp->rho_from_p_T(p, Tm), 923.3571594322216, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->rho_from_p_T, p, Tm, REL_TOL_DERIVATIVE);
  ABS_TEST(_fp->v_from_p_T(p, Tm), 0.001083004544649772, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->v_from_p_T, p, Tm, REL_TOL_DERIVATIVE);

  // Fink and Leibowitz (1979) list density at 800 K is 825.6, so the fit agrees well
  ABS_TEST(_fp->rho_from_p_T(p, T), 826.04056, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);
  ABS_TEST(_fp->v_from_p_T(p, T), 0.0012105943078630425, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->v_from_p_T, p, T, REL_TOL_DERIVATIVE);

  // Fink and Leibowitz (1979) list density at 1156.5 K is 739.4, so the fit agrees well
  ABS_TEST(_fp->rho_from_p_T(p, Tb), 742.0807106065, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->rho_from_p_T, p, Tb, REL_TOL_DERIVATIVE);
  ABS_TEST(_fp->v_from_p_T(p, Tb), 0.0013475623145933863, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->v_from_p_T, p, Tb, REL_TOL_DERIVATIVE);
}

/**
 * Test that the specific internal energy and its derivatives are correctly computed.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, specificInternalEnergy)
{
  const Real T = 800.0;
  const Real p = 101325;

  ABS_TEST(_fp->e_from_p_T(p, T),
           _fp->h_from_p_T(p, T) - p * _fp->v_from_p_T(p, T),
           REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->e_from_p_T, p, T, REL_TOL_DERIVATIVE);
}

/**
 * Test that the specific enthalpy and its derivatives are correctly computed.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, specificEnthalpy)
{
  const Real T = 800.0;
  const Real p = 1e5;

  // Fink and Leibowitz (1979) compute enthalpy at 800 K as 768602, so the fit agrees well
  ABS_TEST(_fp->h_from_p_T(p, T), 767034.6715200001, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->h_from_p_T, p, T, REL_TOL_DERIVATIVE);

  // Fink and Leibowitz (1979) compute enthalpy at 1156.5 K as 1218803, so the fit agrees well
  const Real T2 = 1156.5;
  ABS_TEST(_fp->h_from_p_T(p, T2), 1218897.6776327428, REL_TOL_SAVED_VALUE);
  DERIV_TEST(_fp->h_from_p_T, p, T2, REL_TOL_DERIVATIVE);
}

/**
 * Test the first-order pressure extension and its thermodynamic derivatives.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, pressureExtension)
{
  const Real T = 800.0;
  const Real p0 = 1e5;
  const Real p = 5e5;

  Real v, dv_dp, dv_dT;
  _fp->v_from_p_T(p, T, v, dv_dp, dv_dT);

  Real h, dh_dp, dh_dT;
  _fp->h_from_p_T(p, T, h, dh_dp, dh_dT);
  const Real h0 = _fp->h_from_p_T(p0, T);

  Real s, ds_dp, ds_dT;
  _fp->s_from_p_T(p, T, s, ds_dp, ds_dT);

  DERIV_TEST(_fp->h_from_p_T, p, T, REL_TOL_DERIVATIVE);
  REL_TEST(h - h0, (p - p0) * (v - T * dv_dT), REL_TOL_CONSISTENCY);
  REL_TEST(dh_dp, v - T * dv_dT, REL_TOL_DERIVATIVE);
  REL_TEST(dh_dT, _fp->cp_from_p_T(p, T), REL_TOL_DERIVATIVE);
  REL_TEST(ds_dp, (dh_dp - v) / T, REL_TOL_DERIVATIVE);
  REL_TEST(ds_dT, dh_dT / T, REL_TOL_DERIVATIVE);
}

/**
 * Test inversion of specific enthalpy to temperature.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, temperatureFromPressureEnthalpy)
{
  const Real p = 5e5;
  const Real T = 800.0;
  const Real h = _fp->h_from_p_T(p, T);

  REL_TEST(_fp->T_from_p_h(p, h), T, REL_TOL_CONSISTENCY);
  DERIV_TEST(_fp->T_from_p_h, p, h, REL_TOL_DERIVATIVE);
}

/**
 * Test that the specific entropy and its derivatives are correctly computed.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, specificEntropy)
{
  const Real reference_T = 370.98;
  const Real reference_p = 1e5;
  ABS_TEST(_fp->s_from_p_T(reference_p, reference_T), 0.0, TOLERANCE);

  const Real T = 800.0;
  const Real p = 5e5;
  const Real v = _fp->v_from_p_T(p, T);
  const Real e = _fp->e_from_p_T(p, T);
  const Real s = _fp->s_from_p_T(p, T);
  REL_TEST(_fp->s_from_v_e(v, e), s, REL_TOL_CONSISTENCY);
  REL_TEST(_fp->rho_from_p_s(p, s), 1 / v, REL_TOL_CONSISTENCY);

  DERIV_TEST(_fp->s_from_p_T, p, T, REL_TOL_DERIVATIVE);
  // Larger perturbations keep the finite differences above cancellation and inversion noise.
  DERIV_TEST_CUSTOM_PERTURBATION(_fp->s_from_v_e, v, e, REL_TOL_DERIVATIVE, 2e-6);
  DERIV_TEST_CUSTOM_PERTURBATION(_fp->rho_from_p_s, p, s, REL_TOL_DERIVATIVE, 1e-5);
}

/**
 * Test that the pressure can be computed from the specific volume & energy
 */
TEST_F(SodiumSaturationFluidPropertiesTest, PTFromVE)
{
  // Starting pressure and temperature
  const Real T = 800.0;
  const Real p = 5e5;

  // Obtain specific volume and specific energy
  const Real v = _fp->v_from_p_T(p, T);
  const Real e = _fp->e_from_p_T(p, T);

  // Verify that the conversion back succeeds
  ABS_TEST(_fp->T_from_v_e(v, e), T, TOLERANCE);
  // The pressure inversion contains cancellation between the reference enthalpy and energy;
  // this tolerance is still sub-millipascal relative to the 500000 Pa state.
  ABS_TEST(_fp->p_from_v_e(v, e), p, 1e-5);

  // Verify the remaining (v,e) properties required by compressible-flow applications
  REL_TEST(_fp->c_from_v_e(v, e), 2301.5361599999997, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp->cp_from_v_e(v, e), _fp->cp_from_p_T(p, T), REL_TOL_CONSISTENCY);
  REL_TEST(_fp->cv_from_v_e(v, e), _fp->cv_from_p_T(p, T), REL_TOL_CONSISTENCY);
  REL_TEST(_fp->mu_from_v_e(v, e), _fp->mu_from_p_T(p, T), REL_TOL_CONSISTENCY);
  REL_TEST(_fp->k_from_v_e(v, e), _fp->k_from_p_T(p, T), REL_TOL_CONSISTENCY);
  REL_TEST(_fp->e_from_p_rho(p, 1 / v), e, REL_TOL_CONSISTENCY);

  DERIV_TEST(_fp->p_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->T_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->c_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->cp_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->cv_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->mu_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->k_from_v_e, v, e, REL_TOL_DERIVATIVE);
  DERIV_TEST(_fp->e_from_p_rho, p, 1 / v, REL_TOL_DERIVATIVE);
}

/**
 * Test automatic differentiation of the (v,e) properties used by compressible-flow applications.
 */
TEST_F(SodiumSaturationFluidPropertiesTest, volumeEnergyAutomaticDifferentiation)
{
  const Real p = 5e5;
  const Real T = 800.0;
  const Real v = _fp->v_from_p_T(p, T);
  const Real e = _fp->e_from_p_T(p, T);
  const auto & fp = static_cast<const SinglePhaseFluidProperties &>(*_fp);

  DNDerivativeType dv;
  DNDerivativeType de;
  Moose::derivInsert(dv, 0, 1);
  Moose::derivInsert(dv, 1, 0);
  Moose::derivInsert(de, 0, 0);
  Moose::derivInsert(de, 1, 1);
  const ADReal v_ad(v, dv);
  const ADReal e_ad(e, de);

  const auto check_ad_derivatives =
      [](const ADReal & ad_value, Real value, Real dvalue_dv, Real dvalue_de)
  {
    EXPECT_DOUBLE_EQ(ad_value.value(), value);
    EXPECT_DOUBLE_EQ(ad_value.derivatives()[0], dvalue_dv);
    EXPECT_DOUBLE_EQ(ad_value.derivatives()[1], dvalue_de);
  };

  Real value;
  Real dvalue_dv;
  Real dvalue_de;

  _fp->p_from_v_e(v, e, value, dvalue_dv, dvalue_de);
  check_ad_derivatives(fp.p_from_v_e(v_ad, e_ad), value, dvalue_dv, dvalue_de);

  _fp->T_from_v_e(v, e, value, dvalue_dv, dvalue_de);
  check_ad_derivatives(fp.T_from_v_e(v_ad, e_ad), value, dvalue_dv, dvalue_de);

  _fp->c_from_v_e(v, e, value, dvalue_dv, dvalue_de);
  check_ad_derivatives(fp.c_from_v_e(v_ad, e_ad), value, dvalue_dv, dvalue_de);

  _fp->cp_from_v_e(v, e, value, dvalue_dv, dvalue_de);
  check_ad_derivatives(fp.cp_from_v_e(v_ad, e_ad), value, dvalue_dv, dvalue_de);

  _fp->cv_from_v_e(v, e, value, dvalue_dv, dvalue_de);
  check_ad_derivatives(fp.cv_from_v_e(v_ad, e_ad), value, dvalue_dv, dvalue_de);

  _fp->mu_from_v_e(v, e, value, dvalue_dv, dvalue_de);
  check_ad_derivatives(fp.mu_from_v_e(v_ad, e_ad), value, dvalue_dv, dvalue_de);

  _fp->k_from_v_e(v, e, value, dvalue_dv, dvalue_de);
  check_ad_derivatives(fp.k_from_v_e(v_ad, e_ad), value, dvalue_dv, dvalue_de);

  _fp->s_from_v_e(v, e, value, dvalue_dv, dvalue_de);
  check_ad_derivatives(fp.s_from_v_e(v_ad, e_ad), value, dvalue_dv, dvalue_de);
}

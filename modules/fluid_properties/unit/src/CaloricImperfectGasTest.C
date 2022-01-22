//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CaloricImperfectGasTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Verify that the fluid name is correctly returned
 */
TEST_F(CaloricImperfectGasTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "caloric_imperfect_gas"); }

TEST_F(CaloricImperfectGasTest, testAll)
{
  const_cast<CaloricImperfectGas *>(_fp)->initialSetup();

  Real min_T = 100.0;
  Real max_T = 500.0;
  unsigned int np = 100;
  Real dT = (max_T - min_T) / ((Real)np - 1.0);

  // Test the T -> e, h -> T rountrip lookup
  {
    for (unsigned int j = 0; j < np; ++j)
    {
      // the 1 arguments are not used
      Real T = min_T + j * dT;
      Real h = _fp->h_from_p_T(1, T);
      Real e = _fp->e_from_p_T(1, T);
      Real TT1 = _fp->T_from_v_e(1, e);
      Real TT2 = _fp->T_from_p_h(1, h);
      REL_TEST(T, TT1, 1e-5);
      REL_TEST(T, TT2, 1e-5);
    }
  }

  // check e/h lookups for T = 325.0 & e_from_v_h
  {
    Real T = 325.0;
    Real p = 1.0e6;
    Real h = 17156400 * 0.75 + 28594000 * 0.25;
    Real e = 10342290 * 0.75 + 17034360 * 0.25;

    Real h_pT = _fp->h_from_p_T(p, T);
    Real e_pT = _fp->e_from_p_T(p, T);
    REL_TEST(h, h_pT, 1e-5);
    REL_TEST(e, e_pT, 1e-5);

    Real v = _fp->v_from_p_T(p, T);
    Real e_vh = _fp->e_from_v_h(v, h);
    REL_TEST(e, e_vh, 1e-5);
  }

  // check cv/cp lookups for T = 325.0
  {
    Real T = 325.0;
    Real p = 1.0e6;
    Real cp = 100079 * 0.75 + 128673 * 0.25;
    Real cv = 58809.1 * 0.75 + 75032.3 * 0.25;

    Real cp_pT = _fp->cp_from_p_T(p, T);
    Real cv_pT = _fp->cv_from_p_T(p, T);
    REL_TEST(cp, cp_pT, 1e-5);
    REL_TEST(cv, cv_pT, 1e-5);

    Real v = _fp->v_from_p_T(p, T);
    Real e = _fp->e_from_p_T(p, T);
    Real cp_ve = _fp->cp_from_v_e(v, e);
    Real cv_ve = _fp->cv_from_v_e(v, e);
    REL_TEST(cp, cp_ve, 1e-5);
    REL_TEST(cv, cv_ve, 1e-5);
  }

  // test mu(p, T), mu(v, e), k(p, T), k(v, e) & derivatives
  {
    Real T = 300.0;
    Real p = 1.0e6;
    REL_TEST(_fp->mu_from_p_T(p, T), 3.5, 1e-5);
    REL_TEST(_fp->k_from_p_T(p, T), 6.0, 1e-5);

    Real v = _fp->v_from_p_T(p, T);
    Real e = _fp->e_from_p_T(p, T);
    REL_TEST(_fp->mu_from_v_e(v, e), 3.5, 0.001);
    REL_TEST(_fp->k_from_v_e(v, e), 6.0, 0.001);

    Real mu, dmu_dp, dmu_dT;
    _fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
    REL_TEST(dmu_dT, 0.0125, 1e-5);

    Real k, dk_dp, dk_dT;
    _fp->k_from_p_T(p, T, k, dk_dp, dk_dT);
    REL_TEST(dk_dT, -0.025, 1e-5);

    Real cv = _fp->cv_from_p_T(p, T);
    Real dmu_dv, dmu_de;
    _fp->mu_from_v_e(v, e, mu, dmu_dv, dmu_de);
    REL_TEST(dmu_de, 0.0125 / cv, 0.001);

    Real dk_dv, dk_de;
    _fp->k_from_v_e(v, e, k, dk_dv, dk_de);
    REL_TEST(dk_de, -0.025 / cv, 0.001);
  }

  // test derivatives against FD including gamma
  {

  }

  // test out of bounds call for temperature
  {
    try
    {
      _fp->cp_from_p_T(1.0e6, 600.0);
      FAIL();
    }
    catch (const std::exception & err)
    {
      std::size_t pos = std::string(err.what()).find("Out of bounds.");
      ASSERT_TRUE(pos != std::string::npos);
    }

    try
    {
      _fp->cp_from_p_T(1.0e6, 99.0);
      FAIL();
    }
    catch (const std::exception & err)
    {
      std::size_t pos = std::string(err.what()).find("Out of bounds.");
      ASSERT_TRUE(pos != std::string::npos);
    }
  }

  // h(T) is not monotonically increasing
  {
    try
    {
      const_cast<CaloricImperfectGas *>(_fp_bad_h_fn)->initialSetup();
      FAIL();
    }
    catch (const std::exception & err)
    {
      std::size_t pos =
          std::string(err.what()).find("e(T) or h(T) are not monotonically increasing with T");
      ASSERT_TRUE(pos != std::string::npos);
    }

    try
    {
      const_cast<CaloricImperfectGas *>(_fp_bad_h_fn2)->initialSetup();
      FAIL();
    }
    catch (const std::exception & err)
    {
      std::size_t pos =
          std::string(err.what()).find("e(T) or h(T) are not monotonically increasing with T");
      ASSERT_TRUE(pos != std::string::npos);
    }
  }
}

TEST_F(CaloricImperfectGasTest, compareWithIdeal)
{
  const_cast<CaloricImperfectGas *>(_compare_with_ideal)->initialSetup();

  Real T = 325.0;
  Real p = 1.0e6;

  // test all functions that provide x_from_p_T, x_from_v_e, e_from_v_h, e_from_p_rho
  {
    Real v = _ideal->v_from_p_T(p, T);
    Real e = _ideal->e_from_p_T(p, T);
    Real rho = _ideal->rho_from_p_T(p, T);
    Real h = _ideal->h_from_p_T(p, T);
    REL_TEST(_ideal->c_from_p_T(p, T), _compare_with_ideal->c_from_p_T(p, T), 1e-5);
    REL_TEST(_ideal->rho_from_p_T(p, T), _compare_with_ideal->rho_from_p_T(p, T), 1e-5);
    REL_TEST(_ideal->h_from_p_T(p, T), _compare_with_ideal->h_from_p_T(p, T), 1e-5);
    REL_TEST(_ideal->e_from_p_T(p, T), _compare_with_ideal->e_from_p_T(p, T), 1e-5);
    REL_TEST(_ideal->cv_from_p_T(p, T), _compare_with_ideal->cv_from_p_T(p, T), 1e-5);
    REL_TEST(_ideal->cp_from_p_T(p, T), _compare_with_ideal->cp_from_p_T(p, T), 1e-5);
    REL_TEST(_ideal->T_from_v_e(v, e), _compare_with_ideal->T_from_v_e(v, e), 1e-5);
    REL_TEST(_ideal->c_from_v_e(v, e), _compare_with_ideal->c_from_v_e(v, e), 1e-5);
    REL_TEST(_ideal->cv_from_v_e(v, e), _compare_with_ideal->cv_from_v_e(v, e), 1e-5);
    REL_TEST(_ideal->cp_from_v_e(v, e), _compare_with_ideal->cp_from_v_e(v, e), 1e-5);
    REL_TEST(_ideal->e_from_v_h(v, h), _compare_with_ideal->e_from_v_h(v, h), 1e-5);
    REL_TEST(_ideal->e_from_p_rho(p, rho), _compare_with_ideal->e_from_p_rho(p, rho), 1e-5);
    REL_TEST(_ideal->e_from_T_v(T, v), _compare_with_ideal->e_from_T_v(T, v), 1e-5);
    REL_TEST(_ideal->p_from_T_v(T, v), _compare_with_ideal->p_from_T_v(T, v), 1e-5);
    REL_TEST(_ideal->gamma_from_p_T(p, T), _compare_with_ideal->gamma_from_p_T(p, T), 1e-5);
    REL_TEST(_ideal->gamma_from_v_e(v, e), _compare_with_ideal->gamma_from_v_e(v, e), 1e-5);
  }

  // test derivatives for c_from_v_e
  {
    Real v = _ideal->v_from_p_T(p, T);
    Real e = _ideal->e_from_p_T(p, T);
    Real c = 0, dc_dv = 0, dc_de = 0;
    _ideal->c_from_v_e(v, e, c, dc_dv, dc_de);
    Real c2 = 0, dc_dv2 = 0, dc_de2 = 0;
    _compare_with_ideal->c_from_v_e(v, e, c2, dc_dv2, dc_de2);
    REL_TEST(dc_dv, dc_dv2, 1e-5);
    REL_TEST(dc_de, dc_de2, 1e-5);
  }

  // test derivatives for p_from_v_e
  {
    Real v = _ideal->v_from_p_T(p, T);
    Real e = _ideal->e_from_p_T(p, T);
    Real pp = 0, dp_dv = 0, dp_de = 0;
    _ideal->p_from_v_e(v, e, pp, dp_dv, dp_de);
    Real pp2 = 0, dp_dv2 = 0, dp_de2 = 0;
    _compare_with_ideal->p_from_v_e(v, e, pp2, dp_dv2, dp_de2);
    REL_TEST(dp_dv, dp_dv2, 1e-5);
    REL_TEST(dp_de, dp_de2, 1e-5);
  }

  // test derivatives for T_from_v_e
  {
    Real v = _ideal->v_from_p_T(p, T);
    Real e = _ideal->e_from_p_T(p, T);
    Real TT = 0, dT_dv = 0, dT_de = 0;
    _ideal->T_from_v_e(v, e, TT, dT_dv, dT_de);
    Real TT2 = 0, dT_dv2 = 0, dT_de2 = 0;
    _compare_with_ideal->T_from_v_e(v, e, TT2, dT_dv2, dT_de2);
    REL_TEST(dT_dv, dT_dv2, 1e-5);
    REL_TEST(dT_de, dT_de2, 1e-5);
  }

  // test derivatives for e_from_v_h
  {
    Real v = _ideal->v_from_p_T(p, T);
    Real h = _ideal->h_from_p_T(p, T);
    Real ee = 0, de_dv = 0, de_dh = 0;
    _ideal->e_from_v_h(v, h, ee, de_dv, de_dh);
    Real ee2 = 0, de_dv2 = 0, de_dh2 = 0;
    _compare_with_ideal->e_from_v_h(v, h, ee2, de_dv2, de_dh2);
    REL_TEST(de_dv, de_dv2, 1e-5);
    REL_TEST(de_dh, de_dh2, 1e-5);
  }
  // rho_from_p_T
  // e_from_p_rho
  // e_from_T_v
  // p_from_T_v
  // h_from_T_v
  // h_from_p_T
  // e_from_p_T
  // T_from_p_h
}

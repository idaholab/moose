//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CaloricallyImperfectGasTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Verify that the fluid name is correctly returned
 */
TEST_F(CaloricallyImperfectGasTest, testAll)
{
  const CaloricallyImperfectGas * fp;
  std::vector<Real> T = {100, 200, 300, 400, 500};
  std::vector<Real> e = {1825110, 5272540, 10342290, 17034360, 25348750};
  std::vector<Real> T2 = {100, 500};
  std::vector<Real> mu = {1, 6};
  std::vector<Real> k = {11, 1};

  InputParameters e_fn_params = _factory.getValidParams("PiecewiseLinear");
  e_fn_params.set<std::vector<Real>>("x") = T;
  e_fn_params.set<std::vector<Real>>("y") = e;
  _fe_problem->addFunction("PiecewiseLinear", "e_fn", e_fn_params);

  InputParameters mu_fn_params = _factory.getValidParams("PiecewiseLinear");
  mu_fn_params.set<std::vector<Real>>("x") = T2;
  mu_fn_params.set<std::vector<Real>>("y") = mu;
  _fe_problem->addFunction("PiecewiseLinear", "mu_fn", mu_fn_params);

  InputParameters k_fn_params = _factory.getValidParams("PiecewiseLinear");
  k_fn_params.set<std::vector<Real>>("x") = T2;
  k_fn_params.set<std::vector<Real>>("y") = k;
  _fe_problem->addFunction("PiecewiseLinear", "k_fn", k_fn_params);

  InputParameters uo_pars = _factory.getValidParams("CaloricallyImperfectGas");
  uo_pars.set<Real>("molar_mass") = 0.002;
  uo_pars.set<FunctionName>("e") = "e_fn";
  uo_pars.set<FunctionName>("mu") = "mu_fn";
  uo_pars.set<FunctionName>("k") = "k_fn";
  uo_pars.set<Real>("min_temperature") = 100.0;
  uo_pars.set<Real>("max_temperature") = 500.0;
  uo_pars.set<Real>("temperature_resolution") = 0.01;
  _fe_problem->addUserObject("CaloricallyImperfectGas", "fp", uo_pars);
  fp = &_fe_problem->getUserObject<CaloricallyImperfectGas>("fp");

  const_cast<CaloricallyImperfectGas *>(fp)->initialSetup();

  Real min_T = 100.0;
  Real max_T = 500.0;
  unsigned int np = 200;
  Real dT = (max_T - min_T) / ((Real)np - 1.0);
  Real Ru = 8.31446261815324;
  Real Rs = Ru / 0.002;

  // Consistency checks
  {
    for (unsigned int j = 1; j < np - 1; ++j)
    {
      Real T = min_T + j * dT;
      Real p = 1.0e6;
      const Real rho = fp->rho_from_p_T(p, T);
      const Real v = 1.0 / rho;
      Real h = fp->h_from_p_T(p, T);
      Real e = fp->e_from_p_T(p, T);
      Real cp = fp->cp_from_p_T(p, T);
      Real cv = fp->cv_from_p_T(p, T);
      Real k = fp->k_from_p_T(p, T);
      Real mu = fp->mu_from_p_T(p, T);
      Real s = fp->s_from_p_T(p, T);

      // test rho_from_p_s
      REL_TEST(fp->rho_from_p_s(p, s), rho, 10.0 * REL_TOL_CONSISTENCY);

      // test e_from_x_y functions
      REL_TEST(fp->e_from_v_h(v, h), e, 10.0 * REL_TOL_CONSISTENCY);
      REL_TEST(fp->e_from_p_rho(p, rho), e, 10.0 * REL_TOL_CONSISTENCY);
      REL_TEST(fp->e_from_T_v(T, v), e, 10.0 * REL_TOL_CONSISTENCY);

      // test cv_x_y  functions
      REL_TEST(fp->cv_from_T_v(T, v), cv, 10.0 * REL_TOL_CONSISTENCY);
      REL_TEST(fp->cv_from_v_e(v, e), cv, 10.0 * REL_TOL_CONSISTENCY);

      // test cp_x_y  functions
      REL_TEST(fp->cp_from_v_e(v, e), cp, 10.0 * REL_TOL_CONSISTENCY);

      // test h_from_x_y functions
      REL_TEST(fp->h_from_T_v(T, v), h, 10.0 * REL_TOL_CONSISTENCY);

      // test p_from_x_y functions
      REL_TEST(fp->p_from_v_e(v, e), p, 10.0 * REL_TOL_CONSISTENCY);
      REL_TEST(fp->p_from_T_v(T, v), p, 10.0 * REL_TOL_CONSISTENCY);

      // test T_from_x_y functions
      REL_TEST(fp->T_from_p_h(p, h), T, 10.0 * REL_TOL_CONSISTENCY);
      REL_TEST(fp->T_from_v_e(v, e), T, 10.0 * REL_TOL_CONSISTENCY);

      // test k_from_x_y functions
      REL_TEST(fp->k_from_v_e(v, e), k, 10.0 * REL_TOL_CONSISTENCY);

      // test mu_from_x_y functions
      REL_TEST(fp->mu_from_v_e(v, e), mu, 10.0 * REL_TOL_CONSISTENCY);
    }
  }

  // AD consistency checks
  {
    for (unsigned int j = 1; j < np - 1; ++j)
    {
      Real T = min_T + j * dT;
      Real p = 1.0e6;
      const Real rho = fp->rho_from_p_T(p, T);
      const Real v = 1.0 / rho;
      Real e = fp->e_from_p_T(p, T);
      Real cv = fp->cv_from_p_T(p, T);
      Real c = fp->c_from_p_T(p, T);

      ADReal ad_e = e;
      ADReal ad_v = v;
      ADReal ad_p = fp->p_from_v_e(ad_v, ad_e);
      Real tol = 1e-11;
      REL_TEST(MetaPhysicL::raw_value(ad_p), p, tol);

      ADReal ad_T = fp->T_from_v_e(ad_v, ad_e);
      REL_TEST(MetaPhysicL::raw_value(ad_T), T, tol);

      REL_TEST(MetaPhysicL::raw_value(fp->cv_from_v_e(ad_v, ad_e)), cv, tol);

      ADReal ad_rho = fp->rho_from_p_T(ad_p, ad_T);
      REL_TEST(MetaPhysicL::raw_value(ad_rho), rho, tol);

      REL_TEST(MetaPhysicL::raw_value(fp->e_from_p_rho(ad_p, ad_rho)), e, tol);
      REL_TEST(MetaPhysicL::raw_value(fp->e_from_T_v(ad_T, ad_v)), e, tol);
      REL_TEST(MetaPhysicL::raw_value(fp->e_from_p_T(ad_p, ad_T)), e, tol);
      REL_TEST(
          MetaPhysicL::raw_value(fp->gamma_from_v_e(ad_v, ad_e)), fp->gamma_from_v_e(v, e), tol);
      REL_TEST(MetaPhysicL::raw_value(fp->gamma_from_p_T(ad_p, ad_T)),
               fp->gamma_from_p_T(ad_p, ad_T),
               tol);
      REL_TEST(MetaPhysicL::raw_value(fp->c_from_v_e(ad_v, ad_e)), c, tol);
      REL_TEST(MetaPhysicL::raw_value(fp->c_from_p_T(ad_p, ad_T)), c, tol);
    }
  }

  // check e/h lookups for T = 325.0 & e_from_v_h
  {
    Real T = 325.0;
    Real p = 1.0e6;
    Real e = 10342290 * 0.75 + 17034360 * 0.25;
    Real h = e + p / fp->rho_from_p_T(p, T);
    Real v = fp->v_from_p_T(p, T);

    REL_TEST(h, fp->h_from_p_T(p, T), 1e-5);
    REL_TEST(e, fp->e_from_p_T(p, T), 1e-5);
    REL_TEST(e, fp->e_from_v_h(v, h), 1e-5);
  }

  // check cv/cp lookups for T = 325.0
  {
    Real T = 325.0;
    Real p = 1.0e6;
    Real cv = (17034360.0 - 10342290.0) / 100.0;
    Real cp = cv + 8.3144598 / 0.002;
    REL_TEST(cp, fp->cp_from_p_T(p, T), 1e-5);
    REL_TEST(cv, fp->cv_from_p_T(p, T), 1e-5);

    Real v = fp->v_from_p_T(p, T);
    Real e = fp->e_from_p_T(p, T);
    REL_TEST(cp, fp->cp_from_v_e(v, e), 1e-5);
    REL_TEST(cv, fp->cv_from_v_e(v, e), 1e-5);
  }

  // test mu(p, T), mu(v, e), k(p, T), k(v, e) & derivatives
  {
    Real T = 250.0;
    Real p = 1.0e6;
    REL_TEST(fp->mu_from_p_T(p, T), 2.875, 1e-5);
    REL_TEST(fp->k_from_p_T(p, T), 7.25, 1e-5);

    Real v = fp->v_from_p_T(p, T);
    Real e = fp->e_from_p_T(p, T);
    REL_TEST(fp->mu_from_v_e(v, e), 2.875, 0.001);
    REL_TEST(fp->k_from_v_e(v, e), 7.25, 0.001);

    Real mu, dmu_dp, dmu_dT;
    fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
    REL_TEST(dmu_dT, 0.0125, 1e-5);

    Real k, dk_dp, dk_dT;
    fp->k_from_p_T(p, T, k, dk_dp, dk_dT);
    REL_TEST(dk_dT, -0.025, 1e-5);

    Real cv = fp->cv_from_p_T(p, T);
    Real dmu_dv, dmu_de;
    fp->mu_from_v_e(v, e, mu, dmu_dv, dmu_de);
    REL_TEST(dmu_de, 0.0125 / cv, 0.001);

    Real dk_dv, dk_de;
    fp->k_from_v_e(v, e, k, dk_dv, dk_de);
    REL_TEST(dk_de, -0.025 / cv, 0.001);
  }

  // 5 argument functions
  {
    Real T = 325.0;
    Real p = 1.0e6;
    Real e = 10342290 * 0.75 + 17034360 * 0.25;
    Real h = e + p / fp->rho_from_p_T(p, T);
    Real v = fp->v_from_p_T(p, T);
    Real rho = 1.0 / v;
    Real cp = fp->cp_from_p_T(p, T);
    Real cv = fp->cv_from_p_T(p, T);
    Real s = fp->s_from_p_T(p, T);
    Real mu = fp->mu_from_v_e(v, e);
    Real k = fp->k_from_v_e(v, e);
    Real gamma = fp->gamma_from_p_T(p, T);
    Real ssound = fp->c_from_p_T(p, T);
    Real tol = 1e-12;

    {
      Real a, b, c;
      fp->p_from_v_e(v, e, a, b, c);
      REL_TEST(a, p, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->p_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->T_from_v_e(v, e, a, b, c);
      REL_TEST(a, T, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->T_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->cp_from_v_e(v, e, a, b, c);
      REL_TEST(a, cp, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->cp_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->cv_from_v_e(v, e, a, b, c);
      REL_TEST(a, cv, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->cv_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->mu_from_v_e(v, e, a, b, c);
      REL_TEST(a, mu, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->mu_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->k_from_v_e(v, e, a, b, c);
      REL_TEST(a, k, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->k_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->s_from_v_e(v, e, a, b, c);
      REL_TEST(a, s, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->s_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->s_from_p_T(p, T, a, b, c);
      REL_TEST(a, s, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->s_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->s_from_h_p(h, p, a, b, c);
      REL_TEST(a, s, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->s_from_h_p, h, p, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->e_from_v_h(v, h, a, b, c);
      REL_TEST(a, e, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->e_from_v_h, v, h, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->rho_from_p_T(p, T, a, b, c);
      REL_TEST(a, rho, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->rho_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->e_from_p_rho(p, rho, a, b, c);
      REL_TEST(a, e, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->e_from_p_rho, p, rho, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->e_from_T_v(T, v, a, b, c);
      REL_TEST(a, e, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->e_from_T_v, T, v, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->p_from_T_v(T, v, a, b, c);
      REL_TEST(a, p, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->p_from_T_v, T, v, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->h_from_T_v(T, v, a, b, c);
      REL_TEST(a, h, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->h_from_T_v, T, v, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->s_from_T_v(T, v, a, b, c);
      REL_TEST(a, s, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->s_from_T_v, T, v, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->h_from_p_T(p, T, a, b, c);
      REL_TEST(a, h, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->h_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->e_from_p_T(p, T, a, b, c);
      REL_TEST(a, e, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->e_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->T_from_p_h(p, h, a, b, c);
      REL_TEST(a, T, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->T_from_p_h, p, h, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->cv_from_p_T(p, T, a, b, c);
      REL_TEST(a, cv, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->cv_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->cp_from_p_T(p, T, a, b, c);
      REL_TEST(a, cp, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->cp_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->mu_from_p_T(p, T, a, b, c);
      REL_TEST(a, mu, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->mu_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->k_from_p_T(p, T, a, b, c);
      REL_TEST(a, k, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->k_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->gamma_from_p_T(p, T, a, b, c);
      REL_TEST(a, gamma, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->gamma_from_p_T, p, T, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->gamma_from_v_e(v, e, a, b, c);
      REL_TEST(a, gamma, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->gamma_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->c_from_v_e(v, e, a, b, c);
      REL_TEST(a, ssound, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->c_from_v_e, v, e, 1e-4, 1e-4);
    }

    {
      Real a, b, c;
      fp->c_from_p_T(p, T, a, b, c);
      REL_TEST(a, ssound, tol);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->c_from_p_T, p, T, 1e-4, 1e-4);
    }
  }

  // test entropy functions
  {
    Real T = 489.305;
    Real Z = 0.5 * (80459 + 80460.7);
    Real p = 1.0e6;
    const Real rho = fp->rho_from_p_T(p, T);
    const Real v = 1.0 / rho;
    const Real v0 = 1.0;
    Real e = fp->e_from_p_T(p, T);
    Real h = fp->h_from_p_T(p, T);
    Real s = Z + Rs * std::log(v / v0);

    REL_TEST(fp->s_from_p_T(p, T), s, 1e-6);
    REL_TEST(fp->s_from_v_e(v, e), s, 1e-6);
    REL_TEST(fp->s_from_h_p(h, p), s, 1e-6);
    REL_TEST(fp->s_from_T_v(T, v), s, 1e-6);

    // test 5 arg functions with derivatives for s(p, T)
    {
      Real s_alt, ds_dp, ds_dT;
      fp->s_from_p_T(p, T, s_alt, ds_dp, ds_dT);
      REL_TEST(s_alt, s, 1e-5);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->s_from_p_T, p, T, 1e-4, 1e-4);
    }

    // test 5 arg functions with derivatives for s(e, v)
    {
      Real s_alt, ds_dv, ds_de;
      fp->s_from_v_e(v, e, s_alt, ds_dv, ds_de);
      REL_TEST(s_alt, s, 1e-5);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->s_from_v_e, v, e, 1e-4, 1e-4);
    }

    // test 5 arg functions with derivatives for s(h, p)
    {
      Real s_alt, ds_dh, ds_dp;
      fp->s_from_h_p(h, p, s_alt, ds_dh, ds_dp);
      REL_TEST(s_alt, s, 1e-5);
      DERIV_TEST_CUSTOM_PERTURBATION(fp->s_from_h_p, h, p, 1e-4, 1e-4);
    }
  }

  // test out of bounds call for temperature
  {
    try
    {
      fp->cp_from_p_T(1.0e6, 600.0);
      FAIL();
    }
    catch (const std::exception & err)
    {
      std::size_t pos = std::string(err.what()).find("which is outside of the bounds of");
      ASSERT_TRUE(pos != std::string::npos);
    }

    try
    {
      fp->cp_from_p_T(1.0e6, 99.0);
      FAIL();
    }
    catch (const std::exception & err)
    {
      std::size_t pos = std::string(err.what()).find("which is outside of the bounds of");
      ASSERT_TRUE(pos != std::string::npos);
    }
  }
}

TEST_F(CaloricallyImperfectGasTest, nonMonotonicError1)
{
  // Testing case where e(T) is not monotonic
  const CaloricallyImperfectGas * fp_bad_e_fn;
  std::vector<Real> T = {100, 200, 300, 400, 500};
  std::vector<Real> bad_e = {2859400, 8578200, 17156400, 8578200, 42891000};
  InputParameters bad_e_fn_params = _factory.getValidParams("PiecewiseLinear");
  bad_e_fn_params.set<std::vector<Real>>("x") = T;
  bad_e_fn_params.set<std::vector<Real>>("y") = bad_e;
  _fe_problem->addFunction("PiecewiseLinear", "bad_e_fn", bad_e_fn_params);

  InputParameters uo_pars_bad_e_fn = _factory.getValidParams("CaloricallyImperfectGas");
  uo_pars_bad_e_fn.set<Real>("molar_mass") = 0.002;
  uo_pars_bad_e_fn.set<FunctionName>("e") = "bad_e_fn";
  uo_pars_bad_e_fn.set<FunctionName>("mu") = "default_fn";
  uo_pars_bad_e_fn.set<FunctionName>("k") = "default_fn";
  uo_pars_bad_e_fn.set<Real>("min_temperature") = 100.0;
  uo_pars_bad_e_fn.set<Real>("max_temperature") = 500.0;
  _fe_problem->addUserObject("CaloricallyImperfectGas", "fp_bad_e_fn", uo_pars_bad_e_fn);
  fp_bad_e_fn = &_fe_problem->getUserObject<CaloricallyImperfectGas>("fp_bad_e_fn");

  try
  {
    const_cast<CaloricallyImperfectGas *>(fp_bad_e_fn)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("e(T) is not monotonically increasing with T");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST_F(CaloricallyImperfectGasTest, nonMonotonicError2)
{
  // test for bad temperature values that violate min/max temperature
  const CaloricallyImperfectGas * fp_bad_e_fn2;
  std::vector<Real> e = {1825110, 5272540, 10342290, 17034360, 25348750};
  std::vector<Real> bad_T = {150, 200, 300, 400, 500};
  InputParameters bad_e_fn_params2 = _factory.getValidParams("PiecewiseLinear");
  bad_e_fn_params2.set<std::vector<Real>>("x") = bad_T;
  bad_e_fn_params2.set<std::vector<Real>>("y") = e;
  _fe_problem->addFunction("PiecewiseLinear", "bad_e_fn2", bad_e_fn_params2);

  InputParameters uo_pars_bad_e_fn2 = _factory.getValidParams("CaloricallyImperfectGas");
  uo_pars_bad_e_fn2.set<Real>("molar_mass") = 0.002;
  uo_pars_bad_e_fn2.set<FunctionName>("e") = "bad_e_fn2";
  uo_pars_bad_e_fn2.set<FunctionName>("mu") = "default_fn";
  uo_pars_bad_e_fn2.set<FunctionName>("k") = "default_fn";
  uo_pars_bad_e_fn2.set<Real>("min_temperature") = 100.0;
  uo_pars_bad_e_fn2.set<Real>("max_temperature") = 500.0;
  _fe_problem->addUserObject("CaloricallyImperfectGas", "fp_bad_e_fn2", uo_pars_bad_e_fn2);
  fp_bad_e_fn2 = &_fe_problem->getUserObject<CaloricallyImperfectGas>("fp_bad_e_fn2");

  try
  {
    const_cast<CaloricallyImperfectGas *>(fp_bad_e_fn2)->initialSetup();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("e(T) is not monotonically increasing with T");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST_F(CaloricallyImperfectGasTest, compareWithIdeal)
{
  const IdealGasFluidProperties * ideal;
  const CaloricallyImperfectGas * compare_with_ideal;
  // compare ideal gas and calorically imperfect gas
  InputParameters ideal_uo_pars = _factory.getValidParams("IdealGasFluidProperties");
  ideal_uo_pars.set<Real>("molar_mass") = 0.002;
  ideal_uo_pars.set<Real>("gamma") = 1.41;
  _fe_problem->addUserObject("IdealGasFluidProperties", "ideal_fp", ideal_uo_pars);
  ideal = &_fe_problem->getUserObject<IdealGasFluidProperties>("ideal_fp");

  Real Ru = 8.31446261815324;
  Real Rs = Ru / 0.002;
  Real gamma = 1.41;
  Real cp_val = gamma / (gamma - 1) * Rs;
  Real cv_val = cp_val / gamma;
  std::vector<Real> temps = {0.0, 500.0};
  std::vector<Real> internal_energies = {0.0, 500.0 * cv_val};

  InputParameters lin_e_fn_params = _factory.getValidParams("PiecewiseLinear");
  lin_e_fn_params.set<std::vector<Real>>("x") = temps;
  lin_e_fn_params.set<std::vector<Real>>("y") = internal_energies;
  _fe_problem->addFunction("PiecewiseLinear", "lin_e_fn", lin_e_fn_params);

  InputParameters compare_uo_pars = _factory.getValidParams("CaloricallyImperfectGas");
  compare_uo_pars.set<Real>("molar_mass") = 0.002;
  compare_uo_pars.set<FunctionName>("e") = "lin_e_fn";
  compare_uo_pars.set<FunctionName>("mu") = "default_fn";
  compare_uo_pars.set<FunctionName>("k") = "default_fn";
  compare_uo_pars.set<Real>("min_temperature") = 100.0;
  compare_uo_pars.set<Real>("max_temperature") = 500.0;
  _fe_problem->addUserObject("CaloricallyImperfectGas", "compare_fp", compare_uo_pars);
  compare_with_ideal = &_fe_problem->getUserObject<CaloricallyImperfectGas>("compare_fp");

  const_cast<CaloricallyImperfectGas *>(compare_with_ideal)->initialSetup();

  Real T = 325.0;
  Real p = 1.0e6;

  // test all functions that provide x_from_p_T, x_from_v_e, e_from_v_h, e_from_p_rho
  {
    Real v = ideal->v_from_p_T(p, T);
    Real e = ideal->e_from_p_T(p, T);
    Real rho = ideal->rho_from_p_T(p, T);
    Real h = ideal->h_from_p_T(p, T);
    REL_TEST(ideal->c_from_p_T(p, T), compare_with_ideal->c_from_p_T(p, T), 1e-5);
    REL_TEST(ideal->rho_from_p_T(p, T), compare_with_ideal->rho_from_p_T(p, T), 1e-5);
    REL_TEST(ideal->h_from_p_T(p, T), compare_with_ideal->h_from_p_T(p, T), 1e-5);
    REL_TEST(ideal->e_from_p_T(p, T), compare_with_ideal->e_from_p_T(p, T), 1e-5);
    REL_TEST(ideal->cv_from_p_T(p, T), compare_with_ideal->cv_from_p_T(p, T), 1e-5);
    REL_TEST(ideal->cp_from_p_T(p, T), compare_with_ideal->cp_from_p_T(p, T), 1e-5);
    REL_TEST(ideal->T_from_v_e(v, e), compare_with_ideal->T_from_v_e(v, e), 1e-5);
    REL_TEST(ideal->c_from_v_e(v, e), compare_with_ideal->c_from_v_e(v, e), 1e-5);
    REL_TEST(ideal->cv_from_v_e(v, e), compare_with_ideal->cv_from_v_e(v, e), 1e-5);
    REL_TEST(ideal->cp_from_v_e(v, e), compare_with_ideal->cp_from_v_e(v, e), 1e-5);
    REL_TEST(ideal->e_from_v_h(v, h), compare_with_ideal->e_from_v_h(v, h), 1e-5);
    REL_TEST(ideal->e_from_p_rho(p, rho), compare_with_ideal->e_from_p_rho(p, rho), 1e-5);
    REL_TEST(ideal->e_from_T_v(T, v), compare_with_ideal->e_from_T_v(T, v), 1e-5);
    REL_TEST(ideal->p_from_T_v(T, v), compare_with_ideal->p_from_T_v(T, v), 1e-5);
    REL_TEST(ideal->gamma_from_p_T(p, T), compare_with_ideal->gamma_from_p_T(p, T), 1e-5);
    REL_TEST(ideal->gamma_from_v_e(v, e), compare_with_ideal->gamma_from_v_e(v, e), 1e-5);
  }

  // test derivatives for c_from_v_e
  {
    Real v = ideal->v_from_p_T(p, T);
    Real e = ideal->e_from_p_T(p, T);
    Real c = 0, dc_dv = 0, dc_de = 0;
    ideal->c_from_v_e(v, e, c, dc_dv, dc_de);
    Real c2 = 0, dc_dv2 = 0, dc_de2 = 0;
    compare_with_ideal->c_from_v_e(v, e, c2, dc_dv2, dc_de2);
    REL_TEST(dc_dv, dc_dv2, 1e-5);
    REL_TEST(dc_de, dc_de2, 1e-5);
  }

  // test derivatives for p_from_v_e
  {
    Real v = ideal->v_from_p_T(p, T);
    Real e = ideal->e_from_p_T(p, T);
    Real pp = 0, dp_dv = 0, dp_de = 0;
    ideal->p_from_v_e(v, e, pp, dp_dv, dp_de);
    Real pp2 = 0, dp_dv2 = 0, dp_de2 = 0;
    compare_with_ideal->p_from_v_e(v, e, pp2, dp_dv2, dp_de2);
    REL_TEST(dp_dv, dp_dv2, 1e-5);
    REL_TEST(dp_de, dp_de2, 1e-5);
  }

  // test derivatives for T_from_v_e
  {
    Real v = ideal->v_from_p_T(p, T);
    Real e = ideal->e_from_p_T(p, T);
    Real TT = 0, dT_dv = 0, dT_de = 0;
    ideal->T_from_v_e(v, e, TT, dT_dv, dT_de);
    Real TT2 = 0, dT_dv2 = 0, dT_de2 = 0;
    compare_with_ideal->T_from_v_e(v, e, TT2, dT_dv2, dT_de2);
    REL_TEST(dT_dv, dT_dv2, 1e-5);
    REL_TEST(dT_de, dT_de2, 1e-5);
  }

  // test derivatives for e_from_v_h
  {
    Real v = ideal->v_from_p_T(p, T);
    Real h = ideal->h_from_p_T(p, T);
    Real ee = 0, de_dv = 0, de_dh = 0;
    ideal->e_from_v_h(v, h, ee, de_dv, de_dh);
    Real ee2 = 0, de_dv2 = 0, de_dh2 = 0;
    compare_with_ideal->e_from_v_h(v, h, ee2, de_dv2, de_dh2);
    REL_TEST(de_dv, de_dv2, 1e-5);
    REL_TEST(de_dh, de_dh2, 1e-5);
  }
}

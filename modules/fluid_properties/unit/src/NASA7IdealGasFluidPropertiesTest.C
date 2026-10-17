//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NASA7IdealGasFluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"
#include "SolutionInvalidity.h"

namespace
{
/// Mass-specific properties at a state, computed with Cantera
struct CanteraState
{
  Real p;
  Real T;
  Real rho;
  Real e;
  Real h;
  Real s;
  Real cp;
  Real cv;
  Real c;
};

/**
 * Number of solution warnings (warning = true) or invalid-solution flags (warning = false) raised
 * by NASA7IdealGasFluidProperties objects in the current iteration
 */
unsigned int
flagCount(MooseApp & app, const bool warning)
{
  const auto & registry = moose::internal::getSolutionInvalidityRegistry();
  const auto & counts = app.solutionInvalidity().counts();
  unsigned int count = 0;
  for (const auto id : index_range(counts))
  {
    const auto & info = registry.item(id);
    if (info.object_type == "NASA7IdealGasFluidProperties" && info.warning == warning)
      count += counts[id].current_counts;
  }
  return count;
}
}

TEST_F(NASA7IdealGasFluidPropertiesTest, cantera)
{
  // The reference values were computed with Cantera 3.2.0 from gri30.yaml. Cantera uses the gas
  // constant 8.31446261815324 J/(mol K), whereas FluidProperties::_R is 8.3144598 J/(mol K).
  // Properties proportional to the gas constant are rescaled by the ratio of the two, density by
  // its inverse, and sound speed by its square root, so that only roundoff separates the results.
  const Real R_ratio = FluidProperties::_R / 8.31446261815324;
  const Real tol = 1e-12;

  const auto check =
      [R_ratio, tol](const NASA7IdealGasFluidProperties & fp, const CanteraState & ref)
  {
    REL_TEST(fp.rho_from_p_T(ref.p, ref.T), ref.rho / R_ratio, tol);
    REL_TEST(fp.e_from_p_T(ref.p, ref.T), ref.e * R_ratio, tol);
    REL_TEST(fp.h_from_p_T(ref.p, ref.T), ref.h * R_ratio, tol);
    REL_TEST(fp.s_from_p_T(ref.p, ref.T), ref.s * R_ratio, tol);
    REL_TEST(fp.cp_from_p_T(ref.p, ref.T), ref.cp * R_ratio, tol);
    REL_TEST(fp.cv_from_p_T(ref.p, ref.T), ref.cv * R_ratio, tol);
    REL_TEST(fp.c_from_p_T(ref.p, ref.T), ref.c * std::sqrt(R_ratio), tol);
  };

  // H2O below the midpoint temperature, at 923.15 K (650 C), a typical operating temperature of
  // solid oxide cells, and at a pressure other than the reference pressure
  check(*_fp_h2o,
        {2e5,
         923.15,
         0.46941590772782527,
         -12580176.509683069,
         -12154115.107188027,
         12424.123173583768,
         2236.055923467166,
         1774.5259418877467,
         732.7169706483752});

  // N2 above the midpoint temperature
  check(*_fp_n2,
        {1e5,
         1500.0,
         0.22462065027779515,
         925748.8663660923,
         1370943.9091029332,
         8635.100537093402,
         1242.4266981940143,
         945.6300030361202,
         764.8036043196926});
}

TEST_F(NASA7IdealGasFluidPropertiesTest, consistency)
{
  // A low-pressure state below the midpoint temperature and a high-pressure state above it
  for (const auto * fp : {_fp_h2o, _fp_n2})
    for (const auto & [p, T] :
         {std::pair<Real, Real>(1e5, 600.0), std::pair<Real, Real>(3e6, 1500.0)})
    {
      const Real rho = fp->rho_from_p_T(p, T);
      const Real v = 1 / rho;
      const Real e = fp->e_from_p_T(p, T);
      const Real h = fp->h_from_p_T(p, T);
      const Real s = fp->s_from_p_T(p, T);
      const Real cp = fp->cp_from_p_T(p, T);
      const Real cv = fp->cv_from_p_T(p, T);
      const Real gamma = fp->gamma_from_p_T(p, T);
      const Real c = fp->c_from_p_T(p, T);

      // The other variable sets describe the same state
      REL_TEST(fp->v_from_p_T(p, T), v, REL_TOL_CONSISTENCY);
      REL_TEST(fp->p_from_T_v(T, v), p, REL_TOL_CONSISTENCY);
      REL_TEST(fp->e_from_T_v(T, v), e, REL_TOL_CONSISTENCY);
      REL_TEST(fp->h_from_T_v(T, v), h, REL_TOL_CONSISTENCY);
      REL_TEST(fp->s_from_T_v(T, v), s, REL_TOL_CONSISTENCY);
      REL_TEST(fp->cv_from_T_v(T, v), cv, REL_TOL_CONSISTENCY);
      REL_TEST(fp->e_from_p_rho(p, rho), e, REL_TOL_CONSISTENCY);

      // Ideal-gas relations: definition of enthalpy, Mayer's relation, and sound speed
      REL_TEST(h, e + p * v, REL_TOL_CONSISTENCY);
      REL_TEST(cp - cv, FluidProperties::_R / fp->molarMass(), REL_TOL_CONSISTENCY);
      REL_TEST(gamma, cp / cv, REL_TOL_CONSISTENCY);
      REL_TEST(c * c, gamma * p * v, REL_TOL_CONSISTENCY);

      // (dh/dT)_p = cp, (de/dT)_v = cv, (ds/dT)_p = cp/T, and the Maxwell relation
      // (ds/dp)_T = -(dv/dT)_p = -v/T
      Real value, d_dp, d_dT, d_dv;
      fp->h_from_p_T(p, T, value, d_dp, d_dT);
      REL_TEST(d_dT, cp, REL_TOL_CONSISTENCY);
      fp->e_from_T_v(T, v, value, d_dT, d_dv);
      REL_TEST(d_dT, cv, REL_TOL_CONSISTENCY);
      fp->s_from_p_T(p, T, value, d_dp, d_dT);
      REL_TEST(d_dT, cp / T, REL_TOL_CONSISTENCY);
      REL_TEST(d_dp, -v / T, REL_TOL_CONSISTENCY);
    }
}

TEST_F(NASA7IdealGasFluidPropertiesTest, derivatives)
{
  // One state on each side of the midpoint temperature
  for (const auto & [fp, T] : {std::pair<const NASA7IdealGasFluidProperties *, Real>(_fp_h2o, 700),
                               std::pair<const NASA7IdealGasFluidProperties *, Real>(_fp_n2, 1500)})
  {
    const Real p = 2e5;
    const Real rho = fp->rho_from_p_T(p, T);
    const Real v = 1 / rho;

    DERIV_TEST(fp->rho_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->v_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->e_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->h_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->s_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->cp_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->cv_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->c_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->gamma_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->p_from_T_v, T, v, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->e_from_T_v, T, v, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->h_from_T_v, T, v, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->s_from_T_v, T, v, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->cv_from_T_v, T, v, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->e_from_p_rho, p, rho, REL_TOL_DERIVATIVE);

    const ADReal p_ad = p;
    const ADReal T_ad = T;
    const ADReal v_ad = v;
    const ADReal rho_ad = rho;

    AD_DERIV_TEST(fp->rho_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->v_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->e_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->h_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->s_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->cp_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->cv_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->c_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->gamma_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->p_from_T_v, T_ad, v_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->e_from_T_v, T_ad, v_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->h_from_T_v, T_ad, v_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->s_from_T_v, T_ad, v_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->cv_from_T_v, T_ad, v_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->e_from_p_rho, p_ad, rho_ad, REL_TOL_DERIVATIVE);

    // The AD overloads carry the derivatives of their arguments: (de/dT)_p = cv
    DNDerivativeType dT_dT;
    Moose::derivInsert(dT_dT, 0, 1);
    const ADReal T_seeded(T, dT_dT);
    REL_TEST(fp->e_from_p_T(p_ad, T_seeded).derivatives()[0],
             fp->cv_from_p_T(p, T),
             REL_TOL_CONSISTENCY);
  }
}

TEST_F(NASA7IdealGasFluidPropertiesTest, outOfRange)
{
  const Real p = 1e5;
  const auto warnings = flagCount(*_app, true);

  // 250 K lies inside the range of the H2O data, 200 K to 3500 K, but below that of the N2 data,
  // 300 K to 5000 K
  _fp_h2o->cp_from_p_T(p, 250);
  EXPECT_EQ(flagCount(*_app, true), warnings);

  // Outside the range, the polynomials are extrapolated and each evaluation is counted as a
  // solution warning
  const Real cp = _fp_n2->cp_from_p_T(p, 250);
  EXPECT_EQ(flagCount(*_app, true), warnings + 1);
  REL_TEST(cp,
           FluidProperties::_R / _fp_n2->molarMass() *
               NASA7::cpOverR(NASA7::speciesData("N2"), 250.0),
           REL_TOL_CONSISTENCY);
  _fp_h2o->cp_from_p_T(p, 3600);
  EXPECT_EQ(flagCount(*_app, true), warnings + 2);

  // A non-positive temperature is an invalid solution
  const auto errors = flagCount(*_app, false);
  _fp_n2->e_from_p_T(p, -10);
  EXPECT_EQ(flagCount(*_app, false), errors + 1);
}

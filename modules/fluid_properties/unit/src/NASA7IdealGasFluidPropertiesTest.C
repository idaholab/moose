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

#include <limits>

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
    DERIV_TEST(fp->mu_from_p_T, p, T, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->k_from_p_T, p, T, REL_TOL_DERIVATIVE);
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
    AD_DERIV_TEST(fp->mu_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->k_from_p_T, p_ad, T_ad, REL_TOL_DERIVATIVE);
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

  // A temperature found by inverting the internal energy is checked once
  const Real e = _fp_n2->e_from_p_T(p, 250);
  const auto warnings_before_inversion = flagCount(*_app, true);
  REL_TEST(_fp_n2->T_from_v_e(1, e), 250, REL_TOL_CONSISTENCY);
  EXPECT_EQ(flagCount(*_app, true), warnings_before_inversion + 1);

  // A non-positive or not-a-number temperature is an invalid solution
  const auto errors = flagCount(*_app, false);
  _fp_n2->e_from_p_T(p, -10);
  EXPECT_EQ(flagCount(*_app, false), errors + 1);
  _fp_n2->e_from_p_T(p, std::numeric_limits<Real>::quiet_NaN());
  EXPECT_EQ(flagCount(*_app, false), errors + 2);
}

TEST_F(NASA7IdealGasFluidPropertiesTest, inverses)
{
  for (const auto * fp : {_fp_h2o, _fp_n2})
  {
    // States on each side of the midpoint temperature
    for (const auto & [p, T] :
         {std::pair<Real, Real>(1e5, 600.0), std::pair<Real, Real>(3e6, 1500.0)})
    {
      const Real v = fp->v_from_p_T(p, T);
      const Real e = fp->e_from_p_T(p, T);
      const Real h = fp->h_from_p_T(p, T);
      const Real s = fp->s_from_p_T(p, T);

      REL_TEST(fp->T_from_v_e(v, e), T, REL_TOL_CONSISTENCY);
      REL_TEST(fp->p_from_v_e(v, e), p, REL_TOL_CONSISTENCY);
      REL_TEST(fp->c_from_v_e(v, e), fp->c_from_p_T(p, T), REL_TOL_CONSISTENCY);
      REL_TEST(fp->cp_from_v_e(v, e), fp->cp_from_p_T(p, T), REL_TOL_CONSISTENCY);
      REL_TEST(fp->cv_from_v_e(v, e), fp->cv_from_p_T(p, T), REL_TOL_CONSISTENCY);
      REL_TEST(fp->gamma_from_v_e(v, e), fp->gamma_from_p_T(p, T), REL_TOL_CONSISTENCY);
      REL_TEST(fp->s_from_v_e(v, e), s, REL_TOL_CONSISTENCY);
      REL_TEST(fp->g_from_v_e(v, e), h - T * s, REL_TOL_CONSISTENCY);
      REL_TEST(fp->T_from_p_h(p, h), T, REL_TOL_CONSISTENCY);
      REL_TEST(fp->s_from_h_p(h, p), s, REL_TOL_CONSISTENCY);
      REL_TEST(fp->p_from_h_s(h, s), p, REL_TOL_CONSISTENCY);
      REL_TEST(fp->e_from_v_h(v, h), e, REL_TOL_CONSISTENCY);

      // (ds/dv)_e = p/T, (ds/de)_v = 1/T, (dp/dh)_s = rho, and (dp/ds)_h = -rho T
      Real value, d_d1, d_d2;
      fp->s_from_v_e(v, e, value, d_d1, d_d2);
      REL_TEST(d_d1, p / T, REL_TOL_CONSISTENCY);
      REL_TEST(d_d2, 1 / T, REL_TOL_CONSISTENCY);
      fp->p_from_h_s(h, s, value, d_d1, d_d2);
      REL_TEST(d_d1, 1 / v, REL_TOL_CONSISTENCY);
      REL_TEST(d_d2, -T / v, REL_TOL_CONSISTENCY);
    }

    // At the midpoint temperature the two polynomials do not match exactly, and the internal
    // energy and enthalpy of the low-temperature polynomial are also reached by the
    // high-temperature polynomial slightly above it, less than 3e-4 K for the built-in species. The
    // inversion may return either temperature.
    const Real p = 2e5;
    const Real T_mid = 1000;
    const Real v = fp->v_from_p_T(p, T_mid);
    ABS_TEST(fp->T_from_v_e(v, fp->e_from_p_T(p, T_mid)), T_mid, 1e-3);
    ABS_TEST(fp->T_from_p_h(p, fp->h_from_p_T(p, T_mid)), T_mid, 1e-3);
  }
}

TEST_F(NASA7IdealGasFluidPropertiesTest, energyNearZero)
{
  // The internal energy of N2 includes its enthalpy of formation, zero, and it passes through zero
  // near 417 K. A convergence test relative to the target energy would demand accuracy below
  // roundoff there.
  const Real v = 1;
  for (const Real e : {-1e-3, 0.0, 1e-3})
  {
    const Real T = _fp_n2->T_from_v_e(v, e);
    ABS_TEST(_fp_n2->e_from_T_v(T, v), e, REL_TOL_CONSISTENCY * _fp_n2->cv_from_T_v(T, v) * T);
  }
}

TEST_F(NASA7IdealGasFluidPropertiesTest, inverseDerivatives)
{
  // One state on each side of the midpoint temperature
  for (const auto & [fp, T] : {std::pair<const NASA7IdealGasFluidProperties *, Real>(_fp_h2o, 700),
                               std::pair<const NASA7IdealGasFluidProperties *, Real>(_fp_n2, 1500)})
  {
    const Real p = 2e5;
    const Real v = fp->v_from_p_T(p, T);
    const Real e = fp->e_from_p_T(p, T);
    const Real h = fp->h_from_p_T(p, T);
    const Real s = fp->s_from_p_T(p, T);

    DERIV_TEST(fp->T_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->p_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->c_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->cp_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->cv_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->gamma_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->s_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->g_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->mu_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->k_from_v_e, v, e, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->T_from_p_h, p, h, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->s_from_h_p, h, p, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->p_from_h_s, h, s, REL_TOL_DERIVATIVE);
    DERIV_TEST(fp->e_from_v_h, v, h, REL_TOL_DERIVATIVE);

    const ADReal p_ad = p;
    const ADReal v_ad = v;
    const ADReal e_ad = e;
    const ADReal h_ad = h;
    const ADReal s_ad = s;

    AD_DERIV_TEST(fp->T_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->p_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->c_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->cp_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->cv_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->gamma_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->s_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->g_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->mu_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->k_from_v_e, v_ad, e_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->T_from_p_h, p_ad, h_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->s_from_h_p, h_ad, p_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->p_from_h_s, h_ad, s_ad, REL_TOL_DERIVATIVE);
    AD_DERIV_TEST(fp->e_from_v_h, v_ad, h_ad, REL_TOL_DERIVATIVE);

    // The AD overloads carry the derivatives of their arguments: (dT/de)_v = 1/cv
    DNDerivativeType de_de;
    Moose::derivInsert(de_de, 0, 1);
    const ADReal e_seeded(e, de_de);
    REL_TEST(fp->T_from_v_e(v_ad, e_seeded).derivatives()[0],
             1 / fp->cv_from_p_T(p, T),
             REL_TOL_CONSISTENCY);
  }
}

TEST_F(NASA7IdealGasFluidPropertiesTest, inverseRange)
{
  // Invert the internal energy and enthalpy of every built-in species at temperatures spread
  // geometrically from half the lower end of the range of its data to 1.5 times the upper end,
  // so that the inversion is exercised far from any initial guess and in extrapolation
  // The internal energy and enthalpy of an ideal gas do not depend on pressure or specific volume
  const Real p = 1e5;
  const Real v = 1;
  // 60 geometric intervals place consecutive temperatures about 7% apart
  const unsigned int n = 60;
  for (const std::string species : {"H2", "O2", "H2O", "CH4", "CO", "CO2", "N2"})
  {
    const std::string name = "sweep_" + species;
    const auto & fp = buildFluidProperties(species, name);
    const auto & data = NASA7::speciesData(species);
    const Real T_low = data.T_min / 2;
    const Real T_high = 1.5 * data.T_max;
    for (const auto i : make_range(n + 1))
    {
      const Real T = T_low * std::pow(T_high / T_low, Real(i) / n);
      REL_TEST(fp.T_from_v_e(v, fp.e_from_T_v(T, v)), T, REL_TOL_CONSISTENCY);
      REL_TEST(fp.T_from_p_h(p, fp.h_from_p_T(p, T)), T, REL_TOL_CONSISTENCY);
    }
  }
}

TEST_F(NASA7IdealGasFluidPropertiesTest, transport)
{
  // Viscosity and thermal conductivity at 1e5 Pa compared with the fluid properties of the real
  // fluids, which include the small effect of density at this pressure. The tolerances bound the
  // deviations at these states. The Chapman-Enskog viscosity is within 2.5% for N2, CH4, and CO2,
  // but the Lennard-Jones parameters of H2 give a viscosity up to 6% low at 900 K. The modified
  // Eucken conductivity is within 9%. For steam, both objects evaluate the IAPWS low-density
  // viscosity, whereas Water97FluidProperties evaluates the IAPS 1985 conductivity.
  const Real p = 1e5;
  struct Reference
  {
    std::string species;
    std::string type;
    std::vector<Real> temperatures;
    Real mu_tol;
    Real k_tol;
  };
  const std::vector<Reference> references = {
      {"N2", "NitrogenFluidProperties", {300, 600, 900}, 0.015, 0.06},
      {"H2", "HydrogenFluidProperties", {300, 600, 900}, 0.06, 0.07},
      {"CH4", "MethaneFluidProperties", {300, 600, 900}, 0.03, 0.08},
      {"CO2", "CO2FluidProperties", {300, 600, 900}, 0.01, 0.09},
      {"H2O", "Water97FluidProperties", {400, 700, 1100}, 0.01, 0.02}};

  for (const auto & ref : references)
  {
    const auto & fp = buildFluidProperties(ref.species, "transport_" + ref.species);
    InputParameters params = _factory.getValidParams(ref.type);
    _fe_problem->addUserObject(ref.type, "reference_" + ref.species, params);
    const auto & reference =
        _fe_problem->getUserObject<SinglePhaseFluidProperties>("reference_" + ref.species);
    for (const auto T : ref.temperatures)
    {
      const Real mu = fp.mu_from_p_T(p, T);
      const Real k = fp.k_from_p_T(p, T);
      REL_TEST(mu, reference.mu_from_p_T(p, T), ref.mu_tol);
      REL_TEST(k, reference.k_from_p_T(p, T), ref.k_tol);

      // The (specific volume, internal energy) variable set gives the same values
      const Real v = fp.v_from_p_T(p, T);
      const Real e = fp.e_from_p_T(p, T);
      REL_TEST(fp.mu_from_v_e(v, e), mu, REL_TOL_CONSISTENCY);
      REL_TEST(fp.k_from_v_e(v, e), k, REL_TOL_CONSISTENCY);
    }
  }
}

TEST_F(NASA7IdealGasFluidPropertiesTest, steamTransportRange)
{
  // 1500 K lies inside the range of the H2O data, 200 K to 3500 K, but above that of the IAPWS
  // transport formulations, 273.16 K to 1173.15 K. The viscosity and thermal conductivity of other
  // species are not limited to that range.
  const Real p = 1e5;
  const auto warnings = flagCount(*_app, true);
  _fp_h2o->mu_from_p_T(p, 1000);
  _fp_h2o->k_from_p_T(p, 1000);
  _fp_n2->mu_from_p_T(p, 1500);
  _fp_n2->k_from_p_T(p, 1500);
  EXPECT_EQ(flagCount(*_app, true), warnings);
  _fp_h2o->mu_from_p_T(p, 1500);
  EXPECT_EQ(flagCount(*_app, true), warnings + 1);
  _fp_h2o->k_from_p_T(p, 1500);
  EXPECT_EQ(flagCount(*_app, true), warnings + 2);
}

TEST_F(NASA7IdealGasFluidPropertiesTest, densityFromEntropy)
{
  // Invert the entropy of every built-in species at two pressures and at temperatures spread
  // geometrically from half the lower end of the range of its data to 1.5 times the upper end
  // 60 geometric intervals place consecutive temperatures about 7% apart
  const unsigned int n = 60;
  for (const std::string species : {"H2", "O2", "H2O", "CH4", "CO", "CO2", "N2"})
  {
    const auto & fp = buildFluidProperties(species, "entropy_" + species);
    const auto & data = NASA7::speciesData(species);
    const Real T_low = data.T_min / 2;
    const Real T_high = 1.5 * data.T_max;
    for (const Real p : {1e3, 1e7})
      for (const auto i : make_range(n + 1))
      {
        const Real T = T_low * std::pow(T_high / T_low, Real(i) / n);
        REL_TEST(
            fp.rho_from_p_s(p, fp.s_from_p_T(p, T)), fp.rho_from_p_T(p, T), REL_TOL_CONSISTENCY);
      }
  }

  // Derivatives on each side of the midpoint temperature
  for (const auto & [fp, T] : {std::pair<const NASA7IdealGasFluidProperties *, Real>(_fp_h2o, 700),
                               std::pair<const NASA7IdealGasFluidProperties *, Real>(_fp_n2, 1500)})
  {
    const Real p = 2e5;
    const Real s = fp->s_from_p_T(p, T);
    DERIV_TEST(fp->rho_from_p_s, p, s, REL_TOL_DERIVATIVE);
    const ADReal p_ad = p;
    const ADReal s_ad = s;
    AD_DERIV_TEST(fp->rho_from_p_s, p_ad, s_ad, REL_TOL_DERIVATIVE);

    // At constant pressure, (dT/ds)_p = T / cp and, for an ideal gas, (drho/dT)_p = -rho / T, so
    // (drho/ds)_p = -rho / cp
    Real rho, drho_dp, drho_ds;
    fp->rho_from_p_s(p, s, rho, drho_dp, drho_ds);
    REL_TEST(drho_ds, -rho / fp->cp_from_p_T(p, T), REL_TOL_CONSISTENCY);
  }
}

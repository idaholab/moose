//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasMixtureFluidPropertiesTest.h"
#include "VaporMixtureFluidPropertiesUtils.h"
#include "FluidProperties.h"

TEST_F(IdealGasMixtureFluidPropertiesTest, test)
{
  Real T = 400;
  Real p = 100000;
  std::vector<Real> x = {0.7};

  const Real v = 1.38490747936373;
  const Real rho = 0.7220698962933115;
  const Real e = 1.995566878921797e+06;
  const Real s = 3362.64537354375;
  const Real c = 442.127817187648;
  const Real cp = 1187.64406714542;
  const Real cv = 841.417197304492;
  const Real mu = 1.863687329029527e-05;
  const Real k = 0.0304142253602899;

  REL_TEST(_fp_mix->p_from_v_e(v, e, x), p, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->T_from_v_e(v, e, x), T, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->v_from_p_T(p, T, x), v, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->rho_from_p_T(p, T, x), rho, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->e_from_p_T(p, T, x), e, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->e_from_p_rho(p, rho, x), e, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->s_from_p_T(p, T, x), s, REL_TOL_SAVED_VALUE);

  // The mixture entropy exceeds the mass-weighted component entropies at the mixture pressure
  // by the ideal entropy of mixing, -R / M sum_i psi_i ln(psi_i), with psi_i the mole fractions
  {
    const Real M_steam = 0.01801488;
    const Real M_nitrogen = 0.028012734746133888;
    const Real M_mix = 1.0 / ((1.0 - x[0]) / M_steam + x[0] / M_nitrogen);
    const Real psi_steam = (1.0 - x[0]) * M_mix / M_steam;
    const Real psi_nitrogen = x[0] * M_mix / M_nitrogen;
    const Real s_unmixed =
        (1.0 - x[0]) * _fp_steam->s_from_p_T(p, T) + x[0] * _fp_nitrogen->s_from_p_T(p, T);
    const Real s_mixing = -FluidProperties::_R / M_mix *
                          (psi_steam * std::log(psi_steam) + psi_nitrogen * std::log(psi_nitrogen));
    REL_TEST(_fp_mix->s_from_p_T(p, T, x), s_unmixed + s_mixing, REL_TOL_CONSISTENCY);
  }
  REL_TEST(_fp_mix->c_from_p_T(p, T, x), c, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->cp_from_p_T(p, T, x), cp, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->cv_from_p_T(p, T, x), cv, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->mu_from_p_T(p, T, x), mu, REL_TOL_SAVED_VALUE);
  REL_TEST(_fp_mix->k_from_p_T(p, T, x), k, REL_TOL_SAVED_VALUE);

  // Test sound speed against thermodynamics definition:
  //   c = sqrt((dp/drho)_s) = v sqrt(-(dp/dv)_s)
  // To compute (dp/dv)_s, we will use the following identity:
  //   (dp/dv)_s = 1 / [(dv/dp)_T - (dv/dT)_p (ds/dp)_T / (ds/dT)_p]
  const Real rel_pert = REL_PERTURBATION;
  const Real fpert1 = 1 + rel_pert;
  const Real fpert2 = 1 - rel_pert;
  const Real dvdp_T =
      (_fp_mix->v_from_p_T(p * fpert1, T, x) - _fp_mix->v_from_p_T(p * fpert2, T, x)) /
      (2 * rel_pert * p);
  const Real dvdT_p =
      (_fp_mix->v_from_p_T(p, T * fpert1, x) - _fp_mix->v_from_p_T(p, T * fpert2, x)) /
      (2 * rel_pert * T);
  const Real dsdp_T =
      (_fp_mix->s_from_p_T(p * fpert1, T, x) - _fp_mix->s_from_p_T(p * fpert2, T, x)) /
      (2 * rel_pert * p);
  const Real dsdT_p =
      (_fp_mix->s_from_p_T(p, T * fpert1, x) - _fp_mix->s_from_p_T(p, T * fpert2, x)) /
      (2 * rel_pert * T);
  const Real dpdv_s = 1.0 / (dvdp_T - dvdT_p * dsdp_T / dsdT_p);
  const Real c_gold = v * std::sqrt(-dpdv_s);
  REL_TEST(_fp_mix->c_from_p_T(p, T, x), c_gold, REL_TOL_DERIVATIVE);

  const Real x_steam = 0.3;
  const Real x_nitrogen = 0.7;

  const Real M_steam = 0.01801488;
  const Real M_nitrogen = 0.028012734746133888;
  const Real M = 1.0 / (x_steam / M_steam + x_nitrogen / M_nitrogen);

  const Real v_gold = FluidProperties::_R * T / (M * p);
  REL_TEST(v, v_gold, REL_TOL_SAVED_VALUE);
}

TEST_F(IdealGasMixtureFluidPropertiesTest, adDerivatives)
{
  // Compares the derivatives of the automatic differentiation version of a property f(a, b, x)
  // with respect to a, b, and the secondary mass fraction x with central differences of its Real
  // version
  const auto check = [](const auto & f, const Real a, const Real b, const Real x)
  {
    DNDerivativeType da, db, dx;
    Moose::derivInsert(da, 0, 1);
    Moose::derivInsert(db, 1, 1);
    Moose::derivInsert(dx, 2, 1);
    const ADReal f_ad = f(ADReal(a, da), ADReal(b, db), std::vector<ADReal>{ADReal(x, dx)});
    REL_TEST(f_ad, f(a, b, std::vector<Real>{x}), REL_TOL_CONSISTENCY);

    const Real ha = REL_PERTURBATION * a;
    const Real hb = REL_PERTURBATION * b;
    const Real hx = REL_PERTURBATION * x;
    REL_TEST(f_ad.derivatives()[0],
             (f(a + ha, b, std::vector<Real>{x}) - f(a - ha, b, std::vector<Real>{x})) / (2 * ha),
             REL_TOL_DERIVATIVE);
    REL_TEST(f_ad.derivatives()[1],
             (f(a, b + hb, std::vector<Real>{x}) - f(a, b - hb, std::vector<Real>{x})) / (2 * hb),
             REL_TOL_DERIVATIVE);
    REL_TEST(f_ad.derivatives()[2],
             (f(a, b, std::vector<Real>{x + hx}) - f(a, b, std::vector<Real>{x - hx})) / (2 * hx),
             REL_TOL_DERIVATIVE);
  };

  const Real p = 100000;
  const Real T = 400;
  const Real x = 0.7;
  const Real v = _fp_mix->v_from_p_T(p, T, {x});
  const Real e = _fp_mix->e_from_p_T(p, T, {x});

  check([this](const auto & v, const auto & e, const auto & x)
        { return _fp_mix->T_from_v_e(v, e, x); },
        v,
        e,
        x);
  check([this](const auto & v, const auto & e, const auto & x)
        { return _fp_mix->p_from_v_e(v, e, x); },
        v,
        e,
        x);
  check([this](const auto & p, const auto & rho, const auto & x)
        { return _fp_mix->e_from_p_rho(p, rho, x); },
        p,
        1 / v,
        x);
  check([this](const auto & p, const auto & T, const auto & x)
        { return _fp_mix->c_from_p_T(p, T, x); },
        p,
        T,
        x);
}

TEST_F(IdealGasMixtureFluidPropertiesTest, nasa7Cantera)
{
  // Reformed fuel of a solid oxide cell, with mole fractions H2O 0.35, H2 0.30, CO 0.10, CO2 0.15,
  // CH4 0.05, and N2 0.05, at 1073.15 K (800 C). The secondary mass fractions and the reference
  // values were computed with Cantera 3.2.0 from gri30.yaml. Cantera uses the gas constant
  // 8.31446261815324 J/(mol K), whereas FluidProperties::_R is 8.3144598 J/(mol K). Properties
  // proportional to the gas constant are rescaled by the ratio of the two, density by its inverse,
  // and sound speed by its square root, so that only roundoff separates the results.
  const std::vector<std::string> species = {"H2O", "H2", "CO", "CO2", "CH4", "N2"};
  const std::string name = "reformate";
  const auto & fp = buildNASA7Mixture(species, name);
  const std::vector<Real> x = {0.03266496536638716,
                               0.1512807010437342,
                               0.3565358285737432,
                               0.04332374664128218,
                               0.0756511524284036};
  const Real p = 1.5e5;
  const Real T = 1073.15;
  const Real R_ratio = FluidProperties::_R / 8.31446261815324;
  const Real tol = 1e-12;

  const Real rho = 0.311262086619508 / R_ratio;
  const Real e = -7502142.694955811 * R_ratio;
  REL_TEST(fp.rho_from_p_T(p, T, x), rho, tol);
  REL_TEST(fp.e_from_p_T(p, T, x), e, tol);
  REL_TEST(fp.cp_from_p_T(p, T, x), 2216.7883592581443 * R_ratio, tol);
  REL_TEST(fp.cv_from_p_T(p, T, x), 1767.7281187454187 * R_ratio, tol);
  REL_TEST(fp.c_from_p_T(p, T, x), 777.386327867082 * std::sqrt(R_ratio), tol);

  // The (v, e) and (p, rho) variable sets describe the same state
  REL_TEST(fp.T_from_v_e(1 / rho, e, x), T, REL_TOL_CONSISTENCY);
  REL_TEST(fp.p_from_v_e(1 / rho, e, x), p, REL_TOL_CONSISTENCY);
  REL_TEST(fp.e_from_p_rho(p, rho, x), e, REL_TOL_CONSISTENCY);
}

TEST_F(IdealGasMixtureFluidPropertiesTest, nasa7Inversion)
{
  // The internal energy and enthalpy of ideal gases do not depend on pressure or specific volume
  const Real p = 1e5;
  const Real v = 1;

  // Computes the temperature from the internal energy at temperatures spread geometrically between
  // T_low and T_high, 60 intervals placing consecutive temperatures about 7% apart
  const auto sweep = [p, v](const IdealGasMixtureFluidProperties & fp,
                            const std::vector<Real> & x,
                            const Real T_low,
                            const Real T_high)
  {
    const unsigned int n = 60;
    for (const auto i : make_range(n + 1))
    {
      const Real T = T_low * std::pow(T_high / T_low, Real(i) / n);
      REL_TEST(fp.T_from_v_e(v, fp.e_from_p_T(p, T, x), x), T, REL_TOL_CONSISTENCY);
    }
  };

  // From half the highest lower end to 1.5 times the lowest upper end of the temperature ranges of
  // the data of the components: 300 K for N2 and 3500 K for the others
  const std::vector<std::string> reformate_species = {"H2O", "H2", "CO", "CO2", "CH4", "N2"};
  const std::string reformate_name = "reformate";
  const auto & reformate = buildNASA7Mixture(reformate_species, reformate_name);
  sweep(reformate, {0.033, 0.151, 0.357, 0.043, 0.076}, 150, 5250);

  // Air, with the O2 mass fraction 0.233
  const std::vector<std::string> air_species = {"N2", "O2"};
  const std::string air_name = "air";
  const auto & air = buildNASA7Mixture(air_species, air_name);
  const std::vector<Real> x_air = {0.233};
  sweep(air, x_air, 150, 5250);

  // CH4 and N2 in equal mass fractions. Started below the solution, Newton's method can overshoot
  // to a spurious solution of the extrapolated CH4 polynomials.
  const std::vector<std::string> methane_nitrogen_species = {"N2", "CH4"};
  const std::string methane_nitrogen_name = "methane_nitrogen";
  const auto & methane_nitrogen =
      buildNASA7Mixture(methane_nitrogen_species, methane_nitrogen_name);
  sweep(methane_nitrogen, {0.5}, 150, 5250);

  // The internal energy of air, which includes the zero enthalpies of formation of N2 and O2,
  // passes through zero near 417 K, where a convergence test relative to the target energy would
  // demand accuracy below roundoff
  for (const Real e : {-1e-3, -1e-6, 0.0, 1e-6, 1e-3})
  {
    const Real T = air.T_from_v_e(v, e, x_air);
    ABS_TEST(
        air.e_from_p_T(p, T, x_air), e, REL_TOL_CONSISTENCY * air.cv_from_p_T(p, T, x_air) * T);
  }

  // A NASA 7-coefficient gas, CO2, as a secondary component of nitrogen with constant specific
  // heats; the latter has no temperature range
  {
    InputParameters params = _factory.getValidParams("NASA7IdealGasFluidProperties");
    params.set<MooseEnum>("species") = "CO2";
    _fe_problem->addUserObject("NASA7IdealGasFluidProperties", "co2", params);
  }
  {
    InputParameters params = _factory.getValidParams("IdealGasMixtureFluidProperties");
    params.set<std::vector<UserObjectName>>("component_fluid_properties") = {"fp_nitrogen", "co2"};
    _fe_problem->addUserObject("IdealGasMixtureFluidProperties", "nitrogen_co2", params);
  }
  sweep(
      _fe_problem->getUserObject<IdealGasMixtureFluidProperties>("nitrogen_co2"), {0.3}, 100, 5250);
}

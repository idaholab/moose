//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralVaporMixtureFluidProperties.h"
#include "SinglePhaseFluidProperties.h"
#include <numeric>

registerMooseObject("FluidPropertiesApp", GeneralVaporMixtureFluidProperties);

template <>
InputParameters
validParams<GeneralVaporMixtureFluidProperties>()
{
  InputParameters params = validParams<VaporMixtureFluidProperties>();

  params.addClassDescription("Class for fluid properties of an arbitrary vapor mixture");

  params.addRequiredParam<UserObjectName>(
      "fp_primary", "Name of fluid properties user object for primary vapor component");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "fp_secondary", "Name of fluid properties user object(s) for secondary vapor component(s)");

  params.addParam<Real>("p_initial_guess", 1e5, "Initial guess for mixture pressure");
  params.addParam<Real>("T_initial_guess", 400, "Initial guess for mixture temperature");
  params.addParam<Real>("newton_damping", 1.0, "Damping factor for Newton iteration");
  params.addParam<Real>("newton_rel_tol", 1e-6, "Relative tolerance for Newton iteration");
  params.addParam<unsigned int>("newton_max_its", 25, "Maximum iterations for Newton iteration");
  params.addParam<bool>("update_guesses",
                        true,
                        "Option to update guesses for pressure and temperature based on the "
                        "converged values for the previous solve");

  // This is necessary because initialize() must be called before any interface
  // can be used (which can occur as early as initialization of variables).
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

  return params;
}

GeneralVaporMixtureFluidProperties::GeneralVaporMixtureFluidProperties(
    const InputParameters & parameters)
  : VaporMixtureFluidProperties(parameters),
    _fp_secondary_names(getParam<std::vector<UserObjectName>>("fp_secondary")),
    _n_secondary_vapors(_fp_secondary_names.size()),
    _p_initial_guess(getParam<Real>("p_initial_guess")),
    _T_initial_guess(getParam<Real>("T_initial_guess")),
    _newton_damping(getParam<Real>("newton_damping")),
    _newton_rel_tol(getParam<Real>("newton_rel_tol")),
    _newton_max_its(getParam<unsigned int>("newton_max_its")),
    _update_guesses(getParam<bool>("update_guesses"))
{
  _fp_primary = &getUserObject<SinglePhaseFluidProperties>("fp_primary");

  _fp_secondary.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
    _fp_secondary[i] = &getUserObjectByName<SinglePhaseFluidProperties>(_fp_secondary_names[i]);

  _p_guess = _p_initial_guess;
  _T_guess = _T_initial_guess;
}

GeneralVaporMixtureFluidProperties::~GeneralVaporMixtureFluidProperties() {}

Real
GeneralVaporMixtureFluidProperties::p_from_v_e(Real v, Real e, const std::vector<Real> & x) const
{
  Real p, dp_dv, dp_de, T, dT_dv, dT_de;
  std::vector<Real> dp_dx(_n_secondary_vapors);
  std::vector<Real> dT_dx(_n_secondary_vapors);
  p_T_from_v_e(v, e, x, p, dp_dv, dp_de, dp_dx, T, dT_dv, dT_de, dT_dx);

  return p;
}

void
GeneralVaporMixtureFluidProperties::p_from_v_e(Real v,
                                               Real e,
                                               const std::vector<Real> & x,
                                               Real & p,
                                               Real & dp_dv,
                                               Real & dp_de,
                                               std::vector<Real> & dp_dx) const
{
  Real T, dT_dv, dT_de;
  std::vector<Real> dT_dx(_n_secondary_vapors);
  p_T_from_v_e(v, e, x, p, dp_dv, dp_de, dp_dx, T, dT_dv, dT_de, dT_dx);
}

Real
GeneralVaporMixtureFluidProperties::T_from_v_e(Real v, Real e, const std::vector<Real> & x) const
{
  Real p, dp_dv, dp_de, T, dT_dv, dT_de;
  std::vector<Real> dp_dx(_n_secondary_vapors);
  std::vector<Real> dT_dx(_n_secondary_vapors);
  p_T_from_v_e(v, e, x, p, dp_dv, dp_de, dp_dx, T, dT_dv, dT_de, dT_dx);

  return T;
}

void
GeneralVaporMixtureFluidProperties::T_from_v_e(Real v,
                                               Real e,
                                               const std::vector<Real> & x,
                                               Real & T,
                                               Real & dT_dv,
                                               Real & dT_de,
                                               std::vector<Real> & dT_dx) const
{
  Real p, dp_dv, dp_de;
  std::vector<Real> dp_dx(_n_secondary_vapors);
  p_T_from_v_e(v, e, x, p, dp_dv, dp_de, dp_dx, T, dT_dv, dT_de, dT_dx);
}

Real
GeneralVaporMixtureFluidProperties::rho_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  return 1.0 / v_from_p_T(p, T, x);
}

void
GeneralVaporMixtureFluidProperties::rho_from_p_T(Real p,
                                                 Real T,
                                                 const std::vector<Real> & x,
                                                 Real & rho,
                                                 Real & drho_dp,
                                                 Real & drho_dT,
                                                 std::vector<Real> & drho_dx) const
{
  Real v, dv_dp, dv_dT;
  std::vector<Real> dv_dx;
  v_from_p_T(p, T, x, v, dv_dp, dv_dT, dv_dx);

  rho = 1.0 / v;
  const Real drho_dv = -1.0 / (v * v);
  drho_dp = drho_dv * dv_dp;
  drho_dT = drho_dv * dv_dT;
  drho_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
    drho_dx[i] = drho_dv * dv_dx[i];
}

Real
GeneralVaporMixtureFluidProperties::v_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);

  Real v = x_primary * _fp_primary->v_from_p_T(p, T);
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
    v += x[i] * _fp_secondary[i]->v_from_p_T(p, T);

  return v;
}

void
GeneralVaporMixtureFluidProperties::v_from_p_T(Real p,
                                               Real T,
                                               const std::vector<Real> & x,
                                               Real & v,
                                               Real & dv_dp,
                                               Real & dv_dT,
                                               std::vector<Real> & dv_dx) const
{
  const Real x_primary = primaryMassFraction(x);

  Real vp, dvp_dp, dvp_dT;
  _fp_primary->v_from_p_T(p, T, vp, dvp_dp, dvp_dT);
  v = x_primary * vp;
  dv_dp = x_primary * dvp_dp;
  dv_dT = x_primary * dvp_dT;
  dv_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
  {
    Real vi, dvi_dp, dvi_dT;
    _fp_secondary[i]->v_from_p_T(p, T, vi, dvi_dp, dvi_dT);
    v += x[i] * vi;
    dv_dp += x[i] * dvi_dp;
    dv_dT += x[i] * dvi_dT;
    dv_dx[i] = vi - vp;
  }
}

Real
GeneralVaporMixtureFluidProperties::e_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);

  Real e = x_primary * _fp_primary->e_from_p_T(p, T);
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
    e += x[i] * _fp_secondary[i]->e_from_p_T(p, T);

  return e;
}

void
GeneralVaporMixtureFluidProperties::e_from_p_T(Real p,
                                               Real T,
                                               const std::vector<Real> & x,
                                               Real & e,
                                               Real & de_dp,
                                               Real & de_dT,
                                               std::vector<Real> & de_dx) const
{
  const Real x_primary = primaryMassFraction(x);

  Real ep, dep_dp, dep_dT;
  _fp_primary->e_from_p_T(p, T, ep, dep_dp, dep_dT);
  e = x_primary * ep;
  de_dp = x_primary * dep_dp;
  de_dT = x_primary * dep_dT;
  de_dx.resize(_n_secondary_vapors);
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
  {
    Real ei, dei_dp, dei_dT;
    _fp_secondary[i]->e_from_p_T(p, T, ei, dei_dp, dei_dT);
    e += x[i] * ei;
    de_dp += x[i] * dei_dp;
    de_dT += x[i] * dei_dT;
    de_dx[i] = ei - ep;
  }
}

Real
GeneralVaporMixtureFluidProperties::c_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);

  Real vp, dvp_dp_T, dvp_dT_p;
  _fp_primary->v_from_p_T(p, T, vp, dvp_dp_T, dvp_dT_p);
  Real v = x_primary * vp;
  Real dv_dp_T = x_primary * dvp_dp_T;
  Real dv_dT_p = x_primary * dvp_dT_p;

  Real sp, dsp_dp_T, dsp_dT_p;
  _fp_primary->s_from_p_T(p, T, sp, dsp_dp_T, dsp_dT_p);
  Real ds_dp_T = x_primary * dsp_dp_T;
  Real ds_dT_p = x_primary * dsp_dT_p;

  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
  {
    Real vi, dvi_dp_T, dvi_dT_p;
    _fp_secondary[i]->v_from_p_T(p, T, vi, dvi_dp_T, dvi_dT_p);
    v += x[i] * vi;
    dv_dp_T += x[i] * dvi_dp_T;
    dv_dT_p += x[i] * dvi_dT_p;

    Real si, dsi_dp_T, dsi_dT_p;
    _fp_secondary[i]->s_from_p_T(p, T, si, dsi_dp_T, dsi_dT_p);
    ds_dp_T += x[i] * dsi_dp_T;
    ds_dT_p += x[i] * dsi_dT_p;
  }

  Real dv_dp_s = dv_dp_T - dv_dT_p * ds_dp_T / ds_dT_p;

  if (dv_dp_s >= 0)
    mooseWarning(name(), ":c_from_p_T(), dv_dp_s = ", dv_dp_s, ". Should be negative.");
  return v * std::sqrt(-1. / dv_dp_s);
}

Real
GeneralVaporMixtureFluidProperties::cp_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real vp = _fp_primary->v_from_p_T(p, T);
  Real ep = _fp_primary->e_from_p_T(p, T);
  Real cp = x_primary * _fp_primary->cp_from_v_e(vp, ep);

  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
  {
    Real vi = _fp_secondary[i]->v_from_p_T(p, T);
    Real ei = _fp_secondary[i]->e_from_p_T(p, T);
    cp += x[i] * _fp_secondary[i]->cp_from_v_e(vi, ei);
  }

  return cp;
}

Real
GeneralVaporMixtureFluidProperties::cv_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real vp = _fp_primary->v_from_p_T(p, T);
  Real ep = _fp_primary->e_from_p_T(p, T);
  Real cv = x_primary * _fp_primary->cv_from_v_e(vp, ep);

  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
  {
    Real vi = _fp_secondary[i]->v_from_p_T(p, T);
    Real ei = _fp_secondary[i]->e_from_p_T(p, T);
    cv += x[i] * _fp_secondary[i]->cv_from_v_e(vi, ei);
  }

  return cv;
}

Real
GeneralVaporMixtureFluidProperties::mu_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;

  Real vp = _fp_primary->v_from_p_T(p, T);
  Real ep = _fp_primary->e_from_p_T(p, T);
  Real mu = x_primary * M_star / M_primary * _fp_primary->mu_from_v_e(vp, ep);

  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
  {
    Real vi = _fp_secondary[i]->v_from_p_T(p, T);
    Real ei = _fp_secondary[i]->e_from_p_T(p, T);
    Real Mi = _fp_secondary[i]->molarMass();
    mu += x[i] * M_star / Mi * _fp_secondary[i]->mu_from_v_e(vi, ei);
  }

  return mu;
}

Real
GeneralVaporMixtureFluidProperties::k_from_p_T(Real p, Real T, const std::vector<Real> & x) const
{
  const Real x_primary = primaryMassFraction(x);
  Real M_primary = _fp_primary->molarMass();

  Real sum = x_primary / M_primary;
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
    sum += x[i] / _fp_secondary[i]->molarMass();
  Real M_star = 1. / sum;

  Real vp = _fp_primary->v_from_p_T(p, T);
  Real ep = _fp_primary->e_from_p_T(p, T);
  Real k = x_primary * M_star / M_primary * _fp_primary->k_from_v_e(vp, ep);

  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
  {
    Real vi = _fp_secondary[i]->v_from_p_T(p, T);
    Real ei = _fp_secondary[i]->e_from_p_T(p, T);
    Real Mi = _fp_secondary[i]->molarMass();
    k += x[i] * M_star / Mi * _fp_secondary[i]->k_from_v_e(vi, ei);
  }

  return k;
}

void
GeneralVaporMixtureFluidProperties::p_T_from_v_e(
    Real v, Real e, const std::vector<Real> & x, Real & p, Real & T) const
{
  // The residuals are the following:
  // residual[0] = v(p,T,x) - v
  // residual[1] = e(p,T,x) - e
  std::vector<Real> residual(2);

  // The Jacobian columns are the following:
  // 0: pressure
  // 1: temperature
  std::vector<std::vector<Real>> jacobian(2);
  jacobian[0].resize(2);
  jacobian[1].resize(2);

  std::vector<Real> dv_dx(_n_secondary_vapors);
  std::vector<Real> de_dx(_n_secondary_vapors);

  p = _p_guess;
  T = _T_guess;
  bool converged = false;
  std::stringstream err_ss;
  err_ss << name() << ": The nonlinear solve for p and T did not converge:\n";
  err_ss << "  v = " << v << ", e = " << e;
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
    err_ss << ", x[" << i << "] = " << x[i];
  err_ss << "\n";
  for (unsigned int l = 0; l < _newton_max_its; ++l)
  {
    // specific volume residual and Jacobians from primary vapor and mixture
    Real v_it, dv_it_dp, dv_it_dT;
    std::vector<Real> dv_it_dx;
    v_from_p_T(p, T, x, v_it, dv_it_dp, dv_it_dT, dv_it_dx);
    residual[0] = v_it - v;
    jacobian[0][0] = dv_it_dp;
    jacobian[0][1] = dv_it_dT;

    // specific internal energy residual and Jacobians from primary vapor and mixture
    Real e_it, de_it_dp, de_it_dT;
    std::vector<Real> de_it_dx;
    e_from_p_T(p, T, x, e_it, de_it_dp, de_it_dT, de_it_dx);
    residual[1] = e_it - e;
    jacobian[1][0] = de_it_dp;
    jacobian[1][1] = de_it_dT;

    // compute scaled errors
    const Real scaled_residual_v = residual[0] / std::abs(v);
    const Real scaled_residual_e = residual[1] / std::abs(e);

    // check for convergence
    const Real l2_error =
        std::sqrt(std::pow(scaled_residual_v, 2) + std::pow(scaled_residual_e, 2));
    err_ss << "  Iteration " << l << ": p = " << p << ", T = " << T << ": l2 error = " << l2_error
           << "\n";
    if (l2_error < _newton_rel_tol)
    {
      converged = true;
      break;
    }

    // take Newton step: dx = -J^{-1} r
    const Real elim = jacobian[1][0] / jacobian[0][0];
    const Real T_step =
        (residual[1] - elim * residual[0]) / (jacobian[1][1] - elim * jacobian[0][1]);
    const Real p_step = (residual[0] - jacobian[0][1] * T_step) / jacobian[0][0];
    p -= p_step * _newton_damping;
    T -= T_step * _newton_damping;
  }

  if (converged)
  {
    if (_update_guesses)
    {
      _p_guess = p;
      _T_guess = T;
    }
  }
  else
  {
    err_ss << "\nTry setting the parameters 'p_initial_guess' and 'T_initial_guess'.";
    mooseWarning(err_ss.str());
    p = std::nan("");
    T = std::nan("");
  }
}

void
GeneralVaporMixtureFluidProperties::p_T_from_v_e(Real v,
                                                 Real e,
                                                 const std::vector<Real> & x,
                                                 Real & p,
                                                 Real & dp_dv,
                                                 Real & dp_de,
                                                 std::vector<Real> & dp_dx,
                                                 Real & T,
                                                 Real & dT_dv,
                                                 Real & dT_de,
                                                 std::vector<Real> & dT_dx) const
{
  p_T_from_v_e(v, e, x, p, T);

  // specific volume residual and Jacobians from primary vapor and mixture
  Real v_unused, dv_dp, dv_dT;
  std::vector<Real> dv_dx;
  v_from_p_T(p, T, x, v_unused, dv_dp, dv_dT, dv_dx);

  // specific internal energy residual and Jacobians from primary vapor and mixture
  Real e_unused, de_dp, de_dT;
  std::vector<Real> de_dx;
  e_from_p_T(p, T, x, e_unused, de_dp, de_dT, de_dx);

  // Compute derivatives using the following rules:
  //   * Reciprocity:         da/db|_c = (db/da|_c)^{-1}
  //   * Chain rule:          da/db|_c = da/dc|_b * dc/db|_a
  //   * Triple product rule: da/db|_c * db/dc|_a * dc/da|_b = -1
  dp_dv = de_dT / (dv_dp * de_dT - dv_dT * de_dp);
  dp_de = dv_dT / (de_dp * dv_dT - de_dT * dv_dp);
  dT_dv = de_dp / (dv_dT * de_dp - dv_dp * de_dT);
  dT_de = dv_dp / (de_dT * dv_dp - de_dp * dv_dT);

  // Derivatives with respect to mass fractions are more complicated, so a
  // finite difference approximation is used (this is expensive of course).
  for (unsigned int i = 0; i < _n_secondary_vapors; ++i)
  {
    Real p_perturbed, T_perturbed;
    std::vector<Real> x_perturbed(x);
    x_perturbed[i] += 1e-6;
    p_T_from_v_e(v, e, x_perturbed, p_perturbed, T_perturbed);

    dp_dx[i] = (p_perturbed - p) / 1e-6;
    dT_dx[i] = (T_perturbed - T) / 1e-6;
  }
}

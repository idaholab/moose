//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticMeanCapTC.h"
#include "RankFourTensor.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticMeanCapTC);

InputParameters
TensorMechanicsPlasticMeanCapTC::validParams()
{
  InputParameters params = TensorMechanicsPlasticModel::validParams();
  params.addRangeCheckedParam<unsigned>("max_iterations",
                                        10,
                                        "max_iterations>0",
                                        "Maximum iterations for custom MeanCapTC return map");
  params.addParam<bool>(
      "use_custom_returnMap", true, "Whether to use the custom MeanCapTC returnMap algorithm.");
  params.addParam<bool>("use_custom_cto",
                        true,
                        "Whether to use the custom consistent tangent operator computations.");
  params.addRequiredParam<UserObjectName>("tensile_strength",
                                          "A TensorMechanicsHardening UserObject that defines "
                                          "hardening of the mean-cap tensile strength (it will "
                                          "typically be positive).  Yield function = trace(stress) "
                                          "- tensile_strength for trace(stress)>tensile_strength.");
  params.addRequiredParam<UserObjectName>(
      "compressive_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "mean-cap compressive strength.  This should always be less than "
      "tensile_strength (it will typically be negative).  Yield function = "
      "- (trace(stress) - compressive_strength) for "
      "trace(stress)<compressive_strength.");
  params.addClassDescription(
      "Associative mean-cap tensile and compressive plasticity with hardening/softening");

  return params;
}

TensorMechanicsPlasticMeanCapTC::TensorMechanicsPlasticMeanCapTC(const InputParameters & parameters)
  : TensorMechanicsPlasticModel(parameters),
    _max_iters(getParam<unsigned>("max_iterations")),
    _use_custom_returnMap(getParam<bool>("use_custom_returnMap")),
    _use_custom_cto(getParam<bool>("use_custom_cto")),
    _strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _c_strength(getUserObject<TensorMechanicsHardeningModel>("compressive_strength"))
{
  // cannot check the following for all values of the internal parameter, but this will catch most
  // errors
  if (_strength.value(0) <= _c_strength.value(0))
    mooseError("MeanCapTC: tensile strength (which is usually positive) must not be less than "
               "compressive strength (which is usually negative)");
}

Real
TensorMechanicsPlasticMeanCapTC::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);
  if (tr >= t_str)
    return tr - t_str;

  const Real c_str = compressive_strength(intnl);
  if (tr <= c_str)
    return -(tr - c_str);
  // the following is zero at tr = t_str, and at tr = c_str
  // it also has derivative = 1 at tr = t_str, and derivative = -1 at tr = c_str
  // it also has second derivative = 0, at these points.
  // This makes the complete yield function C2 continuous.
  return (c_str - t_str) / libMesh::pi * std::sin(libMesh::pi * (tr - c_str) / (t_str - c_str));
}

RankTwoTensor
TensorMechanicsPlasticMeanCapTC::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                        Real intnl) const
{
  return df_dsig(stress, intnl);
}

Real
TensorMechanicsPlasticMeanCapTC::dyieldFunction_dintnl(const RankTwoTensor & stress,
                                                       Real intnl) const
{
  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);
  if (tr >= t_str)
    return -dtensile_strength(intnl);

  const Real c_str = compressive_strength(intnl);
  if (tr <= c_str)
    return dcompressive_strength(intnl);

  const Real dt = dtensile_strength(intnl);
  const Real dc = dcompressive_strength(intnl);
  return (dc - dt) / libMesh::pi * std::sin(libMesh::pi * (tr - c_str) / (t_str - c_str)) +
         1.0 / (t_str - c_str) * std::cos(libMesh::pi * (tr - c_str) / (t_str - c_str)) *
             ((tr - c_str) * dt - (tr - t_str) * dc);
}

RankTwoTensor
TensorMechanicsPlasticMeanCapTC::df_dsig(const RankTwoTensor & stress, Real intnl) const
{
  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);
  if (tr >= t_str)
    return stress.dtrace();

  const Real c_str = compressive_strength(intnl);
  if (tr <= c_str)
    return -stress.dtrace();

  return -std::cos(libMesh::pi * (tr - c_str) / (t_str - c_str)) * stress.dtrace();
}

RankTwoTensor
TensorMechanicsPlasticMeanCapTC::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  return df_dsig(stress, intnl);
}

RankFourTensor
TensorMechanicsPlasticMeanCapTC::dflowPotential_dstress(const RankTwoTensor & stress,
                                                        Real intnl) const
{
  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);
  if (tr >= t_str)
    return RankFourTensor();

  const Real c_str = compressive_strength(intnl);
  if (tr <= c_str)
    return RankFourTensor();

  return libMesh::pi / (t_str - c_str) * std::sin(libMesh::pi * (tr - c_str) / (t_str - c_str)) *
         stress.dtrace().outerProduct(stress.dtrace());
}

RankTwoTensor
TensorMechanicsPlasticMeanCapTC::dflowPotential_dintnl(const RankTwoTensor & stress,
                                                       Real intnl) const
{
  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);
  if (tr >= t_str)
    return RankTwoTensor();

  const Real c_str = compressive_strength(intnl);
  if (tr <= c_str)
    return RankTwoTensor();

  const Real dt = dtensile_strength(intnl);
  const Real dc = dcompressive_strength(intnl);
  return std::sin(libMesh::pi * (tr - c_str) / (t_str - c_str)) * stress.dtrace() * libMesh::pi /
         Utility::pow<2>(t_str - c_str) * ((tr - t_str) * dc - (tr - c_str) * dt);
}

Real
TensorMechanicsPlasticMeanCapTC::hardPotential(const RankTwoTensor & stress, Real intnl) const
{
  // This is the key for this whole class!
  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);

  if (tr >= t_str)
    return -1.0; // this will serve to *increase* the internal parameter (so internal parameter will
                 // be a measure of volumetric strain)

  const Real c_str = compressive_strength(intnl);
  if (tr <= c_str)
    return 1.0; // this will serve to *decrease* the internal parameter (so internal parameter will
                // be a measure of volumetric strain)

  return std::cos(libMesh::pi * (tr - c_str) /
                  (t_str - c_str)); // this interpolates C2 smoothly between 1 and -1
}

RankTwoTensor
TensorMechanicsPlasticMeanCapTC::dhardPotential_dstress(const RankTwoTensor & stress,
                                                        Real intnl) const
{
  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);
  if (tr >= t_str)
    return RankTwoTensor();

  const Real c_str = compressive_strength(intnl);
  if (tr <= c_str)
    return RankTwoTensor();

  return -std::sin(libMesh::pi * (tr - c_str) / (t_str - c_str)) * libMesh::pi / (t_str - c_str) *
         stress.dtrace();
}

Real
TensorMechanicsPlasticMeanCapTC::dhardPotential_dintnl(const RankTwoTensor & stress,
                                                       Real intnl) const
{
  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);
  if (tr >= t_str)
    return 0.0;

  const Real c_str = compressive_strength(intnl);
  if (tr <= c_str)
    return 0.0;

  const Real dt = dtensile_strength(intnl);
  const Real dc = dcompressive_strength(intnl);
  return -std::sin(libMesh::pi * (tr - c_str) / (t_str - c_str)) * libMesh::pi /
         Utility::pow<2>(t_str - c_str) * ((tr - t_str) * dc - (tr - c_str) * dt);
}

Real
TensorMechanicsPlasticMeanCapTC::tensile_strength(const Real internal_param) const
{
  return _strength.value(internal_param);
}

Real
TensorMechanicsPlasticMeanCapTC::dtensile_strength(const Real internal_param) const
{
  return _strength.derivative(internal_param);
}

Real
TensorMechanicsPlasticMeanCapTC::compressive_strength(const Real internal_param) const
{
  return _c_strength.value(internal_param);
}

Real
TensorMechanicsPlasticMeanCapTC::dcompressive_strength(const Real internal_param) const
{
  return _c_strength.derivative(internal_param);
}

void
TensorMechanicsPlasticMeanCapTC::activeConstraints(const std::vector<Real> & f,
                                                   const RankTwoTensor & stress,
                                                   Real intnl,
                                                   const RankFourTensor & Eijkl,
                                                   std::vector<bool> & act,
                                                   RankTwoTensor & returned_stress) const
{
  act.assign(1, false);

  if (f[0] <= _f_tol)
  {
    returned_stress = stress;
    return;
  }

  const Real tr = stress.trace();
  const Real t_str = tensile_strength(intnl);
  Real str;
  Real dirn;
  if (tr >= t_str)
  {
    str = t_str;
    dirn = 1.0;
  }
  else
  {
    str = compressive_strength(intnl);
    dirn = -1.0;
  }

  RankTwoTensor n; // flow direction
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      for (unsigned k = 0; k < 3; ++k)
        n(i, j) += dirn * Eijkl(i, j, k, k);

  // returned_stress = stress - gamma*n
  // and taking the trace of this and using
  // Tr(returned_stress) = str, gives
  // gamma = (Tr(stress) - str)/Tr(n)
  Real gamma = (stress.trace() - str) / n.trace();

  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      returned_stress(i, j) = stress(i, j) - gamma * n(i, j);

  act[0] = true;
}

bool
TensorMechanicsPlasticMeanCapTC::returnMap(const RankTwoTensor & trial_stress,
                                           Real intnl_old,
                                           const RankFourTensor & E_ijkl,
                                           Real ep_plastic_tolerance,
                                           RankTwoTensor & returned_stress,
                                           Real & returned_intnl,
                                           std::vector<Real> & dpm,
                                           RankTwoTensor & delta_dp,
                                           std::vector<Real> & yf,
                                           bool & trial_stress_inadmissible) const
{
  if (!(_use_custom_returnMap))
    return TensorMechanicsPlasticModel::returnMap(trial_stress,
                                                  intnl_old,
                                                  E_ijkl,
                                                  ep_plastic_tolerance,
                                                  returned_stress,
                                                  returned_intnl,
                                                  dpm,
                                                  delta_dp,
                                                  yf,
                                                  trial_stress_inadmissible);

  yf.resize(1);

  Real yf_orig = yieldFunction(trial_stress, intnl_old);

  yf[0] = yf_orig;

  if (yf_orig < _f_tol)
  {
    // the trial_stress is admissible
    trial_stress_inadmissible = false;
    return true;
  }

  trial_stress_inadmissible = true;

  // In the following we want to solve
  // trial_stress - stress = E_ijkl * dpm * r   ...... (1)
  // and either
  // stress.trace() = tensile_strength(intnl)  ...... (2a)
  // intnl = intnl_old + dpm                   ...... (3a)
  // or
  // stress.trace() = compressive_strength(intnl) ... (2b)
  // intnl = intnl_old - dpm                   ...... (3b)
  // The former (2a and 3a) are chosen if
  // trial_stress.trace() > tensile_strength(intnl_old)
  // while the latter (2b and 3b) are chosen if
  // trial_stress.trace() < compressive_strength(intnl_old)
  // The variables we want to solve for are stress, dpm
  // and intnl.  We do this using a Newton approach, starting
  // with stress=trial_stress and intnl=intnl_old and dpm=0
  const bool tensile_failure = (trial_stress.trace() >= tensile_strength(intnl_old));
  const Real dirn = (tensile_failure ? 1.0 : -1.0);

  RankTwoTensor n; // flow direction, which is E_ijkl * r
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      for (unsigned k = 0; k < 3; ++k)
        n(i, j) += dirn * E_ijkl(i, j, k, k);
  const Real n_trace = n.trace();

  // Perform a Newton-Raphson to find dpm when
  // residual = trial_stress.trace() - tensile_strength(intnl) - dpm * n.trace()  [for
  // tensile_failure=true]
  // or
  // residual = trial_stress.trace() - compressive_strength(intnl) - dpm * n.trace()  [for
  // tensile_failure=false]
  Real trial_trace = trial_stress.trace();
  Real residual;
  Real jac;
  dpm[0] = 0;
  unsigned int iter = 0;
  do
  {
    if (tensile_failure)
    {
      residual = trial_trace - tensile_strength(intnl_old + dpm[0]) - dpm[0] * n_trace;
      jac = -dtensile_strength(intnl_old + dpm[0]) - n_trace;
    }
    else
    {
      residual = trial_trace - compressive_strength(intnl_old - dpm[0]) - dpm[0] * n_trace;
      jac = -dcompressive_strength(intnl_old - dpm[0]) - n_trace;
    }
    dpm[0] += -residual / jac;
    if (iter > _max_iters) // not converging
      return false;
    iter++;
  } while (residual * residual > _f_tol * _f_tol);

  // set the returned values
  yf[0] = 0;
  returned_intnl = intnl_old + dirn * dpm[0];
  returned_stress = trial_stress - dpm[0] * n;
  delta_dp = dpm[0] * dirn * returned_stress.dtrace();

  return true;
}

RankFourTensor
TensorMechanicsPlasticMeanCapTC::consistentTangentOperator(
    const RankTwoTensor & trial_stress,
    Real intnl_old,
    const RankTwoTensor & stress,
    Real intnl,
    const RankFourTensor & E_ijkl,
    const std::vector<Real> & cumulative_pm) const
{
  if (!_use_custom_cto)
    return TensorMechanicsPlasticModel::consistentTangentOperator(
        trial_stress, intnl_old, stress, intnl, E_ijkl, cumulative_pm);

  Real df_dq;
  Real alpha;
  if (trial_stress.trace() >= tensile_strength(intnl_old))
  {
    df_dq = -dtensile_strength(intnl);
    alpha = 1.0;
  }
  else
  {
    df_dq = dcompressive_strength(intnl);
    alpha = -1.0;
  }

  RankTwoTensor elas;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        elas(i, j) += E_ijkl(i, j, k, k);

  const Real hw = -df_dq + alpha * elas.trace();

  return E_ijkl - alpha / hw * elas.outerProduct(elas);
}

bool
TensorMechanicsPlasticMeanCapTC::useCustomReturnMap() const
{
  return _use_custom_returnMap;
}

bool
TensorMechanicsPlasticMeanCapTC::useCustomCTO() const
{
  return _use_custom_cto;
}

std::string
TensorMechanicsPlasticMeanCapTC::modelName() const
{
  return "MeanCapTC";
}

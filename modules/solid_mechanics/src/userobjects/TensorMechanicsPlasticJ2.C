//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticJ2.h"
#include "RankFourTensor.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticJ2);

InputParameters
TensorMechanicsPlasticJ2::validParams()
{
  InputParameters params = TensorMechanicsPlasticModel::validParams();
  params.addRequiredParam<UserObjectName>(
      "yield_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the yield strength");
  params.addRangeCheckedParam<unsigned>(
      "max_iterations", 10, "max_iterations>0", "Maximum iterations for custom J2 return map");
  params.addParam<bool>("use_custom_returnMap",
                        true,
                        "Whether to use the custom returnMap "
                        "algorithm.  Set to true if you are using "
                        "isotropic elasticity.");
  params.addParam<bool>("use_custom_cto",
                        true,
                        "Whether to use the custom consistent tangent "
                        "operator computations.  Set to true if you are "
                        "using isotropic elasticity.");
  params.addClassDescription("J2 plasticity, associative, with hardening");

  return params;
}

TensorMechanicsPlasticJ2::TensorMechanicsPlasticJ2(const InputParameters & parameters)
  : TensorMechanicsPlasticModel(parameters),
    _strength(getUserObject<TensorMechanicsHardeningModel>("yield_strength")),
    _max_iters(getParam<unsigned>("max_iterations")),
    _use_custom_returnMap(getParam<bool>("use_custom_returnMap")),
    _use_custom_cto(getParam<bool>("use_custom_cto"))
{
}

Real
TensorMechanicsPlasticJ2::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  return std::sqrt(3.0 * stress.secondInvariant()) - yieldStrength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticJ2::dyieldFunction_dstress(const RankTwoTensor & stress, Real /*intnl*/) const
{
  Real sII = stress.secondInvariant();
  if (sII == 0.0)
    return RankTwoTensor();
  else
    return 0.5 * std::sqrt(3.0 / sII) * stress.dsecondInvariant();
}

Real
TensorMechanicsPlasticJ2::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/, Real intnl) const
{
  return -dyieldStrength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticJ2::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  return dyieldFunction_dstress(stress, intnl);
}

RankFourTensor
TensorMechanicsPlasticJ2::dflowPotential_dstress(const RankTwoTensor & stress, Real /*intnl*/) const
{
  Real sII = stress.secondInvariant();
  if (sII == 0)
    return RankFourTensor();

  RankFourTensor dfp = 0.5 * std::sqrt(3.0 / sII) * stress.d2secondInvariant();
  Real pre = -0.25 * std::sqrt(3.0) * std::pow(sII, -1.5);
  RankTwoTensor dII = stress.dsecondInvariant();
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      for (unsigned k = 0; k < 3; ++k)
        for (unsigned l = 0; l < 3; ++l)
          dfp(i, j, k, l) += pre * dII(i, j) * dII(k, l);
  return dfp;
}

RankTwoTensor
TensorMechanicsPlasticJ2::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                Real /*intnl*/) const
{
  return RankTwoTensor();
}

Real
TensorMechanicsPlasticJ2::yieldStrength(Real intnl) const
{
  return _strength.value(intnl);
}

Real
TensorMechanicsPlasticJ2::dyieldStrength(Real intnl) const
{
  return _strength.derivative(intnl);
}

std::string
TensorMechanicsPlasticJ2::modelName() const
{
  return "J2";
}

bool
TensorMechanicsPlasticJ2::returnMap(const RankTwoTensor & trial_stress,
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
  Real mu = E_ijkl(0, 1, 0, 1);

  // Perform a Newton-Raphson to find dpm when
  // residual = 3*mu*dpm - trial_equivalent_stress + yieldStrength(intnl_old + dpm) = 0
  Real trial_equivalent_stress = yf_orig + yieldStrength(intnl_old);
  Real residual;
  Real jac;
  dpm[0] = 0;
  unsigned int iter = 0;
  do
  {
    residual = 3.0 * mu * dpm[0] - trial_equivalent_stress + yieldStrength(intnl_old + dpm[0]);
    jac = 3.0 * mu + dyieldStrength(intnl_old + dpm[0]);
    dpm[0] += -residual / jac;
    if (iter > _max_iters) // not converging
      return false;
    iter++;
  } while (residual * residual > _f_tol * _f_tol);

  // set the returned values
  yf[0] = 0;
  returned_intnl = intnl_old + dpm[0];
  RankTwoTensor nn = 1.5 * trial_stress.deviatoric() /
                     trial_equivalent_stress; // = dyieldFunction_dstress(trial_stress, intnl_old) =
                                              // the normal to the yield surface, at the trial
                                              // stress
  returned_stress = 2.0 / 3.0 * nn * yieldStrength(returned_intnl);
  returned_stress.addIa(1.0 / 3.0 * trial_stress.trace());
  delta_dp = nn * dpm[0];

  return true;
}

RankFourTensor
TensorMechanicsPlasticJ2::consistentTangentOperator(const RankTwoTensor & trial_stress,
                                                    Real intnl_old,
                                                    const RankTwoTensor & stress,
                                                    Real intnl,
                                                    const RankFourTensor & E_ijkl,
                                                    const std::vector<Real> & cumulative_pm) const
{
  if (!_use_custom_cto)
    return TensorMechanicsPlasticModel::consistentTangentOperator(
        trial_stress, intnl_old, stress, intnl, E_ijkl, cumulative_pm);

  Real mu = E_ijkl(0, 1, 0, 1);

  Real h = 3 * mu + dyieldStrength(intnl);
  RankTwoTensor sij = stress.deviatoric();
  Real sII = stress.secondInvariant();
  Real equivalent_stress = std::sqrt(3.0 * sII);
  Real zeta = cumulative_pm[0] / (1.0 + 3.0 * mu * cumulative_pm[0] / equivalent_stress);

  return E_ijkl - 3.0 * mu * mu / sII / h * sij.outerProduct(sij) -
         4.0 * mu * mu * zeta * dflowPotential_dstress(stress, intnl);
}

bool
TensorMechanicsPlasticJ2::useCustomReturnMap() const
{
  return _use_custom_returnMap;
}

bool
TensorMechanicsPlasticJ2::useCustomCTO() const
{
  return _use_custom_cto;
}

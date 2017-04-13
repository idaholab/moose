/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "IsotropicPowerLawHardening.h"

#include "SymmIsotropicElasticityTensor.h"

/**
* Isotropic power law hardening material model. Before yield, the stress is youngs modulus * strain.
* After yielding, the stress is K* pow(strain, n) where K is the strength coefficient,  n is the
*strain
* hardening exponent and strain is the total strain.
* Yield stress is the point of intersection of these two curves.
**/

template <>
InputParameters
validParams<IsotropicPowerLawHardening>()
{
  InputParameters params = validParams<IsotropicPlasticity>();

  params.set<Real>("yield_stress") = 1.0;
  params.set<Real>("hardening_constant") = 1.0;

  params.suppressParameter<Real>("yield_stress");
  params.suppressParameter<FunctionName>("yield_stress_function");
  params.suppressParameter<Real>("hardening_constant");
  params.suppressParameter<FunctionName>("hardening_function");

  params.addRequiredParam<Real>("strength_coefficient",
                                "The strength coefficient (K) for power law hardening");
  params.addRequiredParam<Real>("strain_hardening_exponent",
                                "The strain hardening exponent (n) for power law hardening");
  return params;
}

IsotropicPowerLawHardening::IsotropicPowerLawHardening(const InputParameters & parameters)
  : IsotropicPlasticity(parameters),
    _K(parameters.get<Real>("strength_coefficient")),
    _n(parameters.get<Real>("strain_hardening_exponent"))
{
}

void
IsotropicPowerLawHardening::computeYieldStress(unsigned /*qp*/)
{
  _yield_stress = std::pow(_K / pow(_youngs_modulus, _n), 1.0 / (1.0 - _n));
}

void
IsotropicPowerLawHardening::computeStressInitialize(unsigned qp,
                                                    Real effectiveTrialStress,
                                                    const SymmElasticityTensor & elasticityTensor)
{
  _effectiveTrialStress = effectiveTrialStress;
  const SymmIsotropicElasticityTensor * eT =
      dynamic_cast<const SymmIsotropicElasticityTensor *>(&elasticityTensor);
  if (!eT)
  {
    mooseError("IsotropicPowerLawHardening requires a SymmIsotropicElasticityTensor");
  }
  _shear_modulus = eT->shearModulus();
  _youngs_modulus = eT->youngsModulus();
  computeYieldStress(qp);
  _yield_condition = effectiveTrialStress - _hardening_variable_old[qp] - _yield_stress;
  _hardening_variable[qp] = _hardening_variable_old[qp];
  _plastic_strain[qp] = _plastic_strain_old[qp];
}

Real
IsotropicPowerLawHardening::computeHardeningDerivative(unsigned /*qp*/, Real scalar)
{
  Real stress = _effectiveTrialStress - 3.0 * _shear_modulus * scalar;
  Real slope = std::pow(stress, 1.0 / _n - 1.0) / _n * (1.0 / std::pow(_K, 1.0 / _n)) -
               1.0 / _youngs_modulus;

  slope = 1.0 / slope;

  return slope;
}

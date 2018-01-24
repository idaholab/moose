//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IsotropicPowerLawHardeningStressUpdate.h"
#include "ElasticityTensorTools.h"

template <>
InputParameters
validParams<IsotropicPowerLawHardeningStressUpdate>()
{
  InputParameters params = validParams<IsotropicPlasticityStressUpdate>();
  params.addClassDescription("This class uses the discrete material in a radial return isotropic "
                             "plasticity power law hardening model, solving for the yield stress "
                             "as the intersection of the power law relation curve and Hooke's law. "
                             " This class can be used in conjunction with other creep and "
                             "plasticity materials for more complex simulations.");

  // Set and Suppress paramaters to enable calculation of the yield stress
  params.set<Real>("yield_stress") = 1.0;
  params.set<Real>("hardening_constant") = 1.0;
  params.suppressParameter<Real>("yield_stress");
  params.suppressParameter<Real>("hardening_constant");

  // Power law hardening specific parameters
  params.addRequiredParam<Real>("strength_coefficient",
                                "The strength coefficient (K) for power law hardening");
  params.addRequiredRangeCheckedParam<Real>(
      "strain_hardening_exponent",
      "strain_hardening_exponent>=0.0 & strain_hardening_exponent <=1.0",
      "The strain hardening exponent (n) for power law hardening");

  return params;
}

IsotropicPowerLawHardeningStressUpdate::IsotropicPowerLawHardeningStressUpdate(
    const InputParameters & parameters)
  : IsotropicPlasticityStressUpdate(parameters),
    _K(parameters.get<Real>("strength_coefficient")),
    _strain_hardening_exponent(parameters.get<Real>("strain_hardening_exponent"))
{
}

void
IsotropicPowerLawHardeningStressUpdate::computeStressInitialize(
    const Real effective_trial_stress, const RankFourTensor & elasticity_tensor)
{
  computeYieldStress(elasticity_tensor);

  _effective_trial_stress = effective_trial_stress;
  _yield_condition = effective_trial_stress - _hardening_variable_old[_qp] - _yield_stress;

  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
}

Real
IsotropicPowerLawHardeningStressUpdate::computeHardeningDerivative(Real scalar)
{
  const Real stress_delta = _effective_trial_stress - _three_shear_modulus * scalar;
  Real slope = std::pow(stress_delta, (1.0 / _strain_hardening_exponent - 1.0)) /
               _strain_hardening_exponent * 1.0 / std::pow(_K, 1.0 / _strain_hardening_exponent);
  slope -= 1.0 / _youngs_modulus;

  return 1.0 / slope;
}

void
IsotropicPowerLawHardeningStressUpdate::computeYieldStress(const RankFourTensor & elasticity_tensor)
{
  // Pull in the Lam\`{e} lambda, and caculate E
  const Real lambda = getIsotropicLameLambda(elasticity_tensor);
  const Real shear_modulus = _three_shear_modulus / 3.0;

  _youngs_modulus = shear_modulus * (3.0 * lambda + 2 * shear_modulus) / (lambda + shear_modulus);

  // Then solve for yield stress using equation from the header file
  _yield_stress = std::pow(_K / std::pow(_youngs_modulus, _strain_hardening_exponent),
                           1.0 / (1.0 - _strain_hardening_exponent));
  if (_yield_stress <= 0.0)
    mooseError("The yield stress must be greater than zero, but during the simulation your yield "
               "stress became less than zero.");
}

Real
IsotropicPowerLawHardeningStressUpdate::getIsotropicLameLambda(
    const RankFourTensor & elasticity_tensor)
{
  const Real lame_lambda = elasticity_tensor(0, 0, 1, 1);

  if (_mesh.dimension() == 3 && lame_lambda != elasticity_tensor(1, 1, 2, 2))
    mooseError(
        "Check to ensure that your Elasticity Tensor is truly Isotropic: different lambda values");
  return lame_lambda;
}

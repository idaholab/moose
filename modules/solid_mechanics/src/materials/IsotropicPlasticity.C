//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IsotropicPlasticity.h"

#include "SymmIsotropicElasticityTensor.h"

#include "PiecewiseLinear.h"

registerMooseObject("SolidMechanicsApp", IsotropicPlasticity);

template <>
InputParameters
validParams<IsotropicPlasticity>()
{
  InputParameters params = validParams<ReturnMappingModel>();
  params.addClassDescription("Calculates the stress and plastic strain in the general isotropic "
                             "linear strain hardening plasticity model");

  // Linear strain hardening parameters
  params.addParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
  params.addParam<FunctionName>("yield_stress_function",
                                "Yield stress as a function of temperature");
  params.addParam<Real>("hardening_constant", "Hardening slope");
  params.addParam<FunctionName>("hardening_function",
                                "True stress as a function of plastic strain");

  return params;
}

IsotropicPlasticity::IsotropicPlasticity(const InputParameters & parameters)
  : ReturnMappingModel(parameters, "plastic"),
    _yield_stress_function(
        isParamValid("yield_stress_function") ? &getFunction("yield_stress_function") : NULL),
    _yield_stress(isParamValid("yield_stress") ? getParam<Real>("yield_stress") : 0),
    _hardening_constant(isParamValid("hardening_constant") ? getParam<Real>("hardening_constant")
                                                           : 0),
    _hardening_function(isParamValid("hardening_function")
                            ? dynamic_cast<PiecewiseLinear *>(&getFunction("hardening_function"))
                            : NULL),

    _plastic_strain(declareProperty<SymmTensor>("plastic_strain")),
    _plastic_strain_old(getMaterialPropertyOld<SymmTensor>("plastic_strain")),

    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(getMaterialPropertyOld<Real>("hardening_variable"))
{
  if (isParamValid("yield_stress") && _yield_stress <= 0)
    mooseError("Yield stress must be greater than zero");

  if (_yield_stress_function == NULL && !isParamValid("yield_stress"))
    mooseError("Either yield_stress or yield_stress_function must be given");

  if ((isParamValid("hardening_constant") && isParamValid("hardening_function")) ||
      (!isParamValid("hardening_constant") && !isParamValid("hardening_function")))
    mooseError("Either hardening_constant or hardening_function must be defined");

  if (isParamValid("hardening_function") && !_hardening_function)
    mooseError("The hardening_function must be PiecewiseLinear");
}

void
IsotropicPlasticity::initQpStatefulProperties()
{
  _hardening_variable[_qp] = 0;
  ReturnMappingModel::initQpStatefulProperties();
}

void
IsotropicPlasticity::computeStressInitialize(Real effectiveTrialStress,
                                             const SymmElasticityTensor & elasticityTensor)
{
  const SymmIsotropicElasticityTensor * eT =
      dynamic_cast<const SymmIsotropicElasticityTensor *>(&elasticityTensor);
  if (!eT)
    mooseError("IsotropicPlasticity requires a SymmIsotropicElasticityTensor");

  _shear_modulus = eT->shearModulus();
  computeYieldStress();
  _yield_condition = effectiveTrialStress - _hardening_variable_old[_qp] - _yield_stress;
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
}

void
IsotropicPlasticity::computeStressFinalize(const SymmTensor & plasticStrainIncrement)
{
  _plastic_strain[_qp] += plasticStrainIncrement;
}

Real
IsotropicPlasticity::computeResidual(const Real effectiveTrialStress, const Real scalar)
{
  Real residual = 0.0;
  _hardening_slope = 0.0;
  if (_yield_condition > 0.0)
  {
    _hardening_slope = computeHardeningDerivative(scalar);
    _hardening_variable[_qp] = computeHardeningValue(scalar);

    // The order here is important.  The final term can be small, and we don't want it lost to
    // roundoff.
    residual =
        (effectiveTrialStress - _hardening_variable[_qp] - _yield_stress) / (3.0 * _shear_modulus) -
        scalar;
  }

  return residual;
}

Real
IsotropicPlasticity::computeDerivative(const Real /*effectiveTrialStress*/, const Real /*scalar*/)
{
  Real derivative(1);
  if (_yield_condition > 0)
    derivative = -1.0 - _hardening_slope / (3.0 * _shear_modulus);

  return derivative;
}

void
IsotropicPlasticity::iterationFinalize(Real scalar)
{
  if (_yield_condition > 0)
    _hardening_variable[_qp] = computeHardeningValue(scalar);
}

Real
IsotropicPlasticity::computeHardeningValue(Real scalar)
{
  Real hardening = _hardening_variable_old[_qp] + (_hardening_slope * scalar);
  if (_hardening_function)
  {
    const Real strain_old = _effective_inelastic_strain_old[_qp];
    Point p;

    hardening = _hardening_function->value(strain_old + scalar, p) - _yield_stress;
  }
  return hardening;
}

Real IsotropicPlasticity::computeHardeningDerivative(Real /*scalar*/)
{
  Real slope = _hardening_constant;
  if (_hardening_function)
  {
    const Real strain_old = _effective_inelastic_strain_old[_qp];
    Point p;

    slope = _hardening_function->timeDerivative(strain_old, p);
  }
  return slope;
}

void
IsotropicPlasticity::computeYieldStress()
{
  if (_yield_stress_function)
  {
    Point p;
    _yield_stress = _yield_stress_function->value(_temperature[_qp], p);
    if (_yield_stress <= 0)
      mooseError("Yield stress must be greater than zero");
  }
}

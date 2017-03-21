/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "IsotropicPlasticity.h"

#include "SymmIsotropicElasticityTensor.h"

#include "PiecewiseLinear.h"

template <>
InputParameters
validParams<IsotropicPlasticity>()
{
  InputParameters params = validParams<ReturnMappingModel>();

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
  : ReturnMappingModel(parameters),
    _yield_stress_function(
        isParamValid("yield_stress_function") ? &getFunction("yield_stress_function") : NULL),
    _yield_stress(isParamValid("yield_stress") ? getParam<Real>("yield_stress") : 0),
    _hardening_constant(isParamValid("hardening_constant") ? getParam<Real>("hardening_constant")
                                                           : 0),
    _hardening_function(isParamValid("hardening_function")
                            ? dynamic_cast<PiecewiseLinear *>(&getFunction("hardening_function"))
                            : NULL),

    _plastic_strain(declareProperty<SymmTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<SymmTensor>("plastic_strain")),
    _scalar_plastic_strain(_hardening_function ? &declareProperty<Real>("scalar_plastic_strain")
                                               : NULL),
    _scalar_plastic_strain_old(
        _hardening_function ? &declarePropertyOld<Real>("scalar_plastic_strain") : NULL),

    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(declarePropertyOld<Real>("hardening_variable"))
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
IsotropicPlasticity::initStatefulProperties(unsigned n_points)
{
  for (unsigned qp(0); qp < n_points; ++qp)
  {
    _hardening_variable[qp] = 0;

    if (_scalar_plastic_strain)
      (*_scalar_plastic_strain)[qp] = (*_scalar_plastic_strain_old)[qp] = 0;
  }
  ReturnMappingModel::initStatefulProperties(n_points);
}

void
IsotropicPlasticity::computeStressInitialize(unsigned qp,
                                             Real effectiveTrialStress,
                                             const SymmElasticityTensor & elasticityTensor)
{
  const SymmIsotropicElasticityTensor * eT =
      dynamic_cast<const SymmIsotropicElasticityTensor *>(&elasticityTensor);
  if (!eT)
    mooseError("IsotropicPlasticity requires a SymmIsotropicElasticityTensor");

  _shear_modulus = eT->shearModulus();
  computeYieldStress(qp);
  _yield_condition = effectiveTrialStress - _hardening_variable_old[qp] - _yield_stress;
  _hardening_variable[qp] = _hardening_variable_old[qp];
  _plastic_strain[qp] = _plastic_strain_old[qp];
}

void
IsotropicPlasticity::computeStressFinalize(unsigned qp, const SymmTensor & plasticStrainIncrement)
{
  _plastic_strain[qp] += plasticStrainIncrement;
}

Real
IsotropicPlasticity::computeResidual(unsigned qp, Real effectiveTrialStress, Real scalar)
{
  Real residual(0);
  _hardening_slope = 0;
  if (_yield_condition > 0)
  {
    _hardening_slope = computeHardeningDerivative(qp, scalar);
    _hardening_variable[qp] = computeHardeningValue(qp, scalar);

    // The order here is important.  The final term can be small, and we don't want it lost to
    // roundoff.
    residual = (effectiveTrialStress - _hardening_variable[qp] - _yield_stress) -
               (3 * _shear_modulus * scalar);
  }
  return residual;
}

Real
IsotropicPlasticity::computeDerivative(unsigned /*qp*/,
                                       Real /*effectiveTrialStress*/,
                                       Real /*scalar*/)
{
  Real derivative(1);
  if (_yield_condition > 0)
    derivative = -3 * _shear_modulus - _hardening_slope;

  return derivative;
}

void
IsotropicPlasticity::iterationFinalize(unsigned qp, Real scalar)
{
  if (_yield_condition > 0)
    _hardening_variable[qp] = computeHardeningValue(qp, scalar);

  if (_scalar_plastic_strain)
    (*_scalar_plastic_strain)[qp] = (*_scalar_plastic_strain_old)[qp] + scalar;
}

Real
IsotropicPlasticity::computeHardeningValue(unsigned qp, Real scalar)
{
  Real hardening = _hardening_variable_old[qp] + (_hardening_slope * scalar);
  if (_hardening_function)
  {
    const Real strain_old = (*_scalar_plastic_strain_old)[qp];
    Point p;

    hardening = _hardening_function->value(strain_old + scalar, p) - _yield_stress;
  }
  return hardening;
}

Real
IsotropicPlasticity::computeHardeningDerivative(unsigned qp, Real /*scalar*/)
{
  Real slope = _hardening_constant;
  if (_hardening_function)
  {
    const Real strain_old = (*_scalar_plastic_strain_old)[qp];
    Point p;

    slope = _hardening_function->timeDerivative(strain_old, p);
  }
  return slope;
}

void
IsotropicPlasticity::computeYieldStress(unsigned qp)
{
  if (_yield_stress_function)
  {
    Point p;
    _yield_stress = _yield_stress_function->value(_temperature[qp], p);
    if (_yield_stress <= 0)
      mooseError("Yield stress must be greater than zero");
  }
}

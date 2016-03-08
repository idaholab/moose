/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DiscreteRRIsotropicPlasticity.h"

template<>
InputParameters validParams<DiscreteRRIsotropicPlasticity>()
{
  InputParameters params = validParams<DiscreteRadialReturnStressIncrement>();

  // Linear strain hardening parameters
  params.addParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
  params.addParam<FunctionName>("yield_stress_function", "Yield stress as a function of temperature");
  params.addParam<Real>("hardening_constant", "Hardening slope");
  params.addParam<FunctionName>("hardening_function", "True stress as a function of plastic strain");
  params.addCoupledVar("temp", "Coupled Temperature");

  return params;
}


DiscreteRRIsotropicPlasticity::DiscreteRRIsotropicPlasticity( const InputParameters & parameters) :
    DiscreteRadialReturnStressIncrement(parameters),
    _yield_stress_function(isParamValid("yield_stress_function") ? &getFunction("yield_stress_function") : NULL),
    _yield_stress(isParamValid("yield_stress") ? getParam<Real>("yield_stress") : 0),
    _hardening_constant(isParamValid("hardening_constant") ? getParam<Real>("hardening_constant") : 0),
    _hardening_function(isParamValid("hardening_function") ? dynamic_cast<PiecewiseLinear*>(&getFunction("hardening_function")) : NULL),

    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
    _scalar_plastic_strain(declareProperty<Real>("scalar_plastic_strain")),
    _scalar_plastic_strain_old(declarePropertyOld<Real>("scalar_plastic_strain")),

    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),
    _shear_modulus(declareProperty<Real>("shear_modulus")),
    _temperature(isCoupled("temp") ? coupledValue("temp") : _zero)
{
  if (isParamValid("yield_stress") && _yield_stress <= 0)
    mooseError("Yield stress must be greater than zero");

  if ( _yield_stress_function == NULL && !isParamValid("yield_stress"))
    mooseError("Either yield_stress or yield_stress_function must be given");

  if (!isParamValid("hardening_constant") && !isParamValid("hardening_function"))
    mooseError("Either hardening_constant or hardening_function must be defined");

  if (isParamValid("hardening_constant") && isParamValid("hardening_function"))
    mooseError("Only the hardening_constant or only the hardening_function can be defined but not both");

  if (isParamValid("hardening_function") && !_hardening_function)
    mooseError("The hardening_function must be PiecewiseLinear");
}

void
DiscreteRRIsotropicPlasticity::resetQpProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
  _scalar_plastic_strain[_qp] = 0;
  if (!_hardening_function)
    _scalar_plastic_strain_old[_qp] = 0;

  DiscreteRadialReturnStressIncrement::resetQpProperties();
}

void
DiscreteRRIsotropicPlasticity::computeStressInitialize(Real effectiveTrialStress)
{
  getIsotropicShearModulus();
  computeYieldStress();
  _yield_condition = effectiveTrialStress - _hardening_variable_old[_qp] - _yield_stress;
}

Real
DiscreteRRIsotropicPlasticity::computeResidual(Real effectiveTrialStress, Real scalar)
{
  Real residual = 0;
  _hardening_slope = 0;
  if (_yield_condition > 0)
  {
    _hardening_slope = computeHardening( scalar );
    // The order here is important: the final term can be small, and we don't want it lost to roundoff.
    residual = (effectiveTrialStress - _hardening_variable[_qp] - _yield_stress) - (3 * _shear_modulus[_qp] * scalar);
    _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_slope * scalar);
  }
  return residual;
}

Real
DiscreteRRIsotropicPlasticity::computeDerivative(Real effectiveTrialStress, Real scalar)
{
  Real derivative = 1;
  if (_yield_condition > 0)
    derivative = -3 * _shear_modulus[_qp] - _hardening_slope;

  return derivative;
}

void
DiscreteRRIsotropicPlasticity::iterationFinalize(Real scalar)
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_slope * scalar);
  if (_hardening_function)
    _scalar_plastic_strain[_qp] = _scalar_plastic_strain_old[_qp] + scalar;
}

void
DiscreteRRIsotropicPlasticity::computeStressFinalize(const RankTwoTensor & plasticStrainIncrement)
{
  _plastic_strain[_qp] += plasticStrainIncrement;
}

Real
DiscreteRRIsotropicPlasticity::computeHardening(Real scalar )
{
  Real slope = _hardening_constant;
  if (_hardening_function)
  {
    const Real strain_old = _scalar_plastic_strain_old[_qp];
    Point p;

    slope = _hardening_function->timeDerivative( strain_old, p );
  }
  return slope;
}

void
DiscreteRRIsotropicPlasticity::computeYieldStress()
{
  if (_yield_stress_function)
  {
    Point p;
    _yield_stress = _yield_stress_function->value( _temperature[_qp], p );
    if (_yield_stress <= 0)
      mooseError("The yield stress must be greater than zero, but during the simulation your yield stress became less than zero.");
  }
}


void
DiscreteRRIsotropicPlasticity::getIsotropicShearModulus()
{
  _shear_modulus[_qp] = _elasticity_tensor[_qp](1,2,1,2);
  if ((_mesh.dimension() == 3) && (_shear_modulus[_qp] != _elasticity_tensor[_qp](1,3,1,3)))
    mooseError("Check to ensure that your Elasticity Tensor is truly Isotropic");
}

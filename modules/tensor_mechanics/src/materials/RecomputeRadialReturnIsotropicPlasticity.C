/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RecomputeRadialReturnIsotropicPlasticity.h"

#include "Function.h"

template<>
InputParameters validParams<RecomputeRadialReturnIsotropicPlasticity>()
{
  InputParameters params = validParams<RecomputeRadialReturn>();

  // Linear strain hardening parameters
  params.addParam<FunctionName>("yield_stress_function", "Yield stress as a function of temperature");
  params.addParam<Real>("yield_stress", 0.0, "The point at which plastic strain begins accumulating");
  params.addParam<FunctionName>("hardening_function", "True stress as a function of plastic strain");
  params.addParam<Real>("hardening_constant", 0.0, "Hardening slope");
  params.addCoupledVar("temp", 0.0, "Coupled Temperature");

  return params;
}


RecomputeRadialReturnIsotropicPlasticity::RecomputeRadialReturnIsotropicPlasticity(const InputParameters & parameters) :
    RecomputeRadialReturn(parameters),
    _yield_stress_function(isParamValid("yield_stress_function") ? &getFunction("yield_stress_function") : NULL),
    _yield_stress(getParam<Real>("yield_stress")),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _hardening_function(isParamValid("hardening_function") ? &getFunction("hardening_function") : NULL),

    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain")),
    _scalar_plastic_strain(declareProperty<Real>("scalar_plastic_strain")),
    // only make the scalar plastic strain stateful if the hardening function is used; the scalar plastic strain is needed for the hardening function derivative
    _scalar_plastic_strain_old(isParamValid("hardening_function") ? &declarePropertyOld<Real>("scalar_plastic_strain") : NULL),

    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),
    _shear_modulus(declareProperty<Real>("shear_modulus")),
    _temperature(coupledValue("temp"))
{
  if (parameters.isParamSetByUser("yield_stress") && _yield_stress <= 0.0)
    mooseError("Yield stress must be greater than zero");

  if (_yield_stress_function == NULL && !parameters.isParamSetByUser("yield_stress"))
    mooseError("Either yield_stress or yield_stress_function must be given");

  if (!parameters.isParamSetByUser("hardening_constant") && !isParamValid("hardening_function"))
    mooseError("Either hardening_constant or hardening_function must be defined");

  if (parameters.isParamSetByUser("hardening_constant") && isParamValid("hardening_function"))
    mooseError("Only the hardening_constant or only the hardening_function can be defined but not both");
}

void
RecomputeRadialReturnIsotropicPlasticity::initQpStatefulProperties()
{
  //set a default non-physical value to catch uninitalized yield condition--could cause problems later on
  _yield_condition = -1.0;
  _hardening_variable[_qp] = 0.0;
  _hardening_slope = 0.0;
  _plastic_strain[_qp].zero();

  _scalar_plastic_strain[_qp] = 0.0;
  if ((_scalar_plastic_strain_old) != NULL)
    (*_scalar_plastic_strain_old)[_qp] = 0.0;
}

void
RecomputeRadialReturnIsotropicPlasticity::resetQpProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
  _scalar_plastic_strain[_qp] = 0.;

  RecomputeRadialReturn::resetQpProperties();
}

void
RecomputeRadialReturnIsotropicPlasticity::computeStressInitialize(Real effectiveTrialStress)
{
  getIsotropicShearModulus();
  computeYieldStress();
  _yield_condition = effectiveTrialStress - _hardening_variable_old[_qp] - _yield_stress;
}

Real
RecomputeRadialReturnIsotropicPlasticity::computeResidual(Real effectiveTrialStress, Real scalar)
{
  Real residual = 0.;

  mooseAssert(_yield_condition != -1., "the yield stress was not updated by computeStressInitialize");

  if (_yield_condition > 0.)
  {
    _hardening_slope = computeHardening(scalar);
    _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_slope * scalar);

    // The order here is important: the final term can be small, and we don't want it lost to roundoff.
    residual = effectiveTrialStress - _yield_stress - _hardening_variable[_qp];
    residual -= 3.0 * _shear_modulus[_qp] * scalar;
  }
  return residual;
}

Real
RecomputeRadialReturnIsotropicPlasticity::computeDerivative(Real /*effectiveTrialStress*/, Real /*scalar*/)
{
  Real derivative = 1.0;
  if (_yield_condition > 0.0)
    derivative = -3.0 * _shear_modulus[_qp] - _hardening_slope;

  return derivative;
}

void
RecomputeRadialReturnIsotropicPlasticity::iterationFinalize(Real scalar)
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_slope * scalar);

  if (_hardening_function)
    _scalar_plastic_strain[_qp] = (*_scalar_plastic_strain_old)[_qp] + scalar;
}

void
RecomputeRadialReturnIsotropicPlasticity::computeStressFinalize(const RankTwoTensor & plasticStrainIncrement)
{
  _plastic_strain[_qp] += plasticStrainIncrement;
}

Real
RecomputeRadialReturnIsotropicPlasticity::computeHardening(Real /*scalar*/)
{
  Real slope = _hardening_constant;
  if (_hardening_function)
  {
    const Real strain_old = (*_scalar_plastic_strain_old)[_qp];
    Point p;    // Always (0,0,0)

    slope = _hardening_function->timeDerivative(strain_old, p);
  }
  return slope;
}

void
RecomputeRadialReturnIsotropicPlasticity::computeYieldStress()
{
  if (_yield_stress_function)
  {
    Point p;
    _yield_stress = _yield_stress_function->value(_temperature[_qp], p);
    if (_yield_stress <= 0.0)
      mooseError("The yield stress must be greater than zero, but during the simulation your yield stress became less than zero.");
  }
}


void
RecomputeRadialReturnIsotropicPlasticity::getIsotropicShearModulus()
{
  _shear_modulus[_qp] = _elasticity_tensor[_qp](1,2,1,2);
  if ((_mesh.dimension() == 3) && (_shear_modulus[_qp] != _elasticity_tensor[_qp](1,3,1,3)))
    mooseError("Check to ensure that your Elasticity Tensor is truly Isotropic");
}

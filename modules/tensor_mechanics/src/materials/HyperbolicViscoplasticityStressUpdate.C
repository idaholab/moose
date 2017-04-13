/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HyperbolicViscoplasticityStressUpdate.h"

#include "Function.h"

template <>
InputParameters
validParams<HyperbolicViscoplasticityStressUpdate>()
{
  InputParameters params = validParams<RadialReturnStressUpdate>();
  params.addClassDescription("This class uses the discrete material for a hyperbolic sine "
                             "viscoplasticity model in which the effective plastic strain is "
                             "solved for using a creep approach.");

  // Linear strain hardening parameters
  params.addRequiredParam<Real>("yield_stress",
                                "The point at which plastic strain begins accumulating");
  params.addRequiredParam<Real>("hardening_constant", "Hardening slope");

  // Viscoplasticity constitutive equation parameters
  params.addRequiredParam<Real>("c_alpha",
                                "Viscoplasticity coefficient, scales the hyperbolic function");
  params.addRequiredParam<Real>("c_beta",
                                "Viscoplasticity coefficient inside the hyperbolic sin function");

  return params;
}

HyperbolicViscoplasticityStressUpdate::HyperbolicViscoplasticityStressUpdate(
    const InputParameters & parameters)
  : RadialReturnStressUpdate(parameters),
    _yield_stress(parameters.get<Real>("yield_stress")),
    _hardening_constant(parameters.get<Real>("hardening_constant")),
    _c_alpha(parameters.get<Real>("c_alpha")),
    _c_beta(parameters.get<Real>("c_beta")),
    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),

    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<RankTwoTensor>("plastic_strain"))
{
}

void
HyperbolicViscoplasticityStressUpdate::initQpStatefulProperties()
{
  // set a default non-physical value to catch uninitalized yield condition--could cause problems
  // later on
  _yield_condition = -1.0;
  _hardening_variable[_qp] = 0.0;
  _hardening_variable_old[_qp] = 0.0;
  _plastic_strain[_qp].zero();
}

void
HyperbolicViscoplasticityStressUpdate::computeStressInitialize(Real effectiveTrialStress)
{
  _shear_modulus = getIsotropicShearModulus();

  _yield_condition = effectiveTrialStress - _hardening_variable_old[_qp] - _yield_stress;
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
}

Real
HyperbolicViscoplasticityStressUpdate::computeResidual(Real effectiveTrialStress, Real scalar)
{
  Real residual = 0.0;

  mooseAssert(_yield_condition != -1.,
              "the yield stress was not updated by computeStressInitialize");

  if (_yield_condition > 0.0)
  {
    Real xflow = _c_beta * (effectiveTrialStress - (3.0 * _shear_modulus * scalar) -
                            _hardening_variable[_qp] - _yield_stress);
    Real xphi = _c_alpha * std::sinh(xflow);
    _xphidp = -3.0 * _shear_modulus * _c_alpha * _c_beta * std::cosh(xflow);
    _xphir = -_c_alpha * _c_beta * std::cosh(xflow);
    residual = xphi - scalar / _dt;
  }
  return residual;
}

Real HyperbolicViscoplasticityStressUpdate::computeDerivative(Real /*effectiveTrialStress*/,
                                                              Real /*scalar*/)
{
  Real derivative = 1.0;
  if (_yield_condition > 0.0)
    derivative = _xphidp + _hardening_constant * _xphir - 1 / _dt;

  return derivative;
}

void
HyperbolicViscoplasticityStressUpdate::iterationFinalize(Real scalar)
{
  if (_yield_condition > 0.0)
    _hardening_variable[_qp] = _hardening_variable_old[_qp] + (_hardening_constant * scalar);
}

void
HyperbolicViscoplasticityStressUpdate::computeStressFinalize(
    const RankTwoTensor & plasticStrainIncrement)
{
  _plastic_strain[_qp] += plasticStrainIncrement;
}

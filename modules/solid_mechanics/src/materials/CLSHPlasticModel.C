/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CLSHPlasticModel.h"

#include "SymmIsotropicElasticityTensor.h"
#include <cmath>

template <>
InputParameters
validParams<CLSHPlasticModel>()
{
  InputParameters params = validParams<ConstitutiveModel>();
  params.addRequiredParam<Real>("yield_stress",
                                "The point at which plastic strain begins accumulating");
  params.addRequiredParam<Real>("hardening_constant", "Hardening slope");
  params.addRequiredParam<Real>("c_alpha", "creep constant");
  params.addRequiredParam<Real>("c_beta", "creep constant");
  return params;
}

CLSHPlasticModel::CLSHPlasticModel(const InputParameters & parameters)
  : ReturnMappingModel(parameters),
    _yield_stress(parameters.get<Real>("yield_stress")),
    _hardening_constant(parameters.get<Real>("hardening_constant")),
    _c_alpha(parameters.get<Real>("c_alpha")),
    _c_beta(parameters.get<Real>("c_beta")),
    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),
    _plastic_strain(declareProperty<SymmTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<SymmTensor>("plastic_strain"))
{
}

void
CLSHPlasticModel::initStatefulProperties(unsigned n_points)
{
  for (unsigned qp = 0; qp < n_points; ++qp)
  {
    _hardening_variable[qp] = 0;
    _hardening_variable_old[qp] = 0;
  }
  ReturnMappingModel::initStatefulProperties(n_points);
}

void
CLSHPlasticModel::computeStressInitialize(unsigned qp,
                                          Real effectiveTrialStress,
                                          const SymmElasticityTensor & elasticityTensor)
{
  const SymmIsotropicElasticityTensor * eT =
      dynamic_cast<const SymmIsotropicElasticityTensor *>(&elasticityTensor);
  if (!eT)
  {
    mooseError("CLSHPlasticModel requires a SymmIsotropicElasticityTensor");
  }
  _shear_modulus = eT->shearModulus();
  _yield_condition = effectiveTrialStress - _hardening_variable_old[qp] - _yield_stress;
  _hardening_variable[qp] = _hardening_variable_old[qp];
  _plastic_strain[qp] = _plastic_strain_old[qp];
}

Real
CLSHPlasticModel::computeResidual(unsigned qp, Real effectiveTrialStress, Real scalar)
{
  Real residual(0);
  if (_yield_condition > 0)
  {
    Real xflow = _c_beta * (effectiveTrialStress - (3. * _shear_modulus * scalar) -
                            _hardening_variable[qp] - _yield_stress);
    Real xphi = _c_alpha * std::sinh(xflow);
    _xphidp = -3. * _shear_modulus * _c_alpha * _c_beta * std::cosh(xflow);
    _xphir = -_c_alpha * _c_beta * std::cosh(xflow);
    residual = xphi - scalar / _dt;
  }
  return residual;
}

void
CLSHPlasticModel::iterationFinalize(unsigned qp, Real scalar)
{
  _hardening_variable[qp] = _hardening_variable_old[qp] + (_hardening_constant * scalar);
}

Real
CLSHPlasticModel::computeDerivative(unsigned /*qp*/, Real /*effectiveTrialStress*/, Real /*scalar*/)
{
  Real derivative(1);
  if (_yield_condition > 0)
  {
    derivative = _xphidp + _hardening_constant * _xphir - 1 / _dt;
  }
  return derivative;
}

void
CLSHPlasticModel::computeStressFinalize(unsigned qp, const SymmTensor & plasticStrainIncrement)
{
  _plastic_strain[qp] += plasticStrainIncrement;
}

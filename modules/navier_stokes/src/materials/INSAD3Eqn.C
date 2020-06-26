//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSAD3Eqn.h"
#include "INSADObjectTracker.h"

registerMooseObject("NavierStokesApp", INSAD3Eqn);

InputParameters
INSAD3Eqn::validParams()
{
  InputParameters params = INSADMaterial::validParams();
  params.addClassDescription("This material computes properties needed for stabilized formulations "
                             "of the mass, momentum, and energy equations.");
  params.addRequiredCoupledVar("temperature", "The temperature");
  params.addParam<MaterialPropertyName>("cp_name", "cp", "The name of the specific heat capacity");
  params.addParam<MaterialPropertyName>(
      "k_name", "k", "the name of the thermal conductivity material property");
  params.addParam<MaterialPropertyName>(
      "grad_k_name",
      "grad_k",
      "the name of the gradient of the thermal conductivity material property");
  return params;
}

INSAD3Eqn::INSAD3Eqn(const InputParameters & parameters)
  : INSADMaterial(parameters),
    _grad_temperature(adCoupledGradient("temperature")),
    _second_temperature(adCoupledSecond("temperature")),
    _temperature_dot(nullptr),
    _cp(getADMaterialProperty<Real>("cp_name")),
    _k(getADMaterialProperty<Real>("k_name")),
    _grad_k(hasADMaterialProperty<RealVectorValue>("grad_k_name")
                ? &getADMaterialProperty<RealVectorValue>("grad_k_name")
                : nullptr),
    _temperature_strong_residual(declareADProperty<Real>("temperature_strong_residual")),
    _temperature_convective_strong_residual(
        declareADProperty<Real>("temperature_convective_strong_residual")),
    _temperature_td_strong_residual(declareADProperty<Real>("temperature_td_strong_residual"))
{
}

void
INSAD3Eqn::initialSetup()
{
  INSADMaterial::initialSetup();

  if (_has_transient)
    _temperature_dot = &adCoupledDot("temperature");
}

void
INSAD3Eqn::computeQpProperties()
{
  INSADMaterial::computeQpProperties();

  // Start with the conductive term
  _temperature_strong_residual[_qp] = -_k[_qp] * _second_temperature[_qp].tr();
  if (_grad_k)
    _temperature_strong_residual[_qp] -= (*_grad_k)[_qp] * _grad_temperature[_qp];

  // For the remaining terms we make individual properties so they can be consumed by non-SUPG
  // kernels. This avoids double calculation for the non-supg and supg parts of the residual. We
  // don't need an individual property for the conductive term because the corresponding non-supg
  // contribution is integrated by parts and hence there is no double calculation (the 'weak' and
  // 'strong' terms are diferent in this case)

  _temperature_convective_strong_residual[_qp] =
      _rho[_qp] * _cp[_qp] * _velocity[_qp] * _grad_temperature[_qp];
  _temperature_strong_residual[_qp] += _temperature_convective_strong_residual[_qp];

  if (_has_transient)
  {
    mooseAssert(_temperature_dot, "The temperature time derivative is null");
    _temperature_td_strong_residual[_qp] = _cp[_qp] * _rho[_qp] * (*_temperature_dot)[_qp];
    _temperature_strong_residual[_qp] += _temperature_td_strong_residual[_qp];
  }
}

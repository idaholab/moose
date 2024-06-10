//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADSmagorinskyEddyViscosity.h"

registerMooseObject("NavierStokesApp", INSADSmagorinskyEddyViscosity);

InputParameters
INSADSmagorinskyEddyViscosity::validParams()
{
  InputParameters params = ADVectorKernelGrad::validParams();
  params.addClassDescription("Computes eddy viscosity term using Smagorinky's LES model");
  params.addParam<Real>("smagorinsky_constant", 0.18, "Value of Smagorinsky's constant to use");
  params.addParam<MaterialPropertyName>(
      "rho_name", "rho", "The name of the density material property");
  params.addCoupledVar(
      "mixing_length",
      "Turbulent eddy mixing length. If this is not provided, then the product of the Smagorinsky "
      "constant and the cube root of the element volume will be used instead.");
  return params;
}

INSADSmagorinskyEddyViscosity::INSADSmagorinskyEddyViscosity(const InputParameters & parameters)
  : ADVectorKernelGrad(parameters),
    _rho(getADMaterialProperty<Real>("rho_name")),
    _smagorinsky_constant(getParam<Real>("smagorinsky_constant")),
    _mixing_length(isCoupled("mixing_length") ? &coupledValue("mixing_length") : nullptr)
{
  if (isParamSetByUser("smagorinsky_constant") && _mixing_length)
    paramError("smagorinsky_constant",
               "The Smagorinsky constant is not used if the mixing length is provided");
}

ADRealTensorValue
INSADSmagorinskyEddyViscosity::precomputeQpResidual()
{
  constexpr Real offset = 1e-15; // prevents explosion of sqrt(x) derivative to infinity
  const ADReal strain_rate_tensor_mag = std::sqrt(
      2.0 * Utility::pow<2>(_grad_u[_qp](0, 0)) + 2.0 * Utility::pow<2>(_grad_u[_qp](1, 1)) +
      2.0 * Utility::pow<2>(_grad_u[_qp](2, 2)) +
      Utility::pow<2>(_grad_u[_qp](0, 2) + _grad_u[_qp](2, 0)) +
      Utility::pow<2>(_grad_u[_qp](0, 1) + _grad_u[_qp](1, 0)) +
      Utility::pow<2>(_grad_u[_qp](1, 2) + _grad_u[_qp](2, 1)) + offset);
  constexpr Real one_third = 1.0 / 3.0;
  const auto lc = _mixing_length
                      ? (*_mixing_length)[_qp]
                      : _smagorinsky_constant * std::pow(_current_elem_volume, one_third) /
                            _current_elem->default_order();
  return strain_rate_tensor_mag * Utility::pow<2>(lc) * _rho[_qp] * _grad_u[_qp];
}

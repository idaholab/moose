//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVMomentumFriction.h"

registerMooseObject("MooseApp", NSFVMomentumFriction);

InputParameters
NSFVMomentumFriction::validParams()
{
  InputParameters params = FVElementalKernel::validParams();

  params.addClassDescription("Implements a basic linear or quadratic friction model as "
                             "a volumetric force, for example for the X-momentum equation: "
                             "F = - C * |v_x| (^2). The linear dependence "
                             "is expected for laminar flow, while the quadratic dependence "
                             "is more common for turbulent flow.");
  params.addParam<Real>("linear_coef", "Linear friction coefficient");
  params.addParam<MaterialPropertyName>(
      "linear_coef_name", "Linear friction coefficient name as a material property");
  params.addParam<Real>("quadratic_coef", "Quadratic friction coefficient");
  params.addParam<MaterialPropertyName>(
      "quadratic_coef_name", "Quadratic friction coefficient name as a material property");
  return params;
}

NSFVMomentumFriction::NSFVMomentumFriction(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _linear_friction_coef(isParamValid("linear_coef") ? getParam<Real>("linear_coef") : 0),
    _linear_friction_matprop(
        isParamValid("linear_coef_name") ? &getADMaterialProperty<Real>("linear_coef_name") : nullptr),
    _quadratic_friction_coef(isParamValid("quadratic_coef") ? getParam<Real>("quadratic_coef") : 0),
    _quadratic_friction_matprop(
        isParamValid("quadratic_coef_name") ? &getADMaterialProperty<Real>("quadratic_coef_name") : nullptr),
    _use_linear_friction_coefficient(isParamValid("linear_coef")),
    _use_linear_friction_matprop(isParamValid("linear_coef_name")),
    _use_quadratic_friction_coefficient(isParamValid("quadratic_coef")),
    _use_quadratic_friction_matprop(isParamValid("quadratic_coef_name"))
{
  // Check that at most one friction coefficient has been provided
  if (_use_linear_friction_coefficient + _use_linear_friction_matprop +
      _use_quadratic_friction_coefficient + _use_quadratic_friction_matprop != 1)
    mooseError("NSFVMomentumFriction should be provided with only one friction "
               "coefficient or material property");
}

ADReal
NSFVMomentumFriction::computeQpResidual()
{
  if (_use_linear_friction_coefficient)
    return -_linear_friction_coef * std::abs(_u[_qp]);
  else if (_use_linear_friction_matprop)
    return -(*_linear_friction_matprop)[_qp] * std::abs(_u[_qp]);
  else if (_use_quadratic_friction_coefficient)
    return -_quadratic_friction_coef * std::pow(_u[_qp], 2);
  else
    return -(*_quadratic_friction_matprop)[_qp] * std::pow(_u[_qp], 2);
}

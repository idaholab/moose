//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVMomentumFriction.h"

registerMooseObject("NavierStokesApp", NSFVMomentumFriction);

InputParameters
NSFVMomentumFriction::validParams()
{
  InputParameters params = FVElementalKernel::validParams();

  params.addClassDescription("Implements a basic linear or quadratic friction model as "
                             "a volumetric force, for example for the X-momentum equation: "
                             "$F_x = - C_l * v_x$ and $F_x = - C_q * v_x * |v_x|$ for the linear "
                             "and quadratic models respectively. A linear dependence "
                             "is expected for laminar flow, while a quadratic dependence "
                             "is more common for turbulent flow.");
  params.addParam<MaterialPropertyName>("linear_coef_name",
                                        "Linear friction coefficient name as a material property");
  params.addParam<MaterialPropertyName>(
      "quadratic_coef_name", "Quadratic friction coefficient name as a material property");
  params.addParam<MaterialPropertyName>(
      "drag_quantity",
      "the quantity that the drag force is proportional to. If this is not supplied, then the "
      "variable value will be used.");
  return params;
}

NSFVMomentumFriction::NSFVMomentumFriction(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _linear_friction_matprop(isParamValid("linear_coef_name")
                                 ? &getADMaterialProperty<Real>("linear_coef_name")
                                 : nullptr),
    _quadratic_friction_matprop(isParamValid("quadratic_coef_name")
                                    ? &getADMaterialProperty<Real>("quadratic_coef_name")
                                    : nullptr),
    _use_linear_friction_matprop(isParamValid("linear_coef_name")),
    _drag_quantity(
        isParamValid("drag_quantity") ? getADMaterialProperty<Real>("drag_quantity").get() : _u)
{
  // Check that one and at most one friction coefficient has been provided
  if (isParamValid("linear_coef_name") + isParamValid("quadratic_coef_name") != 1)
    mooseError("NSFVMomentumFriction should be provided with one and only one friction "
               "coefficient material property");
}

ADReal
NSFVMomentumFriction::computeQpResidual()
{
  if (_use_linear_friction_matprop)
    return (*_linear_friction_matprop)[_qp] * _drag_quantity[_qp];
  else
    return (*_quadratic_friction_matprop)[_qp] * _drag_quantity[_qp] *
           std::abs(_drag_quantity[_qp]);
}

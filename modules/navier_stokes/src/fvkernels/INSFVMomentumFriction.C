//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumFriction.h"

registerMooseObject("NavierStokesApp", INSFVMomentumFriction);

InputParameters
INSFVMomentumFriction::validParams()
{
  InputParameters params = INSFVElementalKernel::validParams();

  params.addClassDescription("Implements a basic linear or quadratic friction model as "
                             "a volumetric force, for example for the X-momentum equation: "
                             "$F_x = - C_l * v_x$ and $F_x = - C_q * v_x * |v_x|$ for the linear "
                             "and quadratic models respectively. A linear dependence "
                             "is expected for laminar flow, while a quadratic dependence "
                             "is more common for turbulent flow.");
  params.addParam<MooseFunctorName>("linear_coef_name",
                                    "Linear friction coefficient name as a material property");
  params.addParam<MooseFunctorName>("quadratic_coef_name",
                                    "Quadratic friction coefficient name as a material property");
  params.addParam<MooseFunctorName>(
      "drag_quantity",
      "the quantity that the drag force is proportional to. If this is not supplied, then the "
      "variable value will be used.");
  return params;
}

INSFVMomentumFriction::INSFVMomentumFriction(const InputParameters & parameters)
  : INSFVElementalKernel(parameters),
    _linear_friction(isParamValid("linear_coef_name") ? &getFunctor<ADReal>("linear_coef_name")
                                                      : nullptr),
    _quadratic_friction(
        isParamValid("quadratic_coef_name") ? &getFunctor<ADReal>("quadratic_coef_name") : nullptr),
    _use_linear_friction(isParamValid("linear_coef_name")),
    _drag_quantity(isParamValid("drag_quantity") ? getFunctor<ADReal>("drag_quantity") : _u_functor)
{
  // Check that one and at most one friction coefficient has been provided
  if (isParamValid("linear_coef_name") + isParamValid("quadratic_coef_name") != 1)
    mooseError("INSFVMomentumFriction should be provided with one and only one friction "
               "coefficient material property");
}

void
INSFVMomentumFriction::gatherRCData(const Elem & elem)
{
  const auto elem_arg = makeElemArg(&elem);
  _rc_uo.addToB(&elem,
                _index,
                _use_linear_friction ? (*_linear_friction)(elem_arg)*_drag_quantity(elem_arg)
                                     : (*_quadratic_friction)(elem_arg)*_drag_quantity(elem_arg) *
                                           std::abs(_drag_quantity(elem_arg)));
}

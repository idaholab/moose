//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumFriction.h"
#include "SystemBase.h"
#include "MooseVariableFV.h"

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
  return params;
}

INSFVMomentumFriction::INSFVMomentumFriction(const InputParameters & parameters)
  : INSFVElementalKernel(parameters),
    _linear_friction(isParamValid("linear_coef_name") ? &getFunctor<ADReal>("linear_coef_name")
                                                      : nullptr),
    _quadratic_friction(
        isParamValid("quadratic_coef_name") ? &getFunctor<ADReal>("quadratic_coef_name") : nullptr)
{
  if (!isParamValid("linear_coef_name") && !isParamValid("quadratic_coef_name"))
    mooseError("INSFVMomentumFriction should be provided with at least one friction coefficiant!");
}

void
INSFVMomentumFriction::gatherRCData(const Elem & elem)
{
  const auto & elem_arg = makeElemArg(&elem);
  const auto state = determineState();

  ADReal coefficient = 0.0;
  if (_linear_friction)
    coefficient += (*_linear_friction)(elem_arg, determineState());
  if (_quadratic_friction)
    coefficient += (*_quadratic_friction)(elem_arg, state) * std::abs(_u_functor(elem_arg, state));

  coefficient *= _assembly.elementVolume(&elem);

  _rc_uo.addToA(&elem, _index, coefficient);

  const auto dof_number = elem.dof_number(_sys.number(), _var.number(), 0);
  processResidualAndJacobian(coefficient * _u_functor(elem_arg, state), dof_number);
}

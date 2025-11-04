//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectionHeatTransferBC.h"

registerMooseObject("ThermalHydraulicsApp", ADConvectionHeatTransferBC);

InputParameters
ADConvectionHeatTransferBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor");
  params.addRequiredParam<MooseFunctorName>("htc_ambient",
                                            "Ambient heat transfer coefficient functor");
  params.addParam<MooseFunctorName>(
      "scale", 1.0, "Functor by which to scale the boundary condition");
  params.addClassDescription("Adds a convective heat flux boundary condition with user-specified "
                             "ambient temperature and heat transfer coefficient functions");
  return params;
}

ADConvectionHeatTransferBC::ADConvectionHeatTransferBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _T_ambient(getFunctor<ADReal>("T_ambient")),
    _htc_ambient(getFunctor<ADReal>("htc_ambient")),
    _scale(getFunctor<ADReal>("scale"))
{
}

ADReal
ADConvectionHeatTransferBC::computeQpResidual()
{
  const Moose::ElemSideQpArg space_arg = {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  const auto scale = _scale(space_arg, Moose::currentState());
  const auto htc = _htc_ambient(space_arg, Moose::currentState());
  const auto T_ambient = _T_ambient(space_arg, Moose::currentState());

  return scale * htc * (_u[_qp] - T_ambient) * _test[_i][_qp];
}

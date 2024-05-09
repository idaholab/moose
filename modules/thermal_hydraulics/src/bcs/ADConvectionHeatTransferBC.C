//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectionHeatTransferBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADConvectionHeatTransferBC);

InputParameters
ADConvectionHeatTransferBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature function");
  params.addRequiredParam<FunctionName>("htc_ambient",
                                        "Ambient heat transfer coefficient function");
  params.addParam<MooseFunctorName>(
      "scale", 1.0, "Functor by which to scale the boundary condition");
  params.addClassDescription("Adds a convective heat flux boundary condition with user-specified "
                             "ambient temperature and heat transfer coefficient functions");
  return params;
}

ADConvectionHeatTransferBC::ADConvectionHeatTransferBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _T_ambient_fn(getFunction("T_ambient")),
    _htc_ambient_fn(getFunction("htc_ambient")),
    _scale(getFunctor<ADReal>("scale"))
{
}

ADReal
ADConvectionHeatTransferBC::computeQpResidual()
{
  const Moose::ElemSideQpArg space_arg = {_current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
  return _scale(space_arg, Moose::currentState()) * _htc_ambient_fn.value(_t, _q_point[_qp]) *
         (_u[_qp] - _T_ambient_fn.value(_t, _q_point[_qp])) * _test[_i][_qp];
}

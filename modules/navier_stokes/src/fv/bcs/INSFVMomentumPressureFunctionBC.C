//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumPressureFunctionBC.h"

#include "NS.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSFVMomentumPressureFunctionBC);

InputParameters
INSFVMomentumPressureFunctionBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this BC applies to.");
  params.addRequiredCoupledVar("p", "the pressure variable");
  params.addRequiredParam<FunctionName>("pressure_exact_solution", "The exact pressure solution.");
  return params;
}

INSFVMomentumPressureFunctionBC::INSFVMomentumPressureFunctionBC(const InputParameters & params)
  : FVFluxBC(params),
    _index(getParam<MooseEnum>("momentum_component")),
    _pressure(adCoupledValue("p")),
    _pressure_exact_solution(getFunction("pressure_exact_solution"))
{
}

ADReal
INSFVMomentumPressureFunctionBC::computeQpResidual()
{
  ADReal pressure_face;
  auto pressure_ghost = _pressure_exact_solution.value(
      _t, 2. * _face_info->faceCentroid() - _face_info->elemCentroid());

  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                         pressure_face,
                         _pressure[_qp],
                         pressure_ghost,
                         *_face_info,
                         true);

  return _normal(_index) * pressure_face;
}

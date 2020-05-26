//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetForcingFunctionSUPG.h"
#include "Function.h"

registerMooseObject("LevelSetApp", LevelSetForcingFunctionSUPG);

InputParameters
LevelSetForcingFunctionSUPG::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("The SUPG stablization term for a forcing function.");
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addRequiredCoupledVar("velocity", "Velocity vector variable.");
  return params;
}

LevelSetForcingFunctionSUPG::LevelSetForcingFunctionSUPG(const InputParameters & parameters)
  : ADKernelGrad(parameters),
    _function(getFunction("function")),
    _velocity(adCoupledVectorValue("velocity"))
{
}

ADRealVectorValue
LevelSetForcingFunctionSUPG::precomputeQpResidual()
{
  ADReal tau =
      _current_elem->hmin() /
      (2 * (_velocity[_qp] + RealVectorValue(libMesh::TOLERANCE * libMesh::TOLERANCE)).norm());
  return -tau * _velocity[_qp] * _function.value(_t, _q_point[_qp]);
}

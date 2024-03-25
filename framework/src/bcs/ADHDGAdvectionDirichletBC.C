//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGAdvectionDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", ADHDGAdvectionDirichletBC);

InputParameters
ADHDGAdvectionDirichletBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addRequiredParam<FunctionName>("exact_soln", "The exact solution.");
  return params;
}

ADHDGAdvectionDirichletBC::ADHDGAdvectionDirichletBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _func(getFunction("exact_soln"))
{
}

ADReal
ADHDGAdvectionDirichletBC::computeQpResidual()
{
  const auto vdotn = _velocity[_qp] * _normals[_qp];
  if (MetaPhysicL::raw_value(vdotn) >= 0)
    // outflow
    return _test[_i][_qp] * vdotn * _u[_qp];
  else
    // inflow
    return _test[_i][_qp] * vdotn * _func.value(_t, _q_point[_qp]);
}

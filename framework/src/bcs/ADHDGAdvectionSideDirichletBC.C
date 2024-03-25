//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHDGAdvectionSideDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", ADHDGAdvectionSideDirichletBC);

InputParameters
ADHDGAdvectionSideDirichletBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<MaterialPropertyName>("velocity", "Velocity vector");
  params.addRequiredParam<FunctionName>("exact_soln", "The exact solution.");
  params.addRequiredCoupledVar("interior_variable", "interior variable to find jumps in");
  return params;
}

ADHDGAdvectionSideDirichletBC::ADHDGAdvectionSideDirichletBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _velocity(getADMaterialProperty<RealVectorValue>("velocity")),
    _func(getFunction("exact_soln")),
    _interior_value(adCoupledValue("interior_variable"))
{
}

ADReal
ADHDGAdvectionSideDirichletBC::computeQpResidual()
{
  const auto vdotn = _velocity[_qp] * _normals[_qp];
  if (MetaPhysicL::raw_value(vdotn) >= 0)
    // outflow
    return -_test[_i][_qp] * vdotn * _interior_value[_qp];
  else
    // inflow
    return -_test[_i][_qp] * vdotn * _func.value(_t, _q_point[_qp]);
}

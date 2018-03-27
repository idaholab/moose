//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPenaltyDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseTestApp", VectorPenaltyDirichletBC);

template <>
InputParameters
validParams<VectorPenaltyDirichletBC>()
{
  InputParameters params = validParams<VectorIntegratedBC>();
  params.addRequiredParam<Real>("penalty", "The penalty coefficient");
  params.addParam<FunctionName>("x_exact_sln", 0, "The exact solution for the x component");
  params.addParam<FunctionName>("y_exact_sln", 0, "The exact solution for the y component");
  params.addParam<FunctionName>("z_exact_sln", 0, "The exact solution for the z component");
  params.addParam<bool>("linear", false, "Whether this is formulated as a linear problem");
  return params;
}

VectorPenaltyDirichletBC::VectorPenaltyDirichletBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),
    _penalty(getParam<Real>("penalty")),
    _exact_x(getFunction("x_exact_sln")),
    _exact_y(getFunction("y_exact_sln")),
    _exact_z(getFunction("y_exact_sln")),
    _linear(getParam<bool>("linear"))
{
}

Real
VectorPenaltyDirichletBC::computeQpResidual()
{
  RealVectorValue u_exact = {_exact_x.value(_t, _q_point[_qp]),
                             _exact_y.value(_t, _q_point[_qp]),
                             _exact_z.value(_t, _q_point[_qp])};

  return _penalty * _test[_i][_qp] * ((_linear ? 0. : _u[_qp]) - u_exact);
}

Real
VectorPenaltyDirichletBC::computeQpJacobian()
{
  return _penalty * _test[_i][_qp] * _phi[_j][_qp];
}

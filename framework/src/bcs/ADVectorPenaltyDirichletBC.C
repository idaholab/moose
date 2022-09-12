//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADVectorPenaltyDirichletBC.h"
#include "Function.h"

registerMooseObject("MooseApp", ADVectorPenaltyDirichletBC);

InputParameters
ADVectorPenaltyDirichletBC::validParams()
{
  InputParameters params = ADVectorIntegratedBC::validParams();
  params.addRequiredParam<Real>("penalty", "The penalty coefficient");
  params.addParam<FunctionName>("x_exact_sln", 0, "The exact solution for the x component");
  params.addParam<FunctionName>("y_exact_sln", 0, "The exact solution for the y component");
  params.addParam<FunctionName>("z_exact_sln", 0, "The exact solution for the z component");
  params.addParam<bool>("penalize_x", true, "Whether to penalize the x component");
  params.addParam<bool>("penalize_y", true, "Whether to penalize the y component");
  params.addParam<bool>("penalize_z", true, "Whether to penalize the z component");
  params.addClassDescription("Enforces a Dirichlet boundary condition for "
                             "vector nonlinear variables in a weak sense by "
                             "applying a penalty to the difference in the "
                             "current solution and the Dirichlet data.");
  return params;
}

ADVectorPenaltyDirichletBC::ADVectorPenaltyDirichletBC(const InputParameters & parameters)
  : ADVectorIntegratedBC(parameters),
    _penalty(getParam<Real>("penalty")),
    _exact_x(getFunction("x_exact_sln")),
    _exact_y(getFunction("y_exact_sln")),
    _exact_z(getFunction("y_exact_sln")),
    _penalize_x(getParam<bool>("penalize_x")),
    _penalize_y(getParam<bool>("penalize_y")),
    _penalize_z(getParam<bool>("penalize_z"))
{
}

ADReal
ADVectorPenaltyDirichletBC::computeQpResidual()
{
  RealVectorValue u_exact = {_exact_x.value(_t, _q_point[_qp]),
                             _exact_y.value(_t, _q_point[_qp]),
                             _exact_z.value(_t, _q_point[_qp])};
  auto delta = _u[_qp] - u_exact;
  if (!_penalize_x)
    delta(0) = 0;
  if (!_penalize_y)
    delta(1) = 0;
  if (!_penalize_z)
    delta(2) = 0;

  return _penalty * _test[_i][_qp] * delta;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayPenaltyDirichletBC.h"

registerMooseObject("MooseApp", ArrayPenaltyDirichletBC);

template <>
InputParameters
validParams<ArrayPenaltyDirichletBC>()
{
  InputParameters params = validParams<ArrayIntegratedBC>();
  params.addParam<Real>("penalty", 4, "Penalty scalar");
  params.addRequiredParam<RealEigenVector>("value", "Boundary value of the array variable");
  params.addClassDescription(
      "Enforces a Dirichlet boundary condition "
      "in a weak sense with $p(\\vec{u}^\\ast, \\vec{u} - \\vec{u}_0)$, where $p$ is the constant "
      "scalar penalty; $\\vec{u}^\\ast$ is the test functions and $\\vec{u} - \\vec{u}_0$ is the "
      "differences between the current solution and the Dirichlet data.");
  return params;
}

ArrayPenaltyDirichletBC::ArrayPenaltyDirichletBC(const InputParameters & parameters)
  : ArrayIntegratedBC(parameters),
    _p(getParam<Real>("penalty")),
    _v(getParam<RealEigenVector>("value"))
{
}

RealEigenVector
ArrayPenaltyDirichletBC::computeQpResidual()
{
  return _p * _test[_i][_qp] * (_u[_qp] - _v);
}

RealEigenVector
ArrayPenaltyDirichletBC::computeQpJacobian()
{
  return RealEigenVector::Constant(_count, _p * _phi[_j][_qp] * _test[_i][_qp]);
}

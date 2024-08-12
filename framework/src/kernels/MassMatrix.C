//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassMatrix.h"

registerMooseObject("MooseApp", MassMatrix);

InputParameters
MassMatrix::validParams()
{
  InputParameters params = MassMatrixBase::validParams();
  params.addClassDescription(
      "Computes a finite element mass matrix using a scalar for the density");
  params.addParam<Real>("density", 1.0, "Optional density for scaling the computed mass.");
  return params;
}

MassMatrix::MassMatrix(const InputParameters & parameters)
  : MassMatrixBase(parameters), _density(getParam<Real>("density"))
{
}

Real
MassMatrix::computeQpJacobian()
{
  return _test[_i][_qp] * _density * _phi[_j][_qp];
}

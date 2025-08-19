//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassMatrixDGKernel.h"
#include "MassMatrix.h"

registerMooseObject("NavierStokesApp", MassMatrixDGKernel);

InputParameters
MassMatrixDGKernel::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addClassDescription(
      "Computes a finite element mass matrix on internal faces meant for use in "
      "preconditioning schemes which require one");
  MassMatrix::setMassMatrixParams(params);
  params.addParam<Real>("density", 1, "The density");
  return params;
}

MassMatrixDGKernel::MassMatrixDGKernel(const InputParameters & parameters)
  : DGKernel(parameters), _density(getParam<Real>("density"))
{
  if (!isParamValid("matrix_tags") && !isParamValid("extra_matrix_tags"))
    mooseError("One of 'matrix_tags' or 'extra_matrix_tags' must be provided");
}

Real
MassMatrixDGKernel::computeQpResidual(Moose::DGResidualType)
{
  mooseAssert(false, "should never be called");
  return 0;
}

Real
MassMatrixDGKernel::computeQpJacobian(const Moose::DGJacobianType type)
{
  Real jac = 0;

  switch (type)
  {
    case Moose::ElementElement:
      jac = _test[_i][_qp] * _density * _phi[_j][_qp];
      break;

    default:
      jac = 0;
      break;
  }

  return jac;
}

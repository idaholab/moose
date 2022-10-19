//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayHFEMDiffusion.h"

registerMooseObject("MooseApp", ArrayHFEMDiffusion);

InputParameters
ArrayHFEMDiffusion::validParams()
{
  InputParameters params = ArrayDGLowerDKernel::validParams();
  params.addClassDescription("Imposes the constraints on internal sides with HFEM.");
  return params;
}

ArrayHFEMDiffusion::ArrayHFEMDiffusion(const InputParameters & parameters)
  : ArrayDGLowerDKernel(parameters)
{
}

void
ArrayHFEMDiffusion::computeQpResidual(Moose::DGResidualType type, RealEigenVector & r)
{
  switch (type)
  {
    case Moose::Element:
      r -= _lambda[_qp] * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      r += _lambda[_qp] * _test_neighbor[_i][_qp];
      break;
  }
}

void
ArrayHFEMDiffusion::computeLowerDQpResidual(RealEigenVector & r)
{
  r += (_u_neighbor[_qp] - _u[_qp]) * _test_lambda[_i][_qp];
}

RealEigenVector
ArrayHFEMDiffusion::computeLowerDQpJacobian(Moose::ConstraintJacobianType type)
{
  RealEigenVector r = RealEigenVector::Zero(_count);
  switch (type)
  {
    case Moose::LowerPrimary:
      return RealEigenVector::Constant(_count, -_test_lambda[_i][_qp] * _phi[_j][_qp]);

    case Moose::LowerSecondary:
      return RealEigenVector::Constant(_count, _test_lambda[_i][_qp] * _phi_neighbor[_j][_qp]);

    case Moose::PrimaryLower:
      return RealEigenVector::Constant(_count, -_phi_lambda[_j][_qp] * _test[_i][_qp]);

    case Moose::SecondaryLower:
      return RealEigenVector::Constant(_count, _phi_lambda[_j][_qp] * _test_neighbor[_i][_qp]);

    default:
      break;
  }

  return r;
}

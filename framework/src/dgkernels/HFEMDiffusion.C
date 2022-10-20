//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HFEMDiffusion.h"

registerMooseObject("MooseApp", HFEMDiffusion);

InputParameters
HFEMDiffusion::validParams()
{
  InputParameters params = DGLowerDKernel::validParams();
  params.addClassDescription("Imposes the constraints on internal sides with HFEM.");
  return params;
}

HFEMDiffusion::HFEMDiffusion(const InputParameters & parameters) : DGLowerDKernel(parameters) {}

Real
HFEMDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  switch (type)
  {
    case Moose::Element:
      return -_lambda[_qp] * _test[_i][_qp];

    case Moose::Neighbor:
      return _lambda[_qp] * _test_neighbor[_i][_qp];
  }
  return 0;
}

Real
HFEMDiffusion::computeLowerDQpResidual()
{
  return (_u_neighbor[_qp] - _u[_qp]) * _test_lambda[_i][_qp];
}

Real HFEMDiffusion::computeQpJacobian(Moose::DGJacobianType) { return 0; }

Real
HFEMDiffusion::computeLowerDQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::LowerPrimary:
      return -_test_lambda[_i][_qp] * _phi[_j][_qp];

    case Moose::LowerSecondary:
      return _test_lambda[_i][_qp] * _phi_neighbor[_j][_qp];

    case Moose::PrimaryLower:
      return -_phi_lambda[_j][_qp] * _test[_i][_qp];

    case Moose::SecondaryLower:
      return _phi_lambda[_j][_qp] * _test_neighbor[_i][_qp];

    default:
      break;
  }

  return 0;
}

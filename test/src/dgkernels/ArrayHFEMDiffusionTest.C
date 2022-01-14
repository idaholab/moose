//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayHFEMDiffusionTest.h"

registerMooseObject("MooseTestApp", ArrayHFEMDiffusionTest);

InputParameters
ArrayHFEMDiffusionTest::validParams()
{
  InputParameters params = ArrayDGLowerDKernel::validParams();
  params.addParam<bool>(
      "for_pjfnk", false, "True to avoid zeros when assembling block-diagonal Jacobian for PJFNK");
  params.addClassDescription("Imposes the constraints on internal sides with HFEM.");
  return params;
}

ArrayHFEMDiffusionTest::ArrayHFEMDiffusionTest(const InputParameters & parameters)
  : ArrayDGLowerDKernel(parameters), _for_pjfnk(getParam<bool>("for_pjfnk"))
{
}

void
ArrayHFEMDiffusionTest::computeQpResidual(Moose::DGResidualType type, RealEigenVector & r)
{
  switch (type)
  {
    case Moose::Element:
      r -= transform() * _lambda[_qp] * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      r += transform() * _lambda[_qp] * _test_neighbor[_i][_qp];
      break;
  }
}

void
ArrayHFEMDiffusionTest::computeLowerDQpResidual(RealEigenVector & r)
{
  r += transform().transpose() * (_u_neighbor[_qp] - _u[_qp]) * _test_lambda[_i][_qp];
}

RealEigenVector
ArrayHFEMDiffusionTest::computeLowerDQpJacobian(Moose::ConstraintJacobianType type)
{
  RealEigenVector r = RealEigenVector::Zero(_count);
  switch (type)
  {
    case Moose::LowerPrimary:
      return -transform().diagonal() * (_test_lambda[_i][_qp] * _phi[_j][_qp]);
      break;

    case Moose::LowerSecondary:
      return transform().diagonal() * (_test_lambda[_i][_qp] * _phi_neighbor[_j][_qp]);
      break;

    case Moose::PrimaryLower:
      return -transform().diagonal() * (_phi_lambda[_j][_qp] * _test[_i][_qp]);
      break;

    case Moose::SecondaryLower:
      return transform().diagonal() * (_phi_lambda[_j][_qp] * _test_neighbor[_i][_qp]);
      break;

    case Moose::LowerLower:
      if (_for_pjfnk)
        return RealEigenVector::Ones(_count);
      break;

    default:
      break;
  }

  return r;
}

RealEigenMatrix
ArrayHFEMDiffusionTest::computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                       const MooseVariableFEBase & jvar)
{
  RealEigenMatrix r = RealEigenMatrix::Zero(_count, jvar.count());
  switch (type)
  {
    case Moose::LowerPrimary:
      if (jvar.number() == _var.number())
        return -transform().transpose() * (_test_lambda[_i][_qp] * _phi[_j][_qp]);
      break;

    case Moose::LowerSecondary:
      if (jvar.number() == _var.number())
        return transform().transpose() * (_test_lambda[_i][_qp] * _phi_neighbor[_j][_qp]);
      break;

    case Moose::PrimaryLower:
      if (jvar.number() == _lowerd_var.number())
        return -transform() * (_phi_lambda[_j][_qp] * _test[_i][_qp]);
      break;

    case Moose::SecondaryLower:
      if (jvar.number() == _lowerd_var.number())
        return transform() * (_phi_lambda[_j][_qp] * _test_neighbor[_i][_qp]);
      break;

    default:
      break;
  }

  return r;
}

RealEigenMatrix
ArrayHFEMDiffusionTest::transform()
{
  mooseAssert(_count > 0, "Number of components of an array variable cannot be zero");

  // construct a simple permutation matrix here for testing
  RealEigenMatrix a = RealEigenMatrix::Zero(_count, _count);
  for (unsigned int i = 1; i < _count; ++i)
    a(i - 1, i) = 1;
  a(_count - 1, 0) = 1;
  return a;
}

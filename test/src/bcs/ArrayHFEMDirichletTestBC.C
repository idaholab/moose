//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayHFEMDirichletTestBC.h"

registerMooseObject("MooseTestApp", ArrayHFEMDirichletTestBC);

InputParameters
ArrayHFEMDirichletTestBC::validParams()
{
  InputParameters params = ArrayLowerDIntegratedBC::validParams();
  params.addParam<RealEigenVector>("value", "Value of the BC");
  params.addParam<bool>(
      "for_pjfnk", false, "True to avoid zeros when assembling block-diagonal Jacobian for PJFNK");
  params.addClassDescription("Imposes the Dirichlet BC with HFEM.");
  return params;
}

ArrayHFEMDirichletTestBC::ArrayHFEMDirichletTestBC(const InputParameters & parameters)
  : ArrayLowerDIntegratedBC(parameters),
    _value(isParamValid("value") ? getParam<RealEigenVector>("value")
                                 : RealEigenVector::Zero(_count)),
    _for_pjfnk(getParam<bool>("for_pjfnk"))
{
  if (_value.size() != _count)
    paramError(
        "value", "Number of values must equal number of variable components (", _count, ").");
}

void
ArrayHFEMDirichletTestBC::computeQpResidual(RealEigenVector & residual)
{
  residual += transform() * _lambda[_qp] * _test[_i][_qp];
}

void
ArrayHFEMDirichletTestBC::computeLowerDQpResidual(RealEigenVector & r)
{
  r += transform().transpose() * (_u[_qp] - _value) * _test_lambda[_i][_qp];
}

RealEigenVector
ArrayHFEMDirichletTestBC::computeLowerDQpJacobian(Moose::ConstraintJacobianType type)
{
  RealEigenVector r = RealEigenVector::Zero(_count);
  switch (type)
  {
    case Moose::LowerPrimary:
      return transform().diagonal() * (_test_lambda[_i][_qp] * _phi[_j][_qp]);

    case Moose::PrimaryLower:
      return transform().diagonal() * (_phi_lambda[_j][_qp] * _test[_i][_qp]);

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
ArrayHFEMDirichletTestBC::computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                         const MooseVariableFEBase & jvar)
{
  RealEigenMatrix r = RealEigenMatrix::Zero(_count, jvar.count());
  switch (type)
  {
    case Moose::LowerPrimary:
      if (jvar.number() == _var.number())
        return transform().transpose() * (_test_lambda[_i][_qp] * _phi[_j][_qp]);
      break;

    case Moose::PrimaryLower:
      if (jvar.number() == _lowerd_var.number())
        return transform() * (_phi_lambda[_j][_qp] * _test[_i][_qp]);
      break;

    default:
      break;
  }

  return r;
}

RealEigenMatrix
ArrayHFEMDirichletTestBC::transform()
{
  mooseAssert(_count > 0, "Number of components of an array variable cannot be zero");

  // construct a simple permutation matrix here for testing
  RealEigenMatrix a = RealEigenMatrix::Zero(_count, _count);
  for (unsigned int i = 1; i < _count; ++i)
    a(i - 1, i) = 1;
  a(_count - 1, 0) = 1;
  return a;
}

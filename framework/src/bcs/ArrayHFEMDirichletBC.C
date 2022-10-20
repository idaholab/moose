//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayHFEMDirichletBC.h"

registerMooseObject("MooseApp", ArrayHFEMDirichletBC);

InputParameters
ArrayHFEMDirichletBC::validParams()
{
  InputParameters params = ArrayLowerDIntegratedBC::validParams();
  params.addParam<RealEigenVector>("value", "Value of the BC");
  params.addCoupledVar("uhat", "The coupled variable");
  params.addClassDescription("Imposes the Dirichlet BC with HFEM.");
  return params;
}

ArrayHFEMDirichletBC::ArrayHFEMDirichletBC(const InputParameters & parameters)
  : ArrayLowerDIntegratedBC(parameters),
    _value(isParamValid("value") ? getParam<RealEigenVector>("value")
                                 : RealEigenVector::Zero(_count)),
    _uhat_var(isParamValid("uhat") ? getArrayVar("uhat", 0) : nullptr),
    _uhat(_uhat_var ? (_is_implicit ? &_uhat_var->slnLower() : &_uhat_var->slnLowerOld()) : nullptr)
{
  if (_value.size() != _count)
    paramError(
        "value", "Number of values must equal number of variable components (", _count, ").");

  if (_uhat_var)
  {
    if (!_uhat_var->activeSubdomains().count(Moose::BOUNDARY_SIDE_LOWERD_ID))
      paramError("uhat",
                 "Must be defined on BOUNDARY_SIDE_LOWERD_SUBDOMAIN subdomain that is added by "
                 "Mesh/build_all_side_lowerd_mesh=true");

    if (_uhat_var->count() != _count)
      paramError("uhat",
                 "The number of components must be equal to the number of "
                 "components of 'variable'");

    if (isParamValid("value"))
      paramError("uhat", "'uhat' and 'value' can not be both provided");
  }
}

void
ArrayHFEMDirichletBC::computeQpResidual(RealEigenVector & residual)
{
  residual += _lambda[_qp] * _test[_i][_qp];
}

void
ArrayHFEMDirichletBC::computeLowerDQpResidual(RealEigenVector & r)
{
  if (_uhat)
    r += (_u[_qp] - (*_uhat)[_qp]) * _test_lambda[_i][_qp];
  else
    r += (_u[_qp] - _value) * _test_lambda[_i][_qp];
}

RealEigenVector
ArrayHFEMDirichletBC::computeLowerDQpJacobian(Moose::ConstraintJacobianType type)
{
  RealEigenVector r = RealEigenVector::Zero(_count);
  switch (type)
  {
    case Moose::LowerPrimary:
      return RealEigenVector::Constant(_count, _test_lambda[_i][_qp] * _phi[_j][_qp]);

    case Moose::PrimaryLower:
      return RealEigenVector::Constant(_count, _phi_lambda[_j][_qp] * _test[_i][_qp]);

    default:
      break;
  }

  return r;
}

RealEigenMatrix
ArrayHFEMDirichletBC::computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                     const MooseVariableFEBase & jvar)
{
  if (_uhat_var && jvar.number() == _uhat_var->number() && type == Moose::LowerLower)
  {
    RealEigenVector v = RealEigenVector::Constant(_count, -_test_lambda[_i][_qp] * _phi[_j][_qp]);
    RealEigenMatrix t = RealEigenMatrix::Zero(_var.count(), _var.count());
    t.diagonal() = v;
    return t;
  }
  else
    return ArrayLowerDIntegratedBC::computeLowerDQpOffDiagJacobian(type, jvar);
}

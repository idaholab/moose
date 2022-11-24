//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HFEMDirichletBC.h"

registerMooseObject("MooseApp", HFEMDirichletBC);

InputParameters
HFEMDirichletBC::validParams()
{
  InputParameters params = LowerDIntegratedBC::validParams();
  params.addParam<RealEigenVector>("value", "Value of the BC");
  params.addCoupledVar("uhat", "The coupled variable");
  params.addClassDescription("Imposes the Dirichlet BC with HFEM.");
  return params;
}

HFEMDirichletBC::HFEMDirichletBC(const InputParameters & parameters)
  : LowerDIntegratedBC(parameters),
    _value(isParamValid("value") ? getParam<Real>("value") : 0),
    _uhat_var(isParamValid("uhat") ? getVar("uhat", 0) : nullptr),
    _uhat(_uhat_var ? (_is_implicit ? &_uhat_var->slnLower() : &_uhat_var->slnLowerOld()) : nullptr)
{
  if (_uhat_var)
  {
    if (!_uhat_var->activeSubdomains().count(Moose::BOUNDARY_SIDE_LOWERD_ID))
      paramError("uhat",
                 "Must be defined on BOUNDARY_SIDE_LOWERD_SUBDOMAIN subdomain that is added by "
                 "Mesh/build_all_side_lowerd_mesh=true");

    if (isParamValid("value"))
      paramError("uhat", "'uhat' and 'value' can not be both provided");
  }
}

Real
HFEMDirichletBC::computeQpResidual()
{
  return _lambda[_qp] * _test[_i][_qp];
}

Real
HFEMDirichletBC::computeLowerDQpResidual()
{
  if (_uhat)
    return (_u[_qp] - (*_uhat)[_qp]) * _test_lambda[_i][_qp];
  else
    return (_u[_qp] - _value) * _test_lambda[_i][_qp];
}

Real
HFEMDirichletBC::computeQpJacobian()
{
  return 0;
}

Real
HFEMDirichletBC::computeLowerDQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::LowerPrimary:
      return _test_lambda[_i][_qp] * _phi[_j][_qp];

    case Moose::PrimaryLower:
      return _phi_lambda[_j][_qp] * _test[_i][_qp];

    default:
      break;
  }

  return 0;
}

Real
HFEMDirichletBC::computeLowerDQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                const MooseVariableFEBase & jvar)
{
  if (_uhat_var && jvar.number() == _uhat_var->number() && type == Moose::LowerLower)
    return -_test_lambda[_i][_qp] * _phi[_j][_qp];
  else
    return 0;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LowerDIntegratedBC.h"

#include "Assembly.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"

#include "libmesh/quadrature.h"

InputParameters
LowerDIntegratedBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredCoupledVar("lowerd_variable", "Lagrange multiplier");
  return params;
}

LowerDIntegratedBC::LowerDIntegratedBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _lowerd_var(*getVar("lowerd_variable", 0)),
    _lambda(_is_implicit ? _lowerd_var.slnLower() : _lowerd_var.slnLowerOld()),

    _phi_lambda(_lowerd_var.phiLower()),
    _test_lambda(_lowerd_var.phiLower())
{
  const auto & lower_domains = _lowerd_var.activeSubdomains();
  if (!lower_domains.count(Moose::BOUNDARY_SIDE_LOWERD_ID) && lower_domains.size() != 1)
    paramError(
        "lowerd_variable",
        "Must be only defined on the subdomain BOUNDARY_SIDE_LOWERD_SUBDOMAIN subdomain that is "
        "added by Mesh/build_all_side_lowerd_mesh=true");

  if (_var.activeSubdomains().count(Moose::BOUNDARY_SIDE_LOWERD_ID))
    paramError("variable",
               "Must not be defined on the subdomain INTERNAL_SIDE_LOWERD_SUBDOMAIN subdomain");

  // Note: the above two conditions also ensure that the variable and lower-d variable are
  // different.
}

void
LowerDIntegratedBC::computeResidual()
{
  IntegratedBC::computeResidual();

  prepareVectorTagLower(_assembly, _lowerd_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initLowerDQpResidual();
    for (_i = 0; _i < _test_lambda.size(); _i++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeLowerDQpResidual();
  }

  accumulateTaggedLocalResidual();
}

void
LowerDIntegratedBC::computeJacobian()
{
  IntegratedBC::computeJacobian();

  computeLowerDJacobian(Moose::LowerLower);
}

void
LowerDIntegratedBC::computeLowerDJacobian(Moose::ConstraintJacobianType type)
{
  mooseAssert(type == Moose::LowerLower || type == Moose::LowerPrimary ||
                  type == Moose::PrimaryLower,
              "Jacobian types must have lower in computeLowerDJacobian");

  const auto & test_space =
      (type == Moose::LowerLower || type == Moose::LowerPrimary) ? _test_lambda : _test;
  unsigned int ivar = (type == Moose::LowerLower || type == Moose::LowerPrimary)
                          ? _lowerd_var.number()
                          : _var.number();

  const auto & loc_phi =
      (type == Moose::LowerLower || type == Moose::PrimaryLower) ? _phi_lambda : _phi;
  const auto jvar = (type == Moose::LowerLower || type == Moose::PrimaryLower)
                        ? _lowerd_var.number()
                        : _var.number();

  // need to transform the type for assembling Jacobian on boundary to be consistent with
  // Assembly::addJacobianLowerD() and Assembly::prepareLowerD().
  Moose::ConstraintJacobianType type_tr =
      (type == Moose::LowerLower
           ? type
           : (type == Moose::LowerPrimary ? Moose::LowerSecondary : Moose::SecondaryLower));
  prepareMatrixTagLower(_assembly, ivar, jvar, type_tr);

  if (_local_ke.n() == 0 || _local_ke.m() == 0)
    return;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    initLowerDQpJacobian(type);
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeLowerDQpJacobian(type);
  }

  accumulateTaggedLocalMatrix();
}

void
LowerDIntegratedBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == variable().number())
  {
    IntegratedBC::computeJacobian();
    computeLowerDJacobian(Moose::LowerPrimary);
  }
  else if (jvar_num == _lowerd_var.number())
  {
    computeLowerDJacobian(Moose::LowerLower);
    computeLowerDJacobian(Moose::PrimaryLower);
  }
  else
  {
    IntegratedBC::computeOffDiagJacobian(jvar_num);
    computeLowerDOffDiagJacobian(Moose::LowerLower, jvar_num);
    computeLowerDOffDiagJacobian(Moose::LowerPrimary, jvar_num);
    computeLowerDOffDiagJacobian(Moose::PrimaryLower, jvar_num);
  }
}

void
LowerDIntegratedBC::computeLowerDOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                 const unsigned int jvar_num)
{
  mooseAssert(type == Moose::LowerLower || type == Moose::LowerPrimary ||
                  type == Moose::PrimaryLower,
              "Jacobian types must have lower in computeLowerDJacobian");

  const auto & test_space =
      (type == Moose::LowerLower || type == Moose::LowerPrimary) ? _test_lambda : _test;
  const auto ivar = (type == Moose::LowerLower || type == Moose::LowerPrimary)
                        ? _lowerd_var.number()
                        : _var.number();

  const auto & jvar = getVariable(jvar_num);

  prepareMatrixTagLower(_assembly, ivar, jvar_num, type);
  if (_local_ke.n() == 0 || _local_ke.m() == 0)
    return;

  if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    const auto & jv0 = static_cast<const MooseVariable &>(jvar);
    const auto & loc_phi =
        (type == Moose::LowerLower || type == Moose::PrimaryLower) ? jv0.phiLower() : jv0.phiFace();

    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      initLowerDQpOffDiagJacobian(type, jvar);
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeLowerDQpOffDiagJacobian(type, jvar);
    }
  }
  else if (jvar.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
    mooseError("Array variable cannot be coupled into integrated BC currently");
  else
    mooseError("Vector variable cannot be coupled into integrated BC currently");

  accumulateTaggedLocalMatrix();
}

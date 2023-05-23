//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMortarConstraint.h"

// MOOSE includes
#include "MooseVariable.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "ADUtils.h"

InputParameters
ADMortarConstraint::validParams()
{
  InputParameters params = MortarConstraintBase::validParams();
  return params;
}

ADMortarConstraint::ADMortarConstraint(const InputParameters & parameters)
  : MortarConstraintBase(parameters),
    _lambda_dummy(),
    _lambda(_var ? _var->adSlnLower() : _lambda_dummy),
    _u_secondary(_secondary_var.adSln()),
    _u_primary(_primary_var.adSlnNeighbor()),
    _grad_u_secondary(_secondary_var.adGradSln()),
    _grad_u_primary(_primary_var.adGradSlnNeighbor())
{
  _subproblem.haveADObjects(true);
}

void
ADMortarConstraint::computeResidual(Moose::MortarType mortar_type)
{
  unsigned int test_space_size = 0;
  switch (mortar_type)
  {
    case Moose::MortarType::Secondary:
      prepareVectorTag(_assembly, _secondary_var.number());
      test_space_size = _test_secondary.size();
      break;

    case Moose::MortarType::Primary:
      prepareVectorTagNeighbor(_assembly, _primary_var.number());
      test_space_size = _test_primary.size();
      break;

    case Moose::MortarType::Lower:
      mooseAssert(_var, "LM variable is null");
      prepareVectorTagLower(_assembly, _var->number());
      test_space_size = _test.size();
      break;
  }

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    for (_i = 0; _i < test_space_size; _i++)
      _local_re(_i) += raw_value(_JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type));

  accumulateTaggedLocalResidual();
}

void
ADMortarConstraint::computeJacobian(Moose::MortarType mortar_type)
{
  std::vector<DualReal> residuals;
  std::size_t test_space_size = 0;
  typedef Moose::ConstraintJacobianType JType;
  typedef Moose::MortarType MType;
  std::vector<JType> jacobian_types;
  std::vector<dof_id_type> dof_indices;
  Real scaling_factor = 1;

  switch (mortar_type)
  {
    case MType::Secondary:
      dof_indices = _secondary_var.dofIndices();
      jacobian_types = {JType::SecondarySecondary, JType::SecondaryPrimary, JType::SecondaryLower};
      scaling_factor = _secondary_var.scalingFactor();
      break;

    case MType::Primary:
      dof_indices = _primary_var.dofIndicesNeighbor();
      jacobian_types = {JType::PrimarySecondary, JType::PrimaryPrimary, JType::PrimaryLower};
      scaling_factor = _primary_var.scalingFactor();
      break;

    case MType::Lower:
      mooseAssert(_var, "The Lagrange Multiplier should be non-null if this is getting called");
      dof_indices = _var->dofIndicesLower();
      jacobian_types = {JType::LowerSecondary, JType::LowerPrimary, JType::LowerLower};
      scaling_factor = _var->scalingFactor();
      break;
  }
  test_space_size = dof_indices.size();

  residuals.resize(test_space_size, 0);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    for (_i = 0; _i < test_space_size; _i++)
      residuals[_i] += _JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type);

  addResidualsAndJacobianWithoutConstraints(_assembly, residuals, dof_indices, scaling_factor);
}

void
ADMortarConstraint::computeResidualAndJacobian()
{
  computeJacobian();
}

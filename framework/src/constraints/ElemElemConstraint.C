//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemElemConstraint.h"

InputParameters
ElemElemConstraint::validParams()
{
  return ElemElemConstraintBase::validParams();
}

ElemElemConstraint::ElemElemConstraint(const InputParameters & parameters)
  : ElemElemConstraintBase(parameters),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _u_neighbor(_var.slnNeighbor()),
    _grad_u_neighbor(_var.gradSlnNeighbor())
{
}

void
ElemElemConstraint::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;
  if (is_elem)
    prepareVectorTag(_assembly, _var.number());
  else
    prepareVectorTagNeighbor(_assembly, _var.number());

  std::vector<Real> residuals(test_space.size(), 0.0);
  for (_qp = 0; _qp < _constraint_q_point.size(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      residuals[_i] += _constraint_weight[_qp] * computeQpResidual(type);

  if (is_elem)
    addResiduals(_assembly, residuals, _var.dofIndices(), _var.scalingFactor());
  else
    addResiduals(_assembly, residuals, _var.dofIndicesNeighbor(), _var.scalingFactor());
}

void
ElemElemConstraint::computeResidual()
{
  // Compute the residual for this element
  computeElemNeighResidual(Moose::Element);

  // Compute the residual for the neighbor
  computeElemNeighResidual(Moose::Neighbor);
}

void
ElemElemConstraint::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;
  prepareMatrixTagNeighbor(_assembly, _var.number(), _var.number(), type);

  for (_qp = 0; _qp < _constraint_q_point.size(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += _constraint_weight[_qp] * computeQpJacobian(type);

  accumulateTaggedLocalMatrix();
}

void
ElemElemConstraint::computeJacobian()
{
  // Compute element-element Jacobian
  computeElemNeighJacobian(Moose::ElementElement);

  // Compute element-neighbor Jacobian
  computeElemNeighJacobian(Moose::ElementNeighbor);

  // Compute neighbor-element Jacobian
  computeElemNeighJacobian(Moose::NeighborElement);

  // Compute neighbor-neighbor Jacobian
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}

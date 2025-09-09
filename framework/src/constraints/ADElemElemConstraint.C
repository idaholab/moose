//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADElemElemConstraint.h"
#include "Assembly.h"
#include "EigenADReal.h"
#include "SystemBase.h"

InputParameters
ADElemElemConstraint::validParams()
{
  return ElemElemConstraintBase::validParams();
}

ADElemElemConstraint::ADElemElemConstraint(const InputParameters & parameters)
  : ElemElemConstraintBase(parameters),
    _u(_var.adSln()),
    _grad_u(_var.adGradSln()),
    _u_neighbor(_var.adSlnNeighbor()),
    _grad_u_neighbor(_var.adGradSlnNeighbor())
{
}

void
ADElemElemConstraint::computeElemNeighResidual(Moose::DGResidualType type)
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
      residuals[_i] += _constraint_weight[_qp] * MetaPhysicL::raw_value(computeQpResidual(type));

  if (is_elem)
    addResiduals(_assembly, residuals, _var.dofIndices(), _var.scalingFactor());
  else
    addResiduals(_assembly, residuals, _var.dofIndicesNeighbor(), _var.scalingFactor());
}

void
ADElemElemConstraint::computeResidual()
{
  // Compute the residual for this element
  computeElemNeighResidual(Moose::Element);

  // Compute the residual for the neighbor
  computeElemNeighResidual(Moose::Neighbor);
}

void
ADElemElemConstraint::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  // AD doesn't need to handle all four cases separately
  // We only compute the diagonal blocks; AD handles cross-terms automatically
  if (type == Moose::ElementNeighbor || type == Moose::NeighborElement)
    return;

  bool is_elem = (type == Moose::ElementElement);
  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;

  if (test_space.size() == 0)
    return;

  std::vector<ADReal> residuals(test_space.size(), 0.0);

  // Calculate the residual - use the corresponding DGResidualType
  Moose::DGResidualType residual_type = is_elem ? Moose::Element : Moose::Neighbor;
  for (_qp = 0; _qp < _constraint_q_point.size(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      residuals[_i] += _constraint_weight[_qp] * computeQpResidual(residual_type);

  // Use addJacobian to properly handle the derivatives
  if (is_elem)
    addJacobian(_assembly, residuals, _var.dofIndices(), _var.scalingFactor());
  else
    addJacobian(_assembly, residuals, _var.dofIndicesNeighbor(), _var.scalingFactor());
}

void
ADElemElemConstraint::computeJacobian()
{
  // For AD, we only compute the diagonal blocks
  // AD automatically handles the off-diagonal coupling
  computeElemNeighJacobian(Moose::ElementElement);
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}

Real
ADElemElemConstraint::computeQpJacobian(Moose::DGJacobianType /*type*/)
{
  mooseError("ADElemElemConstraint does not provide a computeQpJacobian() method. Use "
             "automatic differentiation.");
  return 0.0;
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EqualValueNodalConstraint.h"
#include "MooseMesh.h"

registerMooseObject("MooseTestApp", EqualValueNodalConstraint);

InputParameters
EqualValueNodalConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addRequiredParam<unsigned int>("primary", "The ID of the primary node");
  params.addRequiredParam<unsigned int>("secondary", "The ID of the secondary node");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  return params;
}

EqualValueNodalConstraint::EqualValueNodalConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters), _penalty(getParam<Real>("penalty"))
{
  updateConstrainedNodes();
}

void
EqualValueNodalConstraint::meshChanged()
{
  updateConstrainedNodes();
}

void
EqualValueNodalConstraint::updateConstrainedNodes()
{
  _connected_nodes.clear();
  _primary_node_vector.clear();

  // Only the process that owns the secondary node assembles for it.
  const auto secondary_node_id = getParam<unsigned int>("secondary");
  const Node * const secondary_node = _mesh.getMesh().query_node_ptr(secondary_node_id);
  if (secondary_node && secondary_node->processor_id() == _subproblem.processor_id())
    _connected_nodes.push_back(secondary_node_id);

  // Every process keeps the primary node, since the processes owning secondary nodes need its
  // value. ghostConnectedElems() retains an element connected to it so it stays available on a
  // distributed mesh.
  _primary_node_vector.push_back(getParam<unsigned int>("primary"));

  ghostConnectedElems(_primary_node_vector);
}

EqualValueNodalConstraint::~EqualValueNodalConstraint() {}

Real
EqualValueNodalConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Primary:
      return (_u_primary[_j] - _u_secondary[_i]) * _penalty;

    case Moose::Secondary:
      return (_u_secondary[_i] - _u_primary[_j]) * _penalty;
  }

  return 0.;
}

Real
EqualValueNodalConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::PrimaryPrimary:
      return _penalty;

    case Moose::PrimarySecondary:
      return -_penalty;

    case Moose::SecondarySecondary:
      return _penalty;

    case Moose::SecondaryPrimary:
      return -_penalty;

    default:
      return 0;
  }
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LinearNodalConstraint.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", LinearNodalConstraint);

InputParameters
LinearNodalConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addClassDescription(
      "Constrains secondary node to move as a linear combination of primary nodes.");
  params.addRequiredParam<std::vector<unsigned int>>("primary", "The primary node IDs.");
  params.addParam<std::vector<unsigned int>>("secondary_node_ids",
                                             "The list of secondary node ids");
  params.addParam<BoundaryName>(
      "secondary_node_set", "NaN", "The boundary ID associated with the secondary side");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  params.addRequiredParam<std::vector<Real>>("weights",
                                             "The weights associated with the primary node ids. "
                                             "Must be of the same size as primary nodes");
  return params;
}

LinearNodalConstraint::LinearNodalConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _primary_node_ids(getParam<std::vector<unsigned int>>("primary")),
    _secondary_node_ids(getParam<std::vector<unsigned int>>("secondary_node_ids")),
    _secondary_node_set_id(getParam<BoundaryName>("secondary_node_set")),
    _penalty(getParam<Real>("penalty"))
{
  _weights = getParam<std::vector<Real>>("weights");

  if (_primary_node_ids.size() != _weights.size())
    mooseError("primary and weights should be of equal size.");

  const auto & lm_mesh = _mesh.getMesh();

  if ((_secondary_node_ids.size() == 0) && (_secondary_node_set_id == "NaN"))
    mooseError("Please specify secondary_node_ids or secondary_node_set.");
  else if ((_secondary_node_ids.size() == 0) && (_secondary_node_set_id != "NaN"))
  {
    std::vector<dof_id_type> nodelist =
        _mesh.getNodeList(_mesh.getBoundaryID(_secondary_node_set_id));
    std::vector<dof_id_type>::iterator in;

    for (in = nodelist.begin(); in != nodelist.end(); ++in)
    {
      const Node * const node = lm_mesh.query_node_ptr(*in);
      if (node && node->processor_id() == _subproblem.processor_id())
        _connected_nodes.push_back(*in); // defining secondary nodes in the base class
    }
  }
  else if ((_secondary_node_ids.size() > 0) && (_secondary_node_set_id == "NaN"))
  {
    for (const auto & dof : _secondary_node_ids)
    {
      const Node * const node = lm_mesh.query_node_ptr(dof);
      if (node && node->processor_id() == _subproblem.processor_id())
        _connected_nodes.push_back(dof);
    }
  }

  const auto & node_to_elem_map = _mesh.nodeToElemMap();

  // Add elements connected to primary node to Ghosted Elements
  for (const auto & dof : _primary_node_ids)
  {
    auto node_to_elem_pair = node_to_elem_map.find(dof);

    // Our mesh may be distributed
    if (node_to_elem_pair == node_to_elem_map.end())
      continue;

    // defining primary nodes in base class
    _primary_node_vector.push_back(dof);

    const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

    for (const auto & elem_id : elems)
      _subproblem.addGhostedElem(elem_id);
  }
}

Real
LinearNodalConstraint::computeQpResidual(Moose::ConstraintType type)
{
  /**
   * Secondary residual is u_secondary - weights[1]*u_primary[1]-weights[2]*u_primary[2] ...
   *-u_primary[n]*weights[n]
   * However, computeQPresidual is calculated for only a combination of one primary and one
   *secondary node at a time. To get around this, the residual is split up such that the final
   *secondary residual resembles the above expression.
   **/

  unsigned int primary_size = _primary_node_ids.size();

  switch (type)
  {
    case Moose::Primary:
      return (_u_primary[_j] * _weights[_j] - _u_secondary[_i] / primary_size) * _penalty;
    case Moose::Secondary:
      return (_u_secondary[_i] / primary_size - _u_primary[_j] * _weights[_j]) * _penalty;
  }
  return 0.;
}

Real
LinearNodalConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  unsigned int primary_size = _primary_node_ids.size();

  switch (type)
  {
    case Moose::PrimaryPrimary:
      return _penalty * _weights[_j];
    case Moose::PrimarySecondary:
      return -_penalty / primary_size;
    case Moose::SecondarySecondary:
      return _penalty / primary_size;
    case Moose::SecondaryPrimary:
      return -_penalty * _weights[_j];
    default:
      mooseError("Unsupported type");
      break;
  }
  return 0.;
}

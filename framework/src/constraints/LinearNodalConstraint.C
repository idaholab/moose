//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addClassDescription("Constrains secondary node to move as a linear combination of primary "
                             "nodes, with a penalty term or with exact constraints.");
  params.addRequiredParam<std::vector<unsigned int>>("primary", "The primary node IDs.");
  params.addParam<std::vector<unsigned int>>(
      "secondary_node_ids", {}, "The list of secondary node ids");
  params.addParam<BoundaryName>(
      "secondary_node_set", "NaN", "The boundary ID associated with the secondary side");
  params.addParam<Real>("penalty",
                        "The penalty used for the boundary term. It is required with the penalty "
                        "and kinematic formulations and unused with the exact formulation");
  params.addRequiredParam<std::vector<Real>>("weights",
                                             "The weights associated with the primary node ids. "
                                             "Must be of the same size as primary nodes");
  params.declareControllable("weights");
  return params;
}

LinearNodalConstraint::LinearNodalConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _primary_node_ids(getParam<std::vector<unsigned int>>("primary")),
    _secondary_node_ids(getParam<std::vector<unsigned int>>("secondary_node_ids")),
    _secondary_node_set_id(getParam<BoundaryName>("secondary_node_set")),
    _penalty(isParamValid("penalty") ? getParam<Real>("penalty") : 0.0)
{
  checkPenaltyParam();

  _weights = getParam<std::vector<Real>>("weights");
  _dof_map_weights = _weights;

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

  // defining primary nodes in base class
  _primary_node_vector.assign(_primary_node_ids.begin(), _primary_node_ids.end());
  ghostPrimaryNodes();
}

void
LinearNodalConstraint::addExactConstraints(libMesh::DofMap & dof_map) const
{
  if (_secondary_node_set_id == "NaN")
  {
    NodalConstraint::addExactConstraints(dof_map);
    return;
  }

  // Read at call time: libMesh rebuilds the constraints before meshChanged() refreshes
  // _connected_nodes
  addTieConstraints(dof_map, ownedBoundaryNodes(_secondary_node_set_id));
}

void
LinearNodalConstraint::residualSetup()
{
  NodalConstraint::residualSetup();
  refreshWeights();
}

void
LinearNodalConstraint::jacobianSetup()
{
  NodalConstraint::jacobianSetup();
  refreshWeights();
}

void
LinearNodalConstraint::refreshWeights()
{
  // 'weights' is controllable
  _weights = getParam<std::vector<Real>>("weights");
}

bool
LinearNodalConstraint::exactConstraintsChanged()
{
  // Compare against the weights the DofMap last received, not _weights: refreshWeights() updates
  // _weights in every residual and Jacobian setup, including ones before the next solve
  const auto & weights = getParam<std::vector<Real>>("weights");
  if (weights == _dof_map_weights)
    return false;

  _weights = weights;
  _dof_map_weights = weights;
  return true;
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

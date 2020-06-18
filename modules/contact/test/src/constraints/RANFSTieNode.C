//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RANFSTieNode.h"
#include "PenetrationLocator.h"
#include "PenetrationInfo.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "NearestNodeLocator.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"

registerMooseObject("ContactTestApp", RANFSTieNode);

InputParameters
RANFSTieNode::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params.set<bool>("use_displaced_mesh") = true;

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  return params;
}

RANFSTieNode::RANFSTieNode(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _mesh_dimension(_mesh.dimension()),
    _residual_copy(_sys.residualGhosted())
{
  // modern parameter scheme for displacements
  for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
  {
    _vars.push_back(coupled("displacements", i));
    _var_objects.push_back(getVar("displacements", i));
  }

  if (_vars.size() != _mesh_dimension)
    mooseError("The number of displacement variables does not match the mesh dimension!");
}

void
RANFSTieNode::residualSetup()
{
  _node_to_lm.clear();
}

bool
RANFSTieNode::overwriteSecondaryResidual()
{
  return _nearest_node;
}

bool
RANFSTieNode::shouldApply()
{
  auto & nearest_node_loc = _penetration_locator._nearest_node;
  _nearest_node = nearest_node_loc.nearestNode(_current_node->id());
  if (_nearest_node)
  {
    auto secondary_dof_number = _current_node->dof_number(0, _vars[_component], 0);
    // We overwrite the secondary residual so we cannot use the residual
    // copy for determining the Lagrange multiplier when computing the Jacobian
    if (!_subproblem.currentlyComputingJacobian())
      _node_to_lm.insert(std::make_pair(_current_node->id(),
                                        _residual_copy(secondary_dof_number) /
                                            _var_objects[_component]->scalingFactor()));
    else
    {
      std::vector<dof_id_type> primary_cols;
      std::vector<Number> primary_values;

      _jacobian->get_row(secondary_dof_number, primary_cols, primary_values);
      mooseAssert(primary_cols.size() == primary_values.size(),
                  "The size of the dof container and value container are different");

      _dof_number_to_value.clear();

      for (MooseIndex(primary_cols) i = 0; i < primary_cols.size(); ++i)
        _dof_number_to_value.insert(
            std::make_pair(primary_cols[i], primary_values[i] / _var.scalingFactor()));
    }

    mooseAssert(_node_to_lm.find(_current_node->id()) != _node_to_lm.end(),
                "The node " << _current_node->id() << " should map to a lagrange multiplier");
    _lagrange_multiplier = _node_to_lm[_current_node->id()];

    _primary_index = _current_primary->get_node_index(_nearest_node);
    mooseAssert(_primary_index != libMesh::invalid_uint,
                "nearest node not a node on the current primary element");

    return true;
  }

  return false;
}

Real
RANFSTieNode::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::ConstraintType::Secondary:
      return (*_current_node - *_nearest_node)(_component);

    case Moose::ConstraintType::Primary:
    {
      if (_i == _primary_index)
        return _lagrange_multiplier;

      else
        return 0;
    }

    default:
      return 0;
  }
}

Real
RANFSTieNode::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::ConstraintJacobianType::SecondarySecondary:
      return _phi_secondary[_j][_qp];

    case Moose::ConstraintJacobianType::SecondaryPrimary:
      if (_primary_index == _j)
        return -1;
      else
        return 0;

    case Moose::ConstraintJacobianType::PrimarySecondary:
      if (_i == _primary_index)
      {
        mooseAssert(_dof_number_to_value.find(_connected_dof_indices[_j]) !=
                        _dof_number_to_value.end(),
                    "The connected dof index is not found in the _dof_number_to_value container. "
                    "This must mean that insufficient sparsity was allocated");
        return _dof_number_to_value[_connected_dof_indices[_j]];
      }
      else
        return 0;

    default:
      return 0;
  }
}

void
RANFSTieNode::computeSecondaryValue(NumericVector<Number> &)
{
}

Real
RANFSTieNode::computeQpSecondaryValue()
{
  mooseError(
      "We overrode commputeSecondaryValue so computeQpSecondaryValue should never get called");
}

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

registerMooseObject("ContactApp", RANFSTieNode);

template <>
InputParameters
validParams<RANFSTieNode>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
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
RANFSTieNode::initialSetup()
{
}

void
RANFSTieNode::timestepSetup()
{
}

void
RANFSTieNode::residualSetup()
{
  _node_to_lm.clear();
}

void
RANFSTieNode::jacobianSetup()
{
  _jacobian = &_sys.getMatrix(_sys.systemMatrixTag());
}

bool
RANFSTieNode::overwriteSlaveResidual()
{
  return _nearest_node;
}

bool
RANFSTieNode::overwriteSlaveJacobian()
{
  // We did it ourselves
  return false;
}

bool
RANFSTieNode::shouldApply()
{
  auto & nearest_node_loc = _penetration_locator._nearest_node;
  _nearest_node = nearest_node_loc.nearestNode(_current_node->id());
  if (_nearest_node)
  {
    _dof_number = _current_node->dof_number(0, _vars[_component], 0);
    // We overwrite the slave residual so we cannot use the residual
    // copy for determining the Lagrange multiplier when computing the Jacobian
    if (!_subproblem.currentlyComputingJacobian())
      _node_to_lm.insert(
          std::make_pair(_current_node->id(),
                         _residual_copy(_dof_number) / _var_objects[_component]->scalingFactor()));
    else
    {
      // We need the matrix to be assembled so we get the correct Jacobian entries
      if (!_jacobian->closed())
        _jacobian->close();

      _jacobian->get_row(_dof_number, _master_cols, _master_values);
    }

    mooseAssert(_node_to_lm.find(_current_node->id()) != _node_to_lm.end(),
                "The node " << _current_node->id() << " should map to a lagrange multiplier");
    _lagrange_multiplier = _node_to_lm[_current_node->id()];

    _master_index = _current_master->get_node_index(_nearest_node);
    mooseAssert(_master_index != libMesh::invalid_uint,
                "nearest node not a node on the current master element");

    _master_dof_number = _nearest_node->dof_number(0, _vars[_component], 0);

    return true;
  }

  return false;
}

Real
RANFSTieNode::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::ConstraintType::Slave:
      return (*_current_node - *_nearest_node)(_component);

    case Moose::ConstraintType::Master:
    {
      if (_i == _master_index)
        return _lagrange_multiplier;

      else
        return 0;
    }

    default:
      return 0;
  }
}

Real RANFSTieNode::computeQpJacobian(Moose::ConstraintJacobianType)
{
  mooseError("This shouldn't get called");
}

void
RANFSTieNode::computeSlaveValue(NumericVector<Number> &)
{
}

Real
RANFSTieNode::computeQpSlaveValue()
{
  mooseError("We overrode commputeSlaveValue so computeQpSlaveValue should never get called");
}

void
RANFSTieNode::computeJacobian()
{
  // set the slave row
  std::vector<dof_id_type> slave_row = {_dof_number};

  // This is currently a bad design because we're going to (possibly) assemble here and then put the
  // matrix in an unassembled state again
  if (!_jacobian->closed())
    _jacobian->close();

  // This operation requires that the matrix be assembled
  _jacobian->zero_rows(slave_row);

  // Now set the constraint equation. We just zeroed so it's safe to just use add
  _jacobian->add(_dof_number, _dof_number, 1);
  _jacobian->add(_dof_number, _master_dof_number, -1);

  // Now set the master Jacobian

  mooseAssert(_master_cols.size() == _master_values.size(),
              "Somehow the column indices and column values vectors got out of sync in size");

  for (MooseIndex(_master_cols) i = 0; i < _master_cols.size(); ++i)
    _jacobian->add(_master_dof_number, _master_cols[i], _master_values[i]);
}

void
RANFSTieNode::computeOffDiagJacobian(unsigned int)
{
}

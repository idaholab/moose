//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NodalFrictionalConstraint.h"
#include "NodalConstraintUtils.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "SystemBase.h"

#include "libmesh/null_output_iterator.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"

// C++ includes
#include <limits.h>

registerMooseObject("TensorMechanicsApp", NodalFrictionalConstraint);

InputParameters
NodalFrictionalConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addClassDescription("Frictional nodal constraint for contact");
  params.addRequiredParam<BoundaryName>("boundary", "The primary boundary");
  params.addRequiredParam<BoundaryName>("secondary", "The secondary boundary");
  params.addRequiredParam<Real>("friction_coefficient",
                                "Friction coefficient for slippage in the normal direction");
  params.addRequiredParam<Real>("normal_force",
                                "Normal force used together with friction_coefficient to compute "
                                "the normal frictional capacity.");
  params.addRequiredParam<Real>("tangential_penalty",
                                "Stiffness of the spring in the tangential direction.");
  return params;
}

NodalFrictionalConstraint::NodalFrictionalConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _primary_boundary_id(getParam<BoundaryName>("boundary")),
    _secondary_boundary_id(getParam<BoundaryName>("secondary")),
    _normal_force(getParam<Real>("normal_force")),
    _tangential_penalty(getParam<Real>("tangential_penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _u_secondary_old(_var.dofValuesOldNeighbor()),
    _u_primary_old(_var.dofValuesOld())
{
  if (_var.number() != _var_secondary.number())
    paramError("variable_secondary",
               "Primary variable must be identical to secondary "
               "variable. Different variables are currently not supported.");

  updateConstrainedNodes();

  MooseEnum temp_formulation = getParam<MooseEnum>("formulation");
  if (temp_formulation == "penalty")
    _formulation = Moose::Penalty;
  else if (temp_formulation == "kinematic")
    mooseError("NodalFrictionalConstraint: Kinematic formulation is currently not supported for "
               "this constraint.");
  else
    mooseError("Formulation must be set to Penalty.");
}

void
NodalFrictionalConstraint::meshChanged()
{
  updateConstrainedNodes();
}

void
NodalFrictionalConstraint::updateConstrainedNodes()
{
  _primary_node_vector.clear();
  _connected_nodes.clear();
  _primary_conn.clear();

  std::vector<dof_id_type> secondary_nodelist =
      _mesh.getNodeList(_mesh.getBoundaryID(_secondary_boundary_id));
  std::vector<dof_id_type> primary_nodelist =
      _mesh.getNodeList(_mesh.getBoundaryID(_primary_boundary_id));

  // Fill in _connected_nodes, which defines secondary nodes in the base class
  for (auto in : secondary_nodelist)
  {
    if (_mesh.nodeRef(in).processor_id() == _subproblem.processor_id())
      _connected_nodes.push_back(in);
  }

  // Fill in _primary_node_vector, which defines secondary nodes in the base class
  for (auto in : primary_nodelist)
    _primary_node_vector.push_back(in);

  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  std::vector<std::vector<dof_id_type>> elems(_primary_node_vector.size());

  // Add elements connected to primary node to Ghosted Elements.

  // On a distributed mesh, these elements might have already been
  // remoted, in which case we need to gather them back first.
  if (!_mesh.getMesh().is_serial())
  {
    std::set<Elem *, CompareElemsByLevel> primary_elems_to_ghost;
    std::set<Node *> nodes_to_ghost;

    for (unsigned int i = 0; i < primary_nodelist.size(); ++i)
    {
      auto node_to_elem_pair = node_to_elem_map.find(_primary_node_vector[i]);

      bool found_elems = (node_to_elem_pair != node_to_elem_map.end());

#ifndef NDEBUG
      bool someone_found_elems = found_elems;
      _mesh.getMesh().comm().max(someone_found_elems);
      mooseAssert(someone_found_elems, "Missing entry in node to elem map");
#endif

      if (found_elems)
      {
        for (auto id : node_to_elem_pair->second)
        {
          Elem * elem = _mesh.queryElemPtr(id);
          if (elem)
          {
            primary_elems_to_ghost.insert(elem);

            const unsigned int n_nodes = elem->n_nodes();
            for (unsigned int n = 0; n != n_nodes; ++n)
              nodes_to_ghost.insert(elem->node_ptr(n));
          }
        }
      }
    }

    // Send nodes first since elements need them
    _mesh.getMesh().comm().allgather_packed_range(&_mesh.getMesh(),
                                                  nodes_to_ghost.begin(),
                                                  nodes_to_ghost.end(),
                                                  null_output_iterator<Node>());

    _mesh.getMesh().comm().allgather_packed_range(&_mesh.getMesh(),
                                                  primary_elems_to_ghost.begin(),
                                                  primary_elems_to_ghost.end(),
                                                  null_output_iterator<Elem>());

    _mesh.update(); // Rebuild node_to_elem_map

    // Find elems again now that we know they're there
    const auto & new_node_to_elem_map = _mesh.nodeToElemMap();
    auto node_to_elem_pair = new_node_to_elem_map.find(_primary_node_vector[0]);
    bool found_elems = (node_to_elem_pair != new_node_to_elem_map.end());

    if (!found_elems)
      mooseError("Colundn't find any elements connected to primary node.");

    for (unsigned int i = 0; i < _primary_node_vector.size(); ++i)
      elems[i] = node_to_elem_pair->second;
  }
  else // serial mesh
  {
    for (unsigned int i = 0; i < _primary_node_vector.size(); ++i)
    {
      auto node_to_elem_pair = node_to_elem_map.find(_primary_node_vector[i]);
      bool found_elems = (node_to_elem_pair != node_to_elem_map.end());

      if (!found_elems)
        mooseError("Couldn't find any elements connected to primary node");

      elems[i] = node_to_elem_pair->second;
    }
  }

  for (unsigned int i = 0; i < _primary_node_vector.size(); ++i)
  {
    if (elems[i].size() == 0)
      mooseError("Couldn't find any elements connected to primary node");

    for (unsigned int j = 0; j < elems[i].size(); ++j)
      _subproblem.addGhostedElem(elems[i][j]);
  }

  // Cache map between secondary node and primary node
  _connected_nodes.clear();
  _primary_conn.clear();
  for (unsigned int j = 0; j < secondary_nodelist.size(); ++j)
  {
    if (_mesh.nodeRef(secondary_nodelist[j]).processor_id() == _subproblem.processor_id())
    {
      Node & secondary_node = _mesh.nodeRef(secondary_nodelist[j]);
      for (unsigned int i = 0; i < _primary_node_vector.size(); ++i)
      {
        Node & primary_node = _mesh.nodeRef(_primary_node_vector[i]);
        Real d = (secondary_node - primary_node).norm();
        if (MooseUtils::absoluteFuzzyEqual(d, 0.0))
        {
          _primary_conn.push_back(i);
          _connected_nodes.push_back(secondary_nodelist[j]);
          break;
        }
      }
    }
  }

  _console << "total secondary nodes, primary nodes: " << _primary_conn.size() << ", "
           << _primary_node_vector.size() << '\n';
}

void
NodalFrictionalConstraint::computeResidual(NumericVector<Number> &
                                           /*residual*/)
{
  const auto & primarydof = _var.dofIndices();
  const auto & secondarydof = _var.dofIndicesNeighbor();
  std::vector<Number> re(primarydof.size());
  std::vector<Number> neighbor_re(secondarydof.size());

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    _j = _primary_conn[_i];
    re[_j] += computeQpResidual(Moose::Primary);
    neighbor_re[_i] += computeQpResidual(Moose::Secondary);
    break;
  }
  addResiduals(_assembly, re, primarydof, _var.scalingFactor());
  addResiduals(_assembly, neighbor_re, secondarydof, _var.scalingFactor());
}

Real
NodalFrictionalConstraint::computeQpResidual(Moose::ConstraintType type)
{
  // check whether the tangential spring is already in the yielded state
  Real old_force = (_u_secondary_old[_i] - _u_primary_old[_j]) * _tangential_penalty;
  if (MooseUtils::absoluteFuzzyGreaterThan(std::abs(old_force),
                                           _friction_coefficient * _normal_force))
    old_force = _friction_coefficient * _normal_force * old_force / std::abs(old_force);

  Real current_force =
      ((_u_secondary[_i] - _u_secondary_old[_i]) - (_u_primary[_j] - _u_primary_old[_j])) *
          _tangential_penalty +
      old_force;
  if (MooseUtils::absoluteFuzzyGreaterThan(std::abs(current_force),
                                           _friction_coefficient * _normal_force))
    current_force = _friction_coefficient * _normal_force * current_force / std::abs(current_force);

  switch (type)
  {
    case Moose::Secondary:
      return current_force;
    case Moose::Primary:
      return -current_force;
  }
  return 0;
}

void
NodalFrictionalConstraint::computeJacobian(SparseMatrix<Number> & /*jacobian*/)
{
  // Calculate Jacobian enteries and cache those entries along with the row and column indices
  std::vector<dof_id_type> secondarydof = _var.dofIndicesNeighbor();
  std::vector<dof_id_type> primarydof = _var.dofIndices();

  DenseMatrix<Number> Kee(primarydof.size(), primarydof.size());
  DenseMatrix<Number> Ken(primarydof.size(), secondarydof.size());
  DenseMatrix<Number> Kne(secondarydof.size(), primarydof.size());
  DenseMatrix<Number> Knn(secondarydof.size(), secondarydof.size());

  Kee.zero();
  Ken.zero();
  Kne.zero();
  Knn.zero();

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    _j = _primary_conn[_i];
    Kee(_j, _j) += computeQpJacobian(Moose::PrimaryPrimary);
    Ken(_j, _i) += computeQpJacobian(Moose::PrimarySecondary);
    Kne(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);
    Knn(_i, _i) += computeQpJacobian(Moose::SecondarySecondary);
  }
  addJacobian(_assembly, Kee, primarydof, primarydof, _var.scalingFactor());
  addJacobian(_assembly, Ken, primarydof, secondarydof, _var.scalingFactor());
  addJacobian(_assembly, Kne, secondarydof, primarydof, _var.scalingFactor());
  addJacobian(_assembly, Knn, secondarydof, secondarydof, _var.scalingFactor());
}

Real
NodalFrictionalConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  Real jac = _tangential_penalty;

  // set jacobian to zero if spring has yielded
  Real old_force = (_u_secondary_old[_i] - _u_primary_old[_j]) * _tangential_penalty;
  if (MooseUtils::absoluteFuzzyGreaterThan(std::abs(old_force),
                                           _friction_coefficient * _normal_force))
    old_force = _friction_coefficient * _normal_force * old_force / std::abs(old_force);

  Real current_force =
      ((_u_secondary[_i] - _u_secondary_old[_i]) - (_u_primary[_j] - _u_primary_old[_j])) *
          _tangential_penalty +
      old_force;
  if (MooseUtils::absoluteFuzzyGreaterThan(std::abs(current_force),
                                           _friction_coefficient * _normal_force))
    jac = 0.0;

  switch (type)
  {
    case Moose::SecondarySecondary:
      return jac;
    case Moose::SecondaryPrimary:
      return -jac;
    case Moose::PrimaryPrimary:
      return jac;
    case Moose::PrimarySecondary:
      return -jac;
    default:
      mooseError("Invalid type");
  }
  return 0.;
}

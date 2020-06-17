//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NodalStickConstraint.h"
#include "NodalConstraintUtils.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "SystemBase.h"

#include "libmesh/mesh_inserter_iterator.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"

// C++ includes
#include <limits.h>

registerMooseObject("MooseApp", NodalStickConstraint);

template <>
InputParameters
validParams<NodalStickConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();
  params.addClassDescription("Sticky nodal constraint for contact");
  params.addRequiredParam<BoundaryName>("boundary", "The master boundary");
  params.addRequiredParam<BoundaryName>("secondary", "The secondary boundary");
  params.addRequiredParam<Real>("penalty", "Stiffness of the spring.");
  return params;
}

NodalStickConstraint::NodalStickConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _master_boundary_id(getParam<BoundaryName>("boundary")),
    _secondary_boundary_id(getParam<BoundaryName>("secondary")),
    _penalty(getParam<Real>("penalty"))
{
  updateConstrainedNodes();
}

void
NodalStickConstraint::meshChanged()
{
  updateConstrainedNodes();
}

void
NodalStickConstraint::updateConstrainedNodes()
{
  _master_node_vector.clear();
  _connected_nodes.clear();
  _master_conn.clear();

  std::vector<dof_id_type> secondary_nodelist =
      _mesh.getNodeList(_mesh.getBoundaryID(_secondary_boundary_id));
  std::vector<dof_id_type> master_nodelist =
      _mesh.getNodeList(_mesh.getBoundaryID(_master_boundary_id));

  // Fill in _connected_nodes, which defines secondary nodes in the base class
  for (auto in : secondary_nodelist)
  {
    if (_mesh.nodeRef(in).processor_id() == _subproblem.processor_id())
      _connected_nodes.push_back(in);
  }

  // Fill in _master_node_vector, which defines secondary nodes in the base class
  for (auto in : master_nodelist)
    _master_node_vector.push_back(in);

  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  std::vector<std::vector<dof_id_type>> elems(_master_node_vector.size());

  // Add elements connected to master node to Ghosted Elements.

  // On a distributed mesh, these elements might have already been
  // remoted, in which case we need to gather them back first.
  if (!_mesh.getMesh().is_serial())
  {
    std::set<Elem *, CompareElemsByLevel> master_elems_to_ghost;
    std::set<Node *> nodes_to_ghost;

    for (unsigned int i = 0; i < master_nodelist.size(); ++i)
    {
      auto node_to_elem_pair = node_to_elem_map.find(_master_node_vector[i]);

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
            master_elems_to_ghost.insert(elem);

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
                                                  mesh_inserter_iterator<Node>(_mesh.getMesh()));

    _mesh.getMesh().comm().allgather_packed_range(&_mesh.getMesh(),
                                                  master_elems_to_ghost.begin(),
                                                  master_elems_to_ghost.end(),
                                                  mesh_inserter_iterator<Elem>(_mesh.getMesh()));

    _mesh.update(); // Rebuild node_to_elem_map

    // Find elems again now that we know they're there
    const auto & new_node_to_elem_map = _mesh.nodeToElemMap();
    auto node_to_elem_pair = new_node_to_elem_map.find(_master_node_vector[0]);
    bool found_elems = (node_to_elem_pair != new_node_to_elem_map.end());

    if (!found_elems)
      mooseError("Colundn't find any elements connected to master node.");

    for (unsigned int i = 0; i < _master_node_vector.size(); ++i)
      elems[i] = node_to_elem_pair->second;
  }
  else // serial mesh
  {
    for (unsigned int i = 0; i < _master_node_vector.size(); ++i)
    {
      auto node_to_elem_pair = node_to_elem_map.find(_master_node_vector[i]);
      bool found_elems = (node_to_elem_pair != node_to_elem_map.end());

      if (!found_elems)
        mooseError("Couldn't find any elements connected to master node");

      elems[i] = node_to_elem_pair->second;
    }
  }

  for (unsigned int i = 0; i < _master_node_vector.size(); ++i)
  {
    if (elems[i].size() == 0)
      mooseError("Couldn't find any elements connected to master node");

    for (unsigned int j = 0; j < elems[i].size(); ++j)
      _subproblem.addGhostedElem(elems[i][j]);
  }

  // Cache map between secondary node and master node
  _connected_nodes.clear();
  _master_conn.clear();
  for (unsigned int j = 0; j < secondary_nodelist.size(); ++j)
  {
    if (_mesh.nodeRef(secondary_nodelist[j]).processor_id() == _subproblem.processor_id())
    {
      Node & secondary_node = _mesh.nodeRef(secondary_nodelist[j]);
      for (unsigned int i = 0; i < _master_node_vector.size(); ++i)
      {
        Node & master_node = _mesh.nodeRef(_master_node_vector[i]);
        Real d = (secondary_node - master_node).norm();
        if (MooseUtils::absoluteFuzzyEqual(d, 0.0))
        {
          _master_conn.push_back(i);
          _connected_nodes.push_back(secondary_nodelist[j]);
          break;
        }
      }
    }
  }
}

void
NodalStickConstraint::computeJacobian(SparseMatrix<Number> & jacobian)
{
  // Calculate Jacobian enteries and cache those entries along with the row and column indices
  std::vector<dof_id_type> secondarydof = _var.dofIndicesNeighbor();
  std::vector<dof_id_type> masterdof = _var.dofIndices();

  DenseMatrix<Number> Kee(masterdof.size(), masterdof.size());
  DenseMatrix<Number> Ken(masterdof.size(), secondarydof.size());
  DenseMatrix<Number> Kne(secondarydof.size(), masterdof.size());
  DenseMatrix<Number> Knn(secondarydof.size(), secondarydof.size());

  Kee.zero();
  Ken.zero();
  Kne.zero();
  Knn.zero();

  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    _j = _master_conn[_i];
    switch (_formulation)
    {
      case Moose::Penalty:
        Kee(_j, _j) += computeQpJacobian(Moose::MasterMaster);
        Ken(_j, _i) += computeQpJacobian(Moose::MasterSlave);
        Kne(_i, _j) += computeQpJacobian(Moose::SlaveMaster);
        Knn(_i, _i) += computeQpJacobian(Moose::SlaveSlave);
        break;
      case Moose::Kinematic:
        Kee(_j, _j) = 0.;
        Ken(_j, _i) += jacobian(secondarydof[_i], masterdof[_j]);
        Kne(_i, _j) +=
            -jacobian(secondarydof[_i], masterdof[_j]) + computeQpJacobian(Moose::SlaveMaster);
        Knn(_i, _i) += -jacobian(secondarydof[_i], secondarydof[_i]) + computeQpJacobian(Moose::SlaveSlave);
        break;
    }
  }
  _assembly.cacheJacobianBlock(Kee, masterdof, masterdof, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Ken, masterdof, secondarydof, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Kne, secondarydof, masterdof, _var.scalingFactor());
  _assembly.cacheJacobianBlock(Knn, secondarydof, secondarydof, _var.scalingFactor());
}

void
NodalStickConstraint::computeResidual(NumericVector<Number> & residual)
{
  std::vector<dof_id_type> masterdof = _var.dofIndices();
  std::vector<dof_id_type> secondarydof = _var.dofIndicesNeighbor();
  DenseVector<Number> re(masterdof.size());
  DenseVector<Number> neighbor_re(secondarydof.size());

  re.zero();
  neighbor_re.zero();
  for (_i = 0; _i < secondarydof.size(); ++_i)
  {
    _j = _master_conn[_i];
    switch (_formulation)
    {
      case Moose::Penalty:
        re(_j) += computeQpResidual(Moose::Master) * _var.scalingFactor();
        neighbor_re(_i) += computeQpResidual(Moose::Slave) * _var.scalingFactor();
        break;
      case Moose::Kinematic:
        // Transfer the current residual of the secondary node to the master nodes
        Real res = residual(secondarydof[_i]);
        re(_j) += res;
        neighbor_re(_i) += -res + computeQpResidual(Moose::Slave);
        break;
    }
  }
  _assembly.cacheResidualNodes(re, masterdof);
  _assembly.cacheResidualNodes(neighbor_re, secondarydof);
}

Real
NodalStickConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Slave:
      return (_u_secondary[_i] - _u_master[_j]) * _penalty;
    case Moose::Master:
      return (_u_master[_j] - _u_secondary[_i]) * _penalty;
  }
  return 0.;
}

Real
NodalStickConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::SlaveSlave:
      return _penalty;
    case Moose::SlaveMaster:
      return -_penalty;
    case Moose::MasterMaster:
      return _penalty;
    case Moose::MasterSlave:
      return -_penalty;
    default:
      mooseError("Invalid type");
  }
  return 0.;
}

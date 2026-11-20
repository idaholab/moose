//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SystemBase.h"
#include "MooseVariableFieldBase.h"
#include "MooseMesh.h"
#include "libmesh/system.h"
#include "libmesh/enum_norm_type.h"
#include "libmesh/petsc_vector.h"

/**
 * Provide a simple RAII interface for linear lagrange solution variables.
 */
class SolutionHandle
{
public:
  SolutionHandle(const MooseVariableFieldBase & variable)
    : _var(const_cast<MooseVariableFieldBase &>(variable)),
      _soln(_var.sys().solution()),
      _soln_old(_var.sys().solutionOld())
  {
  }

  /**
   * Get a value from the solution vector.
   */
  Number operator()(const Node * node) const
  {
    // The 0 assumes linear Lagrange (I think)
    dof_id_type dof = node->dof_number(_var.sys().number(), _var.number(), 0);
    return _soln(dof);
  }
  /**
   * Get the value at the fraction \p weight of the way from \p node_in to \p node_out, two nodes
   * of the same axial element, by linear interpolation between the values of the variable at the
   * two nodes on that element. This works for any variable type: an elemental variable has the
   * same value at both nodes. Only valid on the processor that holds the solution.
   */
  Number interpolate(const Node * node_in, const Node * node_out, Real weight) const;
  /**
   * Get a value from the old solution vector.
   */
  Number old(const Node * node) const
  {
    // The 0 assumes linear Lagrange (I think)
    dof_id_type dof = node->dof_number(_var.sys().number(), _var.number(), 0);
    return _soln_old(dof);
  }
  /**
   * Set a value in the solution vector.
   */
  void set(const Node * node, Number val)
  {
    dof_id_type dof = node->dof_number(_var.sys().number(), _var.number(), 0);
    _soln.set(dof, val);
  }

  Real L2norm() const
  {
    return _var.sys().system().calculate_norm(_soln, _var.number(), DISCRETE_L2);
  }

  void close() { _soln.close(); }

private:
  MooseVariableFieldBase & _var;
  NumericVector<Number> & _soln;
  const NumericVector<Number> & _soln_old;
};

inline Number
SolutionHandle::interpolate(const Node * node_in, const Node * node_out, Real weight) const
{
  auto & mesh = _var.sys().mesh();
  for (const auto elem_id : libmesh_map_find(mesh.nodeToElemMap(), node_in->id()))
  {
    const auto & elem = *mesh.elemPtr(elem_id);
    if (elem.get_node_index(node_out) == libMesh::invalid_uint)
      continue;
    const auto & system = _var.sys().system();
    const auto value_in = system.point_value(_var.number(), *node_in, elem, &_soln);
    const auto value_out = system.point_value(_var.number(), *node_out, elem, &_soln);
    // This form returns the nodal value exactly at weight 0 and 1
    return (1 - weight) * value_in + weight * value_out;
  }
  mooseError("Nodes ", node_in->id(), " and ", node_out->id(), " do not share an element.");
}

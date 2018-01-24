//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalsCorner.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "MooseMesh.h"
#include "MooseVariable.h"

#include "libmesh/numeric_vector.h"

Threads::spin_mutex nodal_normals_corner_mutex;

template <>
InputParameters
validParams<NodalNormalsCorner>()
{
  InputParameters params = validParams<SideUserObject>();
  params.addRequiredParam<BoundaryName>(
      "corner_boundary", "Node set ID which contains the nodes that are in 'corners'.");
  return params;
}

NodalNormalsCorner::NodalNormalsCorner(const InputParameters & parameters)
  : SideUserObject(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _corner_boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("corner_boundary")))
{
}

void
NodalNormalsCorner::execute()
{
  Threads::spin_mutex::scoped_lock lock(nodal_normals_corner_mutex);
  NumericVector<Number> & sln = _aux.solution();

  // Get a reference to our BoundaryInfo object
  BoundaryInfo & boundary_info = _mesh.getMesh().get_boundary_info();

  for (unsigned int nd = 0; nd < _current_side_elem->n_nodes(); nd++)
  {
    const Node * node = _current_side_elem->node_ptr(nd);
    if (boundary_info.has_boundary_id(node, _corner_boundary_id) &&
        node->n_dofs(_aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_x").number()) > 0)
    {
      dof_id_type dof_x = node->dof_number(
          _aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_x").number(), 0);
      dof_id_type dof_y = node->dof_number(
          _aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_y").number(), 0);
      dof_id_type dof_z = node->dof_number(
          _aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_z").number(), 0);

      // substitute the normal form the face, we are going to have at least one normal every time
      sln.add(dof_x, _normals[0](0));
      sln.add(dof_y, _normals[0](1));
      sln.add(dof_z, _normals[0](2));
    }
  }
}

void
NodalNormalsCorner::initialize()
{
  _aux.solution().close();
}

void
NodalNormalsCorner::finalize()
{
  _aux.solution().close();
}

void
NodalNormalsCorner::threadJoin(const UserObject & /*uo*/)
{
}

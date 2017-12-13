//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalsCorner.h"
#include "NodalNormalsUserObject.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<NodalNormalsCorner>()
{
  InputParameters params = validParams<SideUserObject>();
  params.addRequiredParam<BoundaryName>(
      "corner_boundary", "Node set ID which contains the nodes that are in 'corners'.");
  params.addRequiredParam<UserObjectName>(
      "nodal_normals_uo", "The name of the user object that holds the nodal normals");
  return params;
}

NodalNormalsCorner::NodalNormalsCorner(const InputParameters & parameters)
  : SideUserObject(parameters),
    _corner_boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("corner_boundary"))),
    _nodal_normals_uo(getUserObject<NodalNormalsUserObject>("nodal_normals_uo"))
{
}

void
NodalNormalsCorner::execute()
{
  BoundaryInfo & boundary_info = _mesh.getMesh().get_boundary_info();

  for (unsigned int nd = 0; nd < _current_side_elem->n_nodes(); nd++)
  {
    const Node * node = _current_side_elem->node_ptr(nd);
    // Is it a corner node?
    if (boundary_info.has_boundary_id(node, _corner_boundary_id))
    {
      // Use the side normal at this corner node
      RealGradient grad(_normals[0](0), _normals[0](1), _normals[0](2));
      _nodal_normals_uo.add(node, grad);
    }
  }
}

void
NodalNormalsCorner::initialize()
{
}

void
NodalNormalsCorner::finalize()
{
}

void
NodalNormalsCorner::threadJoin(const UserObject & /*uo*/)
{
}

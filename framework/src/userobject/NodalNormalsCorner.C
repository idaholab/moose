/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NodalNormalsCorner.h"

Threads::spin_mutex nodal_normals_corner_mutex;

template<>
InputParameters validParams<NodalNormalsCorner>()
{
  InputParameters params = validParams<SideUserObject>();
  params.addRequiredParam<BoundaryName>("corner_boundary", "Node set ID which contains the nodes that are in 'corners'.");
  return params;
}

NodalNormalsCorner::NodalNormalsCorner(const std::string & name, InputParameters parameters) :
    SideUserObject(name, parameters),
    _corner_boundary_id(_mesh.getBoundaryID(getParam<BoundaryName>("corner_boundary"))),
    _nx(_fe_problem.getAuxiliarySystem().getVector("nx")),
    _ny(_fe_problem.getAuxiliarySystem().getVector("ny")),
    _nz(_fe_problem.getAuxiliarySystem().getVector("nz"))
{
}

NodalNormalsCorner::~NodalNormalsCorner()
{
}

void
NodalNormalsCorner::execute()
{
  Threads::spin_mutex::scoped_lock lock(nodal_normals_corner_mutex);

  for (unsigned int nd = 0; nd < _current_side_elem->n_nodes(); nd++)
  {
    const Node * node = _current_side_elem->get_node(nd);
    if (_mesh._mesh.boundary_info->has_boundary_id(node, _corner_boundary_id))
    {
      dof_id_type dof = node->id();
      // substitute the normal form the face, we are going to have at least one normal every time
      _nx.add(dof, _normals[0](0));
      _ny.add(dof, _normals[0](1));
      _nz.add(dof, _normals[0](2));
    }
  }
}

void
NodalNormalsCorner::initialize()
{
}

void
NodalNormalsCorner::destroy()
{
}

void
NodalNormalsCorner::finalize()
{
  _nx.close();
  _ny.close();
  _nz.close();
}

void
NodalNormalsCorner::threadJoin(const UserObject & /*uo*/)
{
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshSmootherBase.h"

#include "FEProblemBase.h"
#include "MooseMesh.h"

#include "libmesh/int_range.h"
#include "libmesh/mesh_base.h"
#include "libmesh/node.h"
#include "libmesh/utility.h"

using namespace libMesh;

InputParameters
MeshSmootherBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("MeshSmootherBase");
  return params;
}

MeshSmootherBase::MeshSmootherBase(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _mesh(_fe_problem.mesh()),
    _remeshing(_fe_problem.getRemeshing())
{
}

Remeshing::PointMap &
MeshSmootherBase::pseudoDisplacement()
{
  return _remeshing.pseudoDisplacement();
}

const Remeshing::PointMap &
MeshSmootherBase::referenceCoordinates() const
{
  return _remeshing.referenceCoordinates();
}

void
MeshSmootherBase::placeNodesAtPseudoDisplacement()
{
  const auto & x0 = referenceCoordinates();
  const auto & d = pseudoDisplacement();
  for (auto & node : _mesh.getMesh().node_ptr_range())
  {
    const auto & reference = libmesh_map_find(x0, node->id());
    const auto & displacement = libmesh_map_find(d, node->id());
    for (const auto i : make_range(Moose::dim))
      (*node)(i) = reference(i) + displacement(i);
  }
}

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

#include "GeometryBase.h"
#include "MooseMesh.h"
#include "libmesh/mesh_base.h"

template <>
InputParameters
validParams<GeometryBase>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Snap refined nodes on a given boundary to a given geometry");
  params.addParam<std::vector<BoundaryName>>(
      "boundary", "List of boundaries whose nodes are snapped to a given geometry");
  return params;
}

GeometryBase::GeometryBase(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _boundary_ids(_mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("boundary")))
{
}

void
GeometryBase::initialize()
{
}

void
GeometryBase::execute()
{
}

void
GeometryBase::finalize()
{
}

void
GeometryBase::meshChanged()
{
  auto & mesh = _mesh.getMesh();

  for (auto & boundary_id : _boundary_ids)
  {
    auto node_ids = _mesh.getNodeList(boundary_id);
    for (auto & node_id : node_ids)
    {
      auto & node = mesh.node_ref(node_id);

      snapNode(node);
    }
  }
}

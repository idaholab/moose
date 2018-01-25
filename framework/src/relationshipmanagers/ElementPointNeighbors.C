//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementPointNeighbors.h"
#include "MooseMesh.h"
#include "Conversion.h"

template <>
InputParameters
validParams<ElementPointNeighbors>()
{
  InputParameters params = validParams<GeometricRelationshipManager>();

  return params;
}

ElementPointNeighbors::ElementPointNeighbors(const InputParameters & parameters)
  : GeometricRelationshipManager(parameters)
{
}

void ElementPointNeighbors::attachRelationshipManagersInternal(
    Moose::RelationshipManagerType /*rm_type*/)
{
  if (_mesh.isDistributedMesh())
  {
    _point_coupling = libmesh_make_unique<GhostPointNeighbors>(_mesh);

    attachGeometricFunctorHelper(*_point_coupling);
  }
}

std::string
ElementPointNeighbors::getInfo() const
{
  if (_point_coupling)
  {
    return "ElementPointNeighbors";
  }
  return "";
}

void
ElementPointNeighbors::operator()(const MeshBase::const_element_iterator &,
                                  const MeshBase::const_element_iterator &,
                                  processor_id_type,
                                  map_type &)
{
  mooseError("Unused");
}

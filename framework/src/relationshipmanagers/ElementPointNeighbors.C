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

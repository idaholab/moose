//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSideNeighborLayers.h"
#include "MooseMesh.h"
#include "Conversion.h"

template <>
InputParameters
validParams<ElementSideNeighborLayers>()
{
  InputParameters params = validParams<GeometricRelationshipManager>();

  params.addRangeCheckedParam<unsigned short>(
      "element_side_neighbor_layers",
      1,
      "element_side_neighbor_layers>=1 & element_side_neighbor_layers<=10",
      "The number of additional geometric elements to make available when "
      "using distributed mesh. No effect with replicated mesh.");

  return params;
}

ElementSideNeighborLayers::ElementSideNeighborLayers(const InputParameters & parameters)
  : GeometricRelationshipManager(parameters),
    _element_side_neighbor_layers(getParam<unsigned short>("element_side_neighbor_layers"))
{
}

void ElementSideNeighborLayers::attachRelationshipManagersInternal(
    Moose::RelationshipManagerType /*rm_type*/)
{
  if (_mesh.isDistributedMesh() && _element_side_neighbor_layers > 1)
  {
    _default_coupling = libmesh_make_unique<DefaultCoupling>();
    _default_coupling->set_n_levels(_element_side_neighbor_layers);

    attachGeometricFunctorHelper(*_default_coupling);
  }
}

std::string
ElementSideNeighborLayers::getInfo() const
{
  if (_default_coupling)
  {
    std::ostringstream oss;
    oss << "ElementSideNeighborLayers (" << _element_side_neighbor_layers << " layers)";
    return oss.str();
  }
  return "";
}

void
ElementSideNeighborLayers::operator()(const MeshBase::const_element_iterator &,
                                      const MeshBase::const_element_iterator &,
                                      processor_id_type,
                                      map_type &)
{
  mooseError("Unused");
}

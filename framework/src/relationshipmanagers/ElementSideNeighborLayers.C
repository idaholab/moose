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

#include "ElementSideNeighborLayers.h"
#include "MooseMesh.h"
#include "Conversion.h"

template <>
InputParameters
validParams<ElementSideNeighborLayers>()
{
  InputParameters params = validParams<RelationshipManager>();

  params.addRequiredRangeCheckedParam<unsigned short>(
      "element_side_neighbor_layers",
      "element_side_neighbor_layers>=1 & element_side_neighbor_layers<=10",
      "The number of additional geometric elements to make available when "
      "using distributed mesh. No effect with replicated mesh.");

  params.set<Moose::RelationshipManagerType>("RelationshipManagerType") = Moose::Geometric;

  return params;
}

ElementSideNeighborLayers::ElementSideNeighborLayers(const InputParameters & parameters)
  : RelationshipManager(parameters),
    _element_side_neighbor_layers(getParam<unsigned short>("element_side_neighbor_layers")),
    _is_active(false)
{
}

void
ElementSideNeighborLayers::init()
{
  if (_mesh.isDistributedMesh() && _element_side_neighbor_layers > 1)
  {
    _default_coupling.set_n_levels(_element_side_neighbor_layers);
    _is_active = true;
  }
}

bool
ElementSideNeighborLayers::isActive() const
{
  return _is_active;
}

std::string
ElementSideNeighborLayers::getInfo() const
{
  std::ostringstream oss;
  oss << "ElementSideNeighborLayers (" << _element_side_neighbor_layers << " layers)";
  return oss.str();
}

void
ElementSideNeighborLayers::operator()(const MeshBase::const_element_iterator & range_begin,
                                      const MeshBase::const_element_iterator & range_end,
                                      processor_id_type p,
                                      map_type & coupled_elements)
{
  _default_coupling(range_begin, range_end, p, coupled_elements);
}
